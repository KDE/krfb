/***************************************************************************
                               rfbcontroller.h
                             -------------------
    begin                : Sun Dec 9 2001
    copyright            : (C) 2001 by Tim Jansen
    email                : tim@tjansen.de
 ***************************************************************************/

/***************************************************************************
 * Contains portions & concept from rfb's x0rfbserver.cc
 * Copyright (C) 2000 heXoNet Support GmbH, D-66424 Homburg.
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RFBCONTROLLER_H
#define RFBCONTROLLER_H

#include "configuration.h"
#include "newconnectiondialog.h"
#include "XUpdateScanner.h"
#include <ksock.h>
#include <qobject.h>
#include <qtimer.h>
// rfbconnection must be last because of X11 headers
#include "rfb.h"

#include <X11/Xlib.h>



class QCloseEvent;

typedef enum {
	RFB_STOPPED,
	RFB_WAITING,
	RFB_CONNECTING,
	RFB_CONNECTED
} RFBState;

class ConnectionDialog : public KRFBConnectionDialog {
	Q_OBJECT
public:
	virtual void closeEvent(QCloseEvent *);

signals:
	void closed();
};

/**
 * Manages sockets, drives the RGBConnection and triggers the connection
 * dialog.
 * The controller has three states: 'waiting for connection',
 * 'waiting for confirmation' and 'connected'. In the first state socket and 
 * connection are null, in the second socket is set and in the last both are 
 * set.
 * @author Tim Jansen
 */
class RFBController : public QObject  {
	Q_OBJECT
public: 
	RFBController(Configuration *c);
	virtual ~RFBController();

	RFBState state;

	void acceptConnection(bool allowRemoteConnection);
	void refuseConnection();
	void connectionClosed();
	bool handleCheckPassword(const char *p, int len);
	void handleKeyEvent(bool down, KeySym keySym);
	void handlePointerEvent(int button_mask, int x, int y);
	enum rfbNewClientAction handleNewClient(rfbClientPtr cl);
	void handleClientGone();

public slots:	
	void rebind();
	void closeConnection();

signals:
        void sessionEstablished();
	void sessionFinished(); 
	void sessionRefused();
 
private:	
	void startServer(bool xtestGrab = true);
	void stopServer(bool xtestUngrab = true);
	void tweakModifiers(char mod, bool down);
	void initKeycodes();

	bool allowRemoteControl;
	int connectionNum;

	QTimer idleTimer;
	Configuration *configuration;
	XUpdateScanner *scanner;
	ConnectionDialog dialog;

	rfbScreenInfoPtr server;
	rfbClientPtr client;

	XImage *framebufferImage;
	int buttonMask;
	char modifiers[0x100];
	KeyCode keycodes[0x100], leftShiftCode, rightShiftCode, altGrCode;

private slots:
	void idleSlot();
	void dialogAccepted();
	void dialogRefused();
};

/*
 * Class to call XTestDiscard at idle time (because otherwise
 * it will not work with QT)
 */
class XTestDisabler : public QObject {
	Q_OBJECT
public:
	XTestDisabler();
	bool disable;
	Display *dpy;
public slots:
	void exec();
};

#endif
