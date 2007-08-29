/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
             (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include "krfbserver.h"
#include "krfbserver.moc"

#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QHostInfo>
#include <QApplication>
#include <QDesktopWidget>
#include <QPointer>

#include <KGlobal>
#include <KUser>
#include <KLocale>
#include <K3StaticDeleter>
#include <KMessageBox>
#include <dnssd/publicservice.h>

#include "connectioncontroller.h"
#include "framebuffer.h"
#include "krfbconfig.h"
#include "invitationmanager.h"

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
    KrfbServer *server = KrfbServer::self();
    return server->handleNewClient(cl);
}

static rfbBool passwordCheck(rfbClientPtr cl,
                             const char* encryptedPassword,
                             int len)
{
    ConnectionController *cc = static_cast<ConnectionController *>(cl->clientData);
    return cc->handleCheckPassword(cl, encryptedPassword, len);
}

static void keyboardHook(rfbBool down, rfbKeySym keySym, rfbClientPtr cl)
{
    ConnectionController *cc = static_cast<ConnectionController *>(cl->clientData);
    cc->handleKeyEvent(down ? true : false, keySym);
}

static void pointerHook(int bm, int x, int y, rfbClientPtr cl)
{
    ConnectionController *cc = static_cast<ConnectionController *>(cl->clientData);
    cc->handlePointerEvent(bm, x, y);
}

static void clipboardHook(char* str,int len, rfbClientPtr cl)
{
    ConnectionController *cc = static_cast<ConnectionController *>(cl->clientData);
    cc->clipboardToServer(QString::fromUtf8(str, len));
}


class KrfbServer::KrfbServerP {

    public:
        KrfbServerP() : fb(0), screen(0), numClients(0) {};

        FrameBuffer *fb;
        QList< QPointer<ConnectionController> > controllers;
        rfbScreenInfoPtr screen;
        bool running;
        int numClients;
        QByteArray desktopName;
};


static K3StaticDeleter<KrfbServer> sdServer;
KrfbServer * KrfbServer::_self = 0;
KrfbServer * KrfbServer::self() {
    if (!_self) sdServer.setObject(_self, new KrfbServer);
    return _self;
}


KrfbServer::KrfbServer()
    :d(new KrfbServerP)
{
    kDebug() << "starting ";
    d->running = true;
    d->fb = FrameBuffer::getFrameBuffer(QApplication::desktop()->winId(), this);
    QTimer::singleShot(0, this, SLOT(startListening()));
    connect(InvitationManager::self(), SIGNAL(invitationNumChanged(int)),SLOT(updatePassword()));

}

KrfbServer::~KrfbServer()
{
    delete d;
}


void KrfbServer::startListening()
{
    rfbScreenInfoPtr screen;

    int port = KrfbConfig::port();

    int w = d->fb->width();
    int h = d->fb->height();
    int depth = d->fb->depth();

    rfbLogEnable(0);
    screen = rfbGetScreen(0, 0, w, h, 8, 3,depth / 8);
    screen->paddedWidthInBytes = d->fb->paddedWidth();

    d->fb->getServerFormat(screen->serverFormat);

    screen->frameBuffer = d->fb->data();
    d->screen = screen;
    screen->autoPort = 0;
    screen->port = port;

    // server hooks
    screen->newClientHook = newClientHook;

    screen->kbdAddEvent = keyboardHook;
    screen->ptrAddEvent = pointerHook;
    screen->newClientHook = newClientHook;
    screen->passwordCheck = passwordCheck;
    screen->setXCutText = clipboardHook;

    d->desktopName = i18n("%1@%2 (shared desktop)", KUser().loginName(), QHostInfo::localHostName()).toLatin1();
    screen->desktopName = d->desktopName.constData();

    if (!myCursor) {
        myCursor = rfbMakeXCursor(19, 19, (char*) cur, (char*) mask);
    }
    screen->cursor = myCursor;

    // configure passwords and desktop control here
    updateSettings();

    rfbInitServer(screen);
    if (!rfbIsActive(screen)) {
        KMessageBox::error(0,"krfb","Address already in use");
        disconnectAndQuit();
    };

    if (KrfbConfig::publishService()) {
        DNSSD::PublicService *service = new DNSSD::PublicService(i18n("%1@%2 (shared desktop)", KUser().loginName(), QHostInfo::localHostName()),"_rfb._tcp",port);
        service->publishAsync();
    }

    while (d->running) {
        foreach(QRect r, d->fb->modifiedTiles()) {
            rfbMarkRectAsModified(screen, r.x(), r.y(), r.right(), r.bottom());
        }
        rfbProcessEvents(screen, 100);
        qApp->processEvents();
    }
    rfbShutdownServer(screen, true);
    emit quitApp();
}


void KrfbServer::enableDesktopControl(bool enable)
{
    foreach (QPointer<ConnectionController> ptr, d->controllers) {
        if (ptr) {
            ptr->setControlEnabled(enable);
        }
    }
}

void KrfbServer::disconnectAndQuit()
{
    d->running = false;
}

enum rfbNewClientAction KrfbServer::handleNewClient(struct _rfbClientRec * cl)
{
    ConnectionController *cc = new ConnectionController(cl, this);
    if (d->numClients == 0) {
        d->fb->startMonitor();
        d->numClients++;
    }

    d->controllers.append(cc);
    cc->setControlEnabled(KrfbConfig::allowDesktopControl());

    connect(cc, SIGNAL(sessionEstablished(QString)), SIGNAL(sessionEstablished(QString)));
    connect(cc, SIGNAL(clientDisconnected(ConnectionController *)),SLOT(clientDisconnected(ConnectionController *)));

    return cc->handleNewClient();
}

void KrfbServer::updateSettings()
{
    enableDesktopControl(KrfbConfig::allowDesktopControl());
    updatePassword();
}

void KrfbServer::updatePassword()
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

bool KrfbServer::checkX11Capabilities() {
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

void KrfbServer::clientDisconnected(ConnectionController *cc)
{
    kDebug() << "clients--: " << d->numClients;
    d->numClients--;
    if (d->numClients == 0) {
        d->fb->stopMonitor();
    }
    disconnect(cc, SIGNAL(clientDisconnected(ConnectionController)),this, SLOT(clientDisconnected(ConnectionController)));
}



