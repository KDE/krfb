/*
    SPDX-FileCopyrightText: 2009-2010 Collabora Ltd <info@collabora.co.uk>
    SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>
    SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>
    SPDX-FileCopyrightText: 2001-2003 Tim Jansen <tim@tjansen.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rfbservermanager.h"
#include "rfbserver.h"
#include "framebuffermanager.h"
#include "sockethelpers.h"
#include "krfbconfig.h"
#include "krfbdebug.h"
#include <QTimer>
#include <QApplication>
#include <QGlobalStatic>
#include <QHostInfo>
#include <qpa/qplatformnativeinterface.h>

#include <KLocalizedString>
#include <KUser>
#include <KNotification>
#include <KWindowSystem>
#include <chrono>

using namespace std::chrono_literals;

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

Q_GLOBAL_STATIC(RfbServerManagerStatic, s_instance)

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

QSharedPointer<FrameBuffer> RfbServerManager::framebuffer() const
{
    return d->fb;
}

QVariantMap RfbServerManager::s_pluginArgs;

void RfbServerManager::init()
{
    //qDebug();
    WId rootWindow = 0;

    if (KWindowSystem::isPlatformX11()) {
        QPlatformNativeInterface* native = qApp->platformNativeInterface();
        rootWindow = reinterpret_cast<WId>(native->nativeResourceForScreen(QByteArrayLiteral("rootwindow"), QGuiApplication::primaryScreen()));
    }

    d->fb = FrameBufferManager::instance()->frameBuffer(rootWindow, s_pluginArgs);
    d->myCursor = rfbMakeXCursor(19, 19, (char *) cur, (char *) mask);
    d->myCursor->cleanup = false;
    d->desktopName = QStringLiteral("%1@%2 (shared desktop)") //FIXME check if we can use utf8
                        .arg(KUser().loginName(),QHostInfo::localHostName()).toLatin1();

    connect(d->fb.data(), &FrameBuffer::frameBufferChanged, this, &RfbServerManager::updateFrameBuffer);
    connect(&d->rfbUpdateTimer, &QTimer::timeout, this, &RfbServerManager::updateScreens);
    connect(qApp, &QApplication::aboutToQuit, this, &RfbServerManager::cleanup);
}

void RfbServerManager::updateFrameBuffer()
{
    for (RfbServer *server : std::as_const(d->servers)) {
        server->updateFrameBuffer(d->fb->data(), d->fb->width(), d->fb->height(), d->fb->depth());
    }
}

void RfbServerManager::updateScreens()
{
    QList<QRect> rects = d->fb->modifiedTiles();
    const QPoint currentCursorPos = d->fb->cursorPosition();

    for (RfbServer* server : std::as_const(d->servers)) {
        server->updateScreen(rects);
        server->updateCursorPosition(currentCursorPos);
    }

    //update() might disconnect some of the clients, which will synchronously
    //call the removeClient() method and will change d->clients, so we need
    //to copy the set here to avoid problems.
    const QSet<RfbClient*> clients = d->clients;
    for (RfbClient* client : clients) {
        client->update();
    }
}

void RfbServerManager::cleanup()
{
    //qDebug();

    //copy because d->servers is going to be modified while we delete the servers
    const QSet<RfbServer*> servers = d->servers;
    qDeleteAll(servers);

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
    rfbScreenInfoPtr screen = nullptr;

    if (!d->fb.isNull()) {
        int w = d->fb->width();
        int h = d->fb->height();
        int depth = d->fb->depth();
        int bpp = depth >> 3;

        if (bpp != 1 && bpp != 2 && bpp != 4) {
            bpp = 4;
        }

        //qDebug() << "bpp: " << bpp;

        rfbLogEnable(KRFB().isDebugEnabled());

        screen = rfbGetScreen(nullptr, nullptr, w, h, 8, 3, bpp);
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
        //qDebug() << "Starting framebuffer monitor";
        d->fb->startMonitor();
        d->rfbUpdateTimer.start(50ms);
    }
    d->clients.insert(cc);

    KNotification::event(QStringLiteral("UserAcceptsConnection"),
                         i18n("The remote user %1 is now connected.", cc->name()));

    Q_EMIT clientConnected(cc);
}

void RfbServerManager::removeClient(RfbClient* cc)
{
    d->clients.remove(cc);
    if (d->clients.size() == 0) {
        //qDebug() << "Stopping framebuffer monitor";
        d->fb->stopMonitor();
        d->rfbUpdateTimer.stop();
    }

    KNotification::event(QStringLiteral("ConnectionClosed"), i18n("The remote user %1 disconnected.", cc->name()));

    Q_EMIT clientDisconnected(cc);
}
