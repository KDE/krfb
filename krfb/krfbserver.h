//
// C++ Interface: krfbserver
//
// Description:
//
//
// Author: Alessandro Praduroux <pradu@pradu.it>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KRFBSERVER_H
#define KRFBSERVER_H

#include <QObject>

class QTcpServer;
class RFBController;

/**
This class implements the listening server for the RFB protocol.

	@author Alessandro Praduroux <pradu@pradu.it>
*/
class KrfbServer : public QObject
{
Q_OBJECT
public:
    KrfbServer();

    ~KrfbServer();

signals:
    void sessionEstablished(const QString&);
    void sessionFinished();
    void desktopControlSettingChanged(bool);
    void quitApp();

public Q_SLOTS:

    void newConnection();
    void startListening();
    void enableDesktopControl(bool);
    void disconnectAndQuit();

private:
    RFBController *_controller;
    QTcpServer *_server;

};

#endif
