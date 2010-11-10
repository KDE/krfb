/*
    Copyright (C) 2009-2010 Collabora Ltd <info@collabora.co.uk>
      @author George Goldberg <george.goldberg@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
    Copyright (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "rfbservermanager.h"
#include "rfbserver.h"
#include "framebuffer.h"
#include "framebuffermanager.h"
#include "sockethelpers.h"
#include "krfbconfig.h"
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtNetwork/QHostInfo>
#include <KGlobal>
#include <KDebug>
#include <KLocale>
#include <KUser>

static const char *cur =
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

static const char *mask =
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

/*  copied from vino's bundled libvncserver...
 *  Copyright (C) 2000, 2001 Const Kaplinsky.  All Rights Reserved.
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 */
static void krfb_rfbSetCursorPosition(rfbScreenInfoPtr screen, rfbClientPtr client, int x, int y)
{
    rfbClientIteratorPtr iterator;
    rfbClientPtr cl;

    if (x == screen->cursorX || y == screen->cursorY)
        return;

    LOCK(screen->cursorMutex);
    screen->cursorX = x;
    screen->cursorY = y;
    UNLOCK(screen->cursorMutex);

    /* Inform all clients about this cursor movement. */
    iterator = rfbGetClientIterator(screen);
    while ((cl = rfbClientIteratorNext(iterator)) != NULL) {
        cl->cursorWasMoved = TRUE;
    }
    rfbReleaseClientIterator(iterator);

    /* The cursor was moved by this client, so don't send CursorPos. */
    if (client) {
        client->cursorWasMoved = FALSE;
    }
}

struct RfbClientData
{
    RfbClientData(RfbClient *c, RfbServer *s)
        : client(c), server(s)
    {}

    RfbClient *client;
    RfbServer *server;
};

static rfbBool passwordCheck(rfbClientPtr cl,
                             const char *encryptedPassword,
                             int len)
{
    RfbClientData *data = static_cast<RfbClientData*>(cl->clientData);
    return data->server->checkPassword(data->client, encryptedPassword, len);
}

static void keyboardHook(rfbBool down, rfbKeySym keySym, rfbClientPtr cl)
{
    RfbClientData *data = static_cast<RfbClientData*>(cl->clientData);
    data->server->handleKeyboardEvent(data->client, down ? true : false, keySym);
}

static void pointerHook(int bm, int x, int y, rfbClientPtr cl)
{
    RfbClientData *data = static_cast<RfbClientData*>(cl->clientData);
    data->server->handleMouseEvent(data->client, bm, x, y);
}

static void clipboardHook(char *str, int len, rfbClientPtr cl)
{
    //TODO implement me
}


struct RfbServerManagerStatic
{
    RfbServerManager server;
};

K_GLOBAL_STATIC(RfbServerManagerStatic, s_instance)

RfbServerManager* RfbServerManager::instance()
{
    return &s_instance->server;
}


struct RfbServerManager::Private
{
    Private() : nextServerId(0) {}

    QSharedPointer<FrameBuffer> fb;
    rfbCursorPtr myCursor;
    QByteArray desktopName;
    QTimer rfbProcessEventTimer;

    QSet<RfbClient*> clients;

    int nextServerId;
    QHash<int, RfbServer*> servers;
    QHash<int, rfbScreenInfoPtr> screens;
};


RfbServerManager::RfbServerManager()
    : QObject(), d(new Private)
{
    init();
}

RfbServerManager::~RfbServerManager()
{
    delete d;
}

void RfbServerManager::init()
{
    kDebug();

    d->fb = FrameBufferManager::instance()->frameBuffer(QApplication::desktop()->winId());
    d->myCursor = rfbMakeXCursor(19, 19, (char *) cur, (char *) mask);
    d->myCursor->cleanup = false;
    d->desktopName = QString("%1@%2 (shared desktop)") //FIXME check if we can use utf8
                        .arg(KUser().loginName(),QHostInfo::localHostName()).toLatin1();

    /* Integrate the rfb event mechanism with qt's event loop.
     * Call processRfbEvents() every time the qt event loop is run,
     * so that it also processes and delivers rfb events and call
     * shutdown() when QApplication exits to shutdown the rfb server
     * before the X11 connection goes down.
     */
    connect(&d->rfbProcessEventTimer, SIGNAL(timeout()), SLOT(processRfbEvents()));
    d->rfbProcessEventTimer.start(20);

    connect(qApp, SIGNAL(aboutToQuit()), SLOT(cleanup()));
}

void RfbServerManager::processRfbEvents()
{
    QHashIterator<int, rfbScreenInfoPtr> it(d->screens);
    while(it.hasNext()) {
        it.next();
        rfbScreenInfoPtr screen = it.value();

        //update the cursor position
        QPoint currentCursorPos = QCursor::pos();
        krfb_rfbSetCursorPosition(screen, NULL, currentCursorPos.x(), currentCursorPos.y());

        foreach(const QRect & r, d->fb->modifiedTiles()) {
            rfbMarkRectAsModified(screen, r.x(), r.y(), r.right(), r.bottom());
        }
        rfbProcessEvents(screen, 0);
    }
}

