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
#include "connectiondialog.h"
#include "xupdatescanner.h"
#include <ksock.h>
#include <qobject.h>
#include <qtimer.h>
#include <qmutex.h>

#define HAVE_PTHREADS
#include "rfb.h"

#include <X11/Xlib.h>



class QCloseEvent;
class QClipboard;
class RFBController;

typedef enum {
	RFB_STOPPED,
	RFB_WAITING,
	RFB_CONNECTING,
	RFB_CONNECTED
} RFBState;

class VNCEvent {
public:
	virtual void exec() = 0;
	virtual ~VNCEvent();
};


class KeyboardEvent : public VNCEvent {
	bool down;
	KeySym keySym;

	static Display *dpy;
	static signed char modifiers[0x100];
	static KeyCode keycodes[0x100], leftShiftCode, rightShiftCode, altGrCode;
	static const int LEFTSHIFT;
	static const int RIGHTSHIFT;
	static const int ALTGR;
	static char ModifierState;

	static void tweakModifiers(signed char mod, bool down);
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

class ClipboardEvent : public VNCEvent {
	RFBController *controller;
	QString text;
public:
	ClipboardEvent(RFBController *c, const QString &text);
	virtual void exec();
};

class KNotifyEvent : public VNCEvent {
	QString name;
	QString desc;
public:
	KNotifyEvent(const QString &n, const QString &d);
	virtual ~KNotifyEvent();
	virtual void exec();
};

class SessionEstablishedEvent : public VNCEvent {
        RFBController *controller;
public:
	SessionEstablishedEvent(RFBController *c);
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

	friend class SessionEstablishedEvent;
	friend class ClipboardEvent;
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
	void clipboardToServer(const QString &text);
	void handleClientGone();
	void handleNegotiationFinished(rfbClientPtr cl);
	int getPort();
	void startServer(int inetdFd = -1, bool xtestGrab = true);

	static bool checkX11Capabilities();

public slots:
	void passwordChanged();
	void closeConnection();
	void enableDesktopControl(bool c);

signals:
        void sessionEstablished(QString host);
	void sessionFinished();
	void sessionRefused();
	void quitApp();
	void desktopControlSettingChanged(bool);

private:
	void stopServer(bool xtestUngrab = true);
	void sendKNotifyEvent(const QString &name, const QString &desc);
	void sendSessionEstablished();
	void disableBackground(bool state);

	QString remoteIp;
	volatile bool allowDesktopControl;

	QTimer initIdleTimer;
	QTimer idleTimer;

	enum {
		LAST_SYNC_TO_SERVER,
		LAST_SYNC_TO_CLIENT
	} lastClipboardDirection;
	QString lastClipboardText;
	QClipboard *clipboard;

	Configuration *configuration;
	XUpdateScanner *scanner;
	ConnectionDialog dialog;

	QString desktopName;

	rfbScreenInfoPtr server;

	XImage *framebufferImage;

	QMutex asyncMutex;
	QPtrList<VNCEvent> asyncQueue;

	bool disableBackgroundPending; // background, as desired by libvncserver
	bool disableBackgroundState; // real background state
	bool closePending; // set when libvncserver detected close
	bool forcedClose;  // set when user closed connection
private slots:
	bool checkAsyncEvents();
	void idleSlot();
	void dialogAccepted();
	void dialogRefused();
	void selectionChanged();
	void clipboardChanged();
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
