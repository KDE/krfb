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

#include <KConfig>
#include <KGlobal>
#include <KUser>
#include <KLocale>
#include <KStaticDeleter>
#include <KNotification>

#include "connectioncontroller.h"

const int DEFAULT_TCP_PORT = 5900;

static KStaticDeleter<KrfbServer> sd;
KrfbServer * KrfbServer::_self = 0;
KrfbServer * KrfbServer::self() {
    if (!_self) sd.setObject(_self, new KrfbServer);
    return _self;
}


KrfbServer::KrfbServer()
{
    kDebug() << "starting " << endl;
    QTimer::singleShot(0, this, SLOT(startListening()));
}

void KrfbServer::startListening() {

    KSharedConfigPtr conf = KGlobal::config();
    KConfigGroup tcpConfig(conf, "TCP");

    int port = tcpConfig.readEntry("port",DEFAULT_TCP_PORT);

    _server = new TcpServer(this);
    connect(_server,SIGNAL(connectionReceived(int)),SLOT(newConnection(int)));

    if (!_server->listen(QHostAddress::Any, port)) {
        // TODO: handle error more gracefully
        kDebug() << "server listen error" << endl;
        deleteLater();
        return;
    }
    kDebug() << "server listening on port " << DEFAULT_TCP_PORT << endl;
}


KrfbServer::~KrfbServer()
{
    //delete _controller;
}

void KrfbServer::newConnection(int fdNum)
{
    // TODO: get peer address
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
    ConnectionController *cc = new ConnectionController(fd, this);
    cc->run();
}

TcpServer::TcpServer(QObject * parent)
    :QTcpServer(parent)
{
}

void TcpServer::incomingConnection(int fd)
{
    emit connectionReceived(fd);
}

void KrfbServer::handleNotifications(QString name, QString desc )
{
    KNotification::event(name, desc);
}




