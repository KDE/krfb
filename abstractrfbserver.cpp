/* This file is part of the KDE project
 *   Copyright (C) 2009 Collabora Ltd <info@collabora.co.uk>
 *    @author George Goldberg <george.goldberg@collabora.co.uk>
 * 
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program; see the file COPYING.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */

#include "servermanager.h"

#include "abstractrfbserver.h"

#include "abstractconnectioncontroller.h"
#include "framebuffer.h"
#include "invitationmanager.h"
#include "krfbconfig.h"
#include "sockethelpers.h"

#include <QtCore/QPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

static const char* cur=
"                   "
" x                 "
" xx                "
" xxx               "
" xxxx              "
" xxxxx             "
" xxxxxx            "
" xxxxxxx           "
" xxxxxxxx          "
" xxxxxxxxx         "
" xxxxxxxxxx        "
" xxxxx             "
" xx xxx            "
" x  xxx            "
"     xxx           "
"     xxx           "
"      xxx          "
"      xxx          "
"                   ";

static const char* mask=
"xx                 "
"xxx                "
"xxxx               "
"xxxxx              "
"xxxxxx             "
"xxxxxxx            "
"xxxxxxxx           "
"xxxxxxxxx          "
"xxxxxxxxxx         "
"xxxxxxxxxxx        "
"xxxxxxxxxxxx       "
"xxxxxxxxxx         "
"xxxxxxxx           "
"xxxxxxxx           "
"xx  xxxxx          "
"    xxxxx          "
"     xxxxx         "
"     xxxxx         "
"      xxx          ";

static rfbCursorPtr myCursor;


static enum rfbNewClientAction newClientHook(struct _rfbClientRec *cl)
{
    AbstractRfbServer *server = ServerManager::instance()->serverForClient(cl);
    return server->handleNewClient(cl);
}

static rfbBool passwordCheck(rfbClientPtr cl,
                             const char* encryptedPassword,
                             int len)
{
    AbstractConnectionController *cc = static_cast<AbstractConnectionController *>(cl->clientData);
    return cc->handleCheckPassword(cl, encryptedPassword, len);
}

static void keyboardHook(rfbBool down, rfbKeySym keySym, rfbClientPtr cl)
{
    AbstractConnectionController *cc = static_cast<AbstractConnectionController *>(cl->clientData);
    cc->handleKeyEvent(down ? true : false, keySym);
}

static void pointerHook(int bm, int x, int y, rfbClientPtr cl)
{
    AbstractConnectionController *cc = static_cast<AbstractConnectionController *>(cl->clientData);
    cc->handlePointerEvent(bm, x, y);
}

static void clipboardHook(char* str,int len, rfbClientPtr cl)
{
    AbstractConnectionController *cc = static_cast<AbstractConnectionController *>(cl->clientData);
    cc->clipboardToServer(QString::fromUtf8(str, len));
}


class AbstractRfbServer::AbstractRfbServerPrivate {
public:
    AbstractRfbServerPrivate()
    : port(0), screen(0), numClients(0), listeningPort(0),
    passwordRequired(true)
    {
        address = "0.0.0.0";
        desktopName = "Shared Desktop";
    }

    // Parameters for startListening();
    QByteArray address;
    int port;
    QSharedPointer<FrameBuffer> fb;
    rfbScreenInfoPtr screen;
    QByteArray desktopName;
    QTimer rfbProcessEventTimer;
    int numClients;
    QList< QPointer<AbstractConnectionController> > controllers;

    QString listeningAddress;
    unsigned int listeningPort;

    bool passwordRequired;
};

AbstractRfbServer::AbstractRfbServer()
: d(new AbstractRfbServerPrivate)
{
    kDebug();
}

AbstractRfbServer::~AbstractRfbServer()
{
    kDebug();

    delete d;
}

void AbstractRfbServer::startListening()
{
    if (d->fb.isNull()) {
        kWarning() << "Cannot start a rfb server without a valid framebuffer.";
        return;
    }

    rfbScreenInfoPtr screen;

    int w = d->fb->width();
    int h = d->fb->height();
    int depth = d->fb->depth();

    int bpp = depth >> 3;
    if (bpp != 1 && bpp != 2 && bpp != 4) bpp = 4;
    kDebug() << "bpp: " << bpp;

    rfbLogEnable(0);
    screen = rfbGetScreen(0, 0, w, h, 8, 3, bpp);

    screen->paddedWidthInBytes = d->fb->paddedWidth();

    d->fb->getServerFormat(screen->serverFormat);

    screen->frameBuffer = d->fb->data();
    d->screen = screen;

    if (d->address != "0.0.0.0") {
        d->address.resize(254);
        strcpy(screen->thisHost, d->address.data());
    }

    if (d->port == 0) {
        screen->autoPort = 1;
    }

    screen->port = d->port;

    // Disable/Enable password checking
    if (d->passwordRequired) {
        d->screen->authPasswdData = (void *)1;
    } else {
        d->screen->authPasswdData = (void *)0;
    }

    // server hooks
    screen->newClientHook = newClientHook;

    screen->kbdAddEvent = keyboardHook;
    screen->ptrAddEvent = pointerHook;
    screen->newClientHook = newClientHook;
    screen->passwordCheck = passwordCheck;
    screen->setXCutText = clipboardHook;

    screen->desktopName = d->desktopName.constData();

    if (!myCursor) {
        myCursor = rfbMakeXCursor(19, 19, (char*) cur, (char*) mask);
    }
    screen->cursor = myCursor;

    rfbInitServer(screen);
    if (!rfbIsActive(screen)) {
        KMessageBox::error(0,i18n("Address already in use"),"krfb");
        shutdown();
        qApp->quit();
        return;
    };

    d->listeningPort = localPort(screen->listenSock);
    d->listeningAddress = localAddress(screen->listenSock);

    kDebug() << "Listen port:" << d->listeningPort << "Listen Address:" << d->listeningAddress;

    /* Integrate the rfb event mechanism with qt's event loop.
     * Call processRfbEvents() every time the qt event loop is run,
     * so that it also processes and delivers rfb events and call
     * shutdown() when QApplication exits to shutdown the rfb server
     * before the X11 connection goes down.
     */
    connect(&d->rfbProcessEventTimer, SIGNAL(timeout()), SLOT(processRfbEvents()));
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(shutdown()));
    d->rfbProcessEventTimer.start(0);
}

