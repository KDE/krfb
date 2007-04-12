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
#include <rfb/rfb.h>

class FrameBuffer;

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

    enum rfbNewClientAction handleNewClient(struct _rfbClientRec *cl);
    bool checkX11Capabilities();

signals:
    void sessionEstablished(QString);
    void sessionFinished();
    void desktopControlSettingChanged(bool);
    void quitApp();

public Q_SLOTS:

    void startListening();
    void enableDesktopControl(bool);
    void disconnectAndQuit();
    void updateSettings();
    void updatePassword();

private:
    KrfbServer();
    static KrfbServer *_self;

    class KrfbServerP;
    KrfbServerP * const d;

};

#endif
