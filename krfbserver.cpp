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

//#include "rfbcontroller.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QHostInfo>
#include <QApplication>
#include <QDesktopWidget>

#include <KConfig>
#include <KGlobal>
#include <KUser>
#include <KLocale>

#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <QX11Info>

#include <rfb/rfb.h>


const int DEFAULT_TCP_PORT = 5900;

KrfbServer::KrfbServer()
    : QObject(0), _controller(0) //new RFBController(0))
{
    QTimer::singleShot(0, this, SLOT(startListening()));

    // TESTING!!!
    QTimer::singleShot(100000, this, SLOT(disconnectAndQuit()));
}

void KrfbServer::startListening() {

    KSharedConfigPtr conf = KGlobal::config();
    KConfigGroup tcpConfig(conf, "TCP");

    int port = tcpConfig.readEntry("port",DEFAULT_TCP_PORT);

    _server = new QTcpServer(this);
    connect(_server,SIGNAL(newConnection()),SLOT(newConnection()));

    if (!_server->listen(QHostAddress::Any, port)) {
        // TODO: handle error more gracefully
        kDebug() << "server listen error" << endl;
        deleteLater();
        return;
    }
}


KrfbServer::~KrfbServer()
{
    //delete _controller;
}

void KrfbServer::newConnection()
{
    int fdNum = 0;
    QTcpSocket *conn = _server->nextPendingConnection();
    QString peer = conn->peerAddress().toString();
    emit sessionEstablished(peer);
    connect (conn, SIGNAL(disconnected()), SIGNAL(sessionFinished()));

    fdNum = conn->socketDescriptor();
    // TODO: start the actual sharing implementation
    startServer(fdNum);
}

void KrfbServer::enableDesktopControl(bool enable)
{
    // TODO
}

void KrfbServer::disconnectAndQuit()
{
    // TODO: cleanup of existing connections
    _server->close();
    emit quitApp();
}

void KrfbServer::startServer(int fd)
{
    rfbScreenInfoPtr server;
    XImage *framebufferImage;

    framebufferImage = XGetImage(QX11Info::display(),
        QApplication::desktop()->winId(),
        0,
        0,
        QApplication::desktop()->width(),
        QApplication::desktop()->height(),
        AllPlanes,
        ZPixmap);

    int w = framebufferImage->width;
    int h = framebufferImage->height;
    char *fb = framebufferImage->data;

    rfbLogEnable(0);
    server = rfbGetScreen(0, 0, w, h,
            framebufferImage->bits_per_pixel,
            8,
            framebufferImage->bits_per_pixel/8);

    server->paddedWidthInBytes = framebufferImage->bytes_per_line;
    server->frameBuffer = fb;
    server->autoPort = TRUE;
    server->inetdSock = fd;

    server->desktopName = i18n("%1@%2 (shared desktop)", KUser().loginName(), QHostInfo::localHostName()).toLatin1();

//     if (!myCursor)
//         myCursor = rfbMakeXCursor(19, 19, (char*) cur, (char*) mask);
//     server->cursor = myCursor;

    rfbInitServer(server);

    rfbRunEventLoop(server, -1, TRUE);

}


