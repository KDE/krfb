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
#include <qmutex.h>

#define HAVE_PTHREADS
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

class VNCEvent {
public:
	virtual void exec() = 0;
};


class KeyboardEvent : public VNCEvent {
	bool down;
	KeySym keySym;

	static Display *dpy;
	static char modifiers[0x100];
	static KeyCode keycodes[0x100], leftShiftCode, rightShiftCode, altGrCode;
	static const int LEFTSHIFT;
	static const int RIGHTSHIFT;
	static const int ALTGR;
	static char ModifierState;

	static void tweakModifiers(char mod, bool down);
public:
	static void initKeycodes();

	KeyboardEvent(bool d, KeySym k);
	virtual void exec();
};

class PointerEvent : public VNCEvent {
	int button_mask, x, y;

	static bool initialized;
	static Display *dpy;
	static int buttonMask;
public:
	PointerEvent(int b, int _x, int _y);
	virtual void exec();
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
	void connectionAccepted(bool allowRemoteConnection);
	void refuseConnection();
	void connectionClosed();
	bool handleCheckPassword(rfbClientPtr, const char *, int);
	void handleKeyEvent(bool down, KeySym keySym);
	void handlePointerEvent(int button_mask, int x, int y);
	enum rfbNewClientAction handleNewClient(rfbClientPtr cl);
	void handleClientGone();
	int getPort();

	static bool checkX11Capabilities();

public slots:	
	void rebind();
	void passwordChanged();
	void closeConnection();

signals:
        void sessionEstablished();
	void sessionFinished(); 
	void sessionRefused();
	void portProbed(int);
 
private:	
	void startServer(bool xtestGrab = true);
	void stopServer(bool xtestUngrab = true);
	bool checkAsyncEvents();

	bool allowRemoteControl;
	int connectionNum;
	QString remoteIp;

	QTimer idleTimer;
	Configuration *configuration;
	XUpdateScanner *scanner;
	ConnectionDialog dialog;

	rfbScreenInfoPtr server;

	XImage *framebufferImage;

	QMutex asyncMutex;
	QPtrList<VNCEvent> asyncQueue;
	bool closePending;

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
