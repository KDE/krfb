/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KRFBSERVER_H
#define KRFBSERVER_H

#include <QObject>
#include <rfb/rfb.h>

class ConnectionController;
/**
This class implements the listening server for the RFB protocol.

	@author Alessandro Praduroux <pradu@pradu.it>
*/
class KrfbServer : public QObject
{
Q_OBJECT
friend class ServerManager;
public:
    ~KrfbServer();

    enum rfbNewClientAction handleNewClient(struct _rfbClientRec *cl);
    bool checkX11Capabilities();

signals:
    void sessionEstablished(QString);
    void sessionFinished();
    void desktopControlSettingChanged(bool);

public Q_SLOTS:

    void startListening();
    void processRfbEvents();
    void shutdown();
    void enableDesktopControl(bool);
    void updateSettings();
    void updatePassword();
    void clientDisconnected(ConnectionController *);

    QString listeningAddress() const;
    unsigned int listeningPort() const;

private:
    KrfbServer();

    class KrfbServerP;
    KrfbServerP * const d;

};

#endif
