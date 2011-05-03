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
#include <KNotification>

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
    QSharedPointer<FrameBuffer> fb;
    rfbCursorPtr myCursor;
    QByteArray desktopName;
    QTimer rfbUpdateTimer;
    QSet<RfbServer*> servers;
    QSet<RfbClient*> clients;
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

    connect(&d->rfbUpdateTimer, SIGNAL(timeout()), SLOT(updateScreens()));
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(cleanup()));
}

void RfbServerManager::updateScreens()
{
    QList<QRect> rects = d->fb->modifiedTiles();
    QPoint currentCursorPos = QCursor::pos();

    Q_FOREACH(RfbServer *server, d->servers) {
        server->updateScreen(rects);
        server->updateCursorPosition(currentCursorPos);
    }

    //update() might disconnect some of the clients, which will synchronously
    //call the removeClient() method and will change d->clients, so we need
    //to copy the set here to avoid problems.
    QSet<RfbClient*> clients = d->clients;
    Q_FOREACH(RfbClient *client, clients) {
        client->update();
    }
}

void RfbServerManager::cleanup()
{
    kDebug();

    //copy because d->servers is going to be modified while we delete the servers
    QSet<RfbServer*> servers = d->servers;
    Q_FOREACH(RfbServer *server, servers) {
        delete server;
    }

    Q_ASSERT(d->servers.isEmpty());
    Q_ASSERT(d->clients.isEmpty());

    d->myCursor->cleanup = true;
    rfbFreeCursor(d->myCursor);
    d->fb.clear();
}

void RfbServerManager::registerServer(RfbServer* server)
{
    d->servers.insert(server);
}

void RfbServerManager::unregisterServer(RfbServer* server)
{
    d->servers.remove(server);
}

rfbScreenInfoPtr RfbServerManager::newScreen()
{
    rfbScreenInfoPtr screen = NULL;

    if (!d->fb.isNull()) {
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

        screen->desktopName = d->desktopName.constData();
        screen->cursor = d->myCursor;
    }

    return screen;
}

void RfbServerManager::addClient(RfbClient* cc)
{
    if (d->clients.size() == 0) {
        kDebug() << "Starting framebuffer monitor";
        d->fb->startMonitor();
        d->rfbUpdateTimer.start(50);
    }
    d->clients.insert(cc);

    KNotification::event("UserAcceptsConnection",
                         i18n("The remote user %1 is now connected.", cc->name()));

    Q_EMIT clientConnected(cc);
}

void RfbServerManager::removeClient(RfbClient* cc)
{
    d->clients.remove(cc);
    if (d->clients.size() == 0) {
        kDebug() << "Stopping framebuffer monitor";
        d->fb->stopMonitor();
        d->rfbUpdateTimer.stop();
    }

    KNotification::event("ConnectionClosed", i18n("The remote user %1 disconnected.", cc->name()));

    Q_EMIT clientDisconnected(cc);
}

#include "rfbservermanager.moc"
