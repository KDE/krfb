/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef CONNECTIONCONTROLLER_H
#define CONNECTIONCONTROLLER_H

#include <QObject>

#include <rfb/rfb.h>

class KrfbServer;

/**
	@author Alessandro Praduroux <pradu@pradu.it>
*/
class ConnectionController : public QObject
{
Q_OBJECT
public:
    ConnectionController(struct _rfbClientRec *_cl, KrfbServer *parent);

    ~ConnectionController();

    bool handleCheckPassword(rfbClientPtr cl, const char *response, int len);
    void handleNegotiationFinished(struct _rfbClientRec *cl);

    void handleKeyEvent(bool down , rfbKeySym keySym );
    void handlePointerEvent( int bm, int x, int y);
    void handleClientGone();
    void clipboardToServer(const QString &);

    enum rfbNewClientAction handleNewClient();

    void setControlEnabled(bool enable);

    void setControlCanBeEnabled(bool canBeEnabled);
    bool controlCanBeEnabled() const;

Q_SIGNALS:
    void sessionEstablished(QString);
    void notification(QString, QString);
    void clientDisconnected(ConnectionController *);

protected Q_SLOTS:
    void dialogAccepted();
    void dialogRejected();

private:
    QString remoteIp;
    struct _rfbClientRec *cl;
    bool controlEnabled;
    bool m_controlCanBeEnabled;
    /*
    int fd;
    KrfbServer *server;
    rfbScreenInfoPtr screen;
    rfbClientPtr client;
    QTcpSocket *tcpConn;
    */
};

#endif
