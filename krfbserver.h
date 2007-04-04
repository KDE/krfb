/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef KRFBSERVER_H
#define KRFBSERVER_H

#include <QObject>
#include <QTcpServer>

class TcpServer: public QTcpServer {
    Q_OBJECT

    public:
        TcpServer(QObject *parent = 0);

    signals:

        void connectionReceived(int fd);

    protected:
        virtual void incomingConnection(int fd);

};

/**
This class implements the listening server for the RFB protocol.

	@author Alessandro Praduroux <pradu@pradu.it>
*/
class KrfbServer : public QObject
{
Q_OBJECT
public:

    static KrfbServer *self();

    ~KrfbServer();

signals:
    void sessionEstablished(QString);
    void sessionFinished();
    void desktopControlSettingChanged(bool);
    void quitApp();

public Q_SLOTS:

    void newConnection(int fd);
    void startListening();
    void enableDesktopControl(bool);
    void disconnectAndQuit();
    void handleNotifications(QString, QString);

private:
    KrfbServer();
    static KrfbServer *_self;

    void startServer(int fd);
    TcpServer *_server;
};

#endif