void RfbServerManager::cleanup()
{
    kDebug();

    QHashIterator<int, rfbScreenInfoPtr> it(d->screens);
    while(it.hasNext()) {
        it.next();
        rfbScreenInfoPtr screen = it.value();
        rfbShutdownServer(screen, true);
        rfbScreenCleanup(screen);
    }

    d->myCursor->cleanup = true;
    rfbFreeCursor(d->myCursor);
    d->fb.clear();
    d->rfbProcessEventTimer.stop();
    d->screens.clear();
    d->servers.clear();
}

int RfbServerManager::startServer(RfbServer *server)
{
    kDebug() << "Starting server. Listen port:" << server->listeningPort()
             << "Listen Address:" << server->listeningAddress()
             << "Password enabled:" << server->passwordRequired();

    rfbScreenInfoPtr screen;

    int w = d->fb->width();
    int h = d->fb->height();
    int depth = d->fb->depth();

    int bpp = depth >> 3;

    if (bpp != 1 && bpp != 2 && bpp != 4) {
        bpp = 4;
    }

    kDebug() << "bpp: " << bpp;

    rfbLogEnable(0);
    screen = rfbGetScreen(0, 0, w, h, 8, 3, bpp);

    screen->paddedWidthInBytes = d->fb->paddedWidth();

    d->fb->getServerFormat(screen->serverFormat);

    screen->frameBuffer = d->fb->data();

    if (server->listeningAddress() != "0.0.0.0") {
        strncpy(screen->thisHost, server->listeningAddress().data(), 254);
    }

    if (server->listeningPort() == 0) {
        screen->autoPort = 1;
    }

    screen->port = server->listeningPort();

    // Disable/Enable password checking
    if (server->passwordRequired()) {
        screen->authPasswdData = (void *)1;
    } else {
        screen->authPasswdData = (void *)0;
    }

    // server hooks
    screen->newClientHook = newClientHook;

    screen->kbdAddEvent = keyboardHook;
    screen->ptrAddEvent = pointerHook;
    screen->passwordCheck = passwordCheck;
    screen->setXCutText = clipboardHook;

    screen->desktopName = d->desktopName.constData();
    screen->cursor = d->myCursor;

    rfbInitServer(screen);

    if (!rfbIsActive(screen)) {
        kDebug() << "Failed to start server";
        rfbShutdownServer(screen, true);
        rfbScreenCleanup(screen);
        return -1;
    };

    server->setListeningPort(localPort(screen->listenSock));
    server->setListeningAddress(localAddress(screen->listenSock).toAscii());

    d->nextServerId++;
    d->servers.insert(d->nextServerId, server);
    d->screens.insert(d->nextServerId, screen);

    kDebug() << "Server started. Listen port:" << server->listeningPort()
             << "Listen Address:" << server->listeningAddress();

    return d->nextServerId;
}

void RfbServerManager::stopServer(int id, bool disconnectClients)
{
    if (d->screens.contains(id)) {
        rfbScreenInfoPtr screen = d->screens.value(id);
        rfbShutdownServer(screen, disconnectClients);

        /* If we disconnect all clients, cleanup the
         * internal data as they are not needed anymore */
        if (disconnectClients) {
            d->servers.remove(id);
            d->screens.remove(id);
            rfbScreenCleanup(screen);
        }
    }
}

void RfbServerManager::addClient(RfbClient* cc)
{
    if (d->clients.size() == 0) {
        kDebug() << "Starting framebuffer monitor";
        d->fb->startMonitor();
    }
    d->clients.insert(cc);
}

void RfbServerManager::removeClient(RfbClient* cc)
{
    d->clients.remove(cc);
    if (d->clients.size() == 0) {
        kDebug() << "Stopping framebuffer monitor";
        d->fb->stopMonitor();
    }
}

//static
rfbNewClientAction RfbServerManager::newClientHook(rfbClientPtr cl)
{
    kDebug() << "New client";

    int serverid = instance()->d->screens.key(cl->screen);
    RfbServer *server = instance()->d->servers.value(serverid);

    RfbClient *client = server->newClient(cl);
    instance()->addClient(client);

    //clientData is used by the static callbacks to determine their context
    cl->clientData = new RfbClientData(client, server);
    cl->clientGoneHook = clientGoneHook;

    QObject::connect(client, SIGNAL(connected(RfbClient*)),
                     instance(), SIGNAL(clientConnected(RfbClient*)));
    return client->doHandle();
}

//static
void RfbServerManager::clientGoneHook(rfbClientPtr cl)
{
    kDebug() << "client gone";

    RfbClientData *data = static_cast<RfbClientData*>(cl->clientData);

    if (data->client->isConnected()) {
        Q_EMIT instance()->clientDisconnected(data->client);
    }
    instance()->removeClient(data->client);

    data->client->deleteLater();
    delete data;
}

#include "rfbservermanager.moc"