void AbstractRfbServer::setListeningAddress(const QByteArray &address)
{
    d->address = address;
}

void AbstractRfbServer::setListeningPort(int port)
{
    d->port = port;
}

void AbstractRfbServer::setFrameBuffer(QSharedPointer<FrameBuffer> frameBuffer)
{
    d->fb = frameBuffer;
}

void AbstractRfbServer::setDesktopName(const QByteArray &desktopName)
{
    d->desktopName = desktopName;
}

void AbstractRfbServer::setPasswordRequired(bool passwordRequired)
{
    d->passwordRequired = passwordRequired;
}

void AbstractRfbServer::processRfbEvents()
{
    foreach(const QRect &r, d->fb->modifiedTiles()) {
        rfbMarkRectAsModified(d->screen, r.x(), r.y(), r.right(), r.bottom());
    }
    rfbProcessEvents(d->screen, 100);
}

void AbstractRfbServer::shutdown()
{
    rfbShutdownServer(d->screen, true);
    d->fb.clear();

    d->listeningPort = 0;
    d->listeningAddress.clear();
}

QString AbstractRfbServer::listeningAddress() const
{
    return d->listeningAddress;
}

unsigned int AbstractRfbServer::listeningPort() const
{
    kDebug() << "Stored listening port:" << d->listeningPort;
    if (d->screen && d->screen->listenSock) {
        kDebug() << "Actual listening port:" << localPort(d->screen->listenSock);
        kDebug() << d->screen->thisHost;
    }
    return d->listeningPort;
}

bool AbstractRfbServer::checkX11Capabilities() {
    int bp1, bp2, majorv, minorv;
    Bool r = XTestQueryExtension(QX11Info::display(), &bp1, &bp2,
                                 &majorv, &minorv);
    if ((!r) || (((majorv*1000)+minorv) < 2002)) {
        KMessageBox::error(0,
                           i18n("Your X11 Server does not support the required XTest extension version 2.2. Sharing your desktop is not possible."),
                           i18n("Desktop Sharing Error"));
                           return false;
    }

    return true;
}

void AbstractRfbServer::updatePassword()
{
    if (!d->screen) return;
                                QString pw = KrfbConfig::uninvitedConnectionPassword();
    kDebug() << "password: " << pw << " allow " <<
    KrfbConfig::allowUninvitedConnections() <<
    " invitations " << InvitationManager::self()->activeInvitations() << endl;

    if (pw.isEmpty() && InvitationManager::self()->activeInvitations() == 0) {
        kDebug() << "no password from now on";
        d->screen->authPasswdData = (void *)0;
    } else {
        kDebug() << "Ask for password to accept connections";
        d->screen->authPasswdData = (void *)1;
    }
}

void AbstractRfbServer::clientDisconnected(AbstractConnectionController *cc)
{
    kDebug() << "clients--: " << d->numClients;
    if (!--d->numClients) {
        d->fb->stopMonitor();
        kDebug() << "stopMonitor: d->numClients = " << d->numClients;
    }
    disconnect(cc, SIGNAL(clientDisconnected(AbstractConnectionController*)),this, SLOT(clientDisconnected(AbstractConnectionController*)));

    Q_EMIT sessionFinished();
}

void AbstractRfbServer::enableDesktopControl(bool enable)
{
    foreach (QPointer<AbstractConnectionController> ptr, d->controllers) {
        if (ptr) {
            if (ptr->controlCanBeEnabled()) {
                ptr->setControlEnabled(enable);
            }
        }
    }
}

void AbstractRfbServer::appendController(AbstractConnectionController *c)
{
    d->controllers.append(c);
}

int AbstractRfbServer::numClients() const
{
    return d->numClients;
}

void AbstractRfbServer::incrementNumClients()
{
    d->numClients++;
}

void AbstractRfbServer::startFrameBufferMonitor()
{
    d->fb->startMonitor();
}

void AbstractRfbServer::addController(AbstractConnectionController *cc)
{
    if (d->numClients++ == 0)
        d->fb->startMonitor();

    d->controllers.append(cc);
}


#include "abstractrfbserver.moc"

