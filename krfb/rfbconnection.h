/***************************************************************************
                               rfbconnection.h
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

#ifndef RFBCONNECTION_H
#define RFBCONNECTION_H

// QT must be first because of conflicts with X11
#include <qobject.h>
#include <qstring.h>

#include "XUpdateScanner.h"

#include "../include/rfbServer.h"

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>


using namespace rfb;

/**
 * This is a port from x0rfbserver's BaseServer to KDE. It is called 
 * RFBConnection because the original Server, despite its name, handles only 
 * a single connection after it has been established. This class and its 
 * connections are created and destroyed by RFBController. RFBController is the real
 * serverk, but I did not want to call it Server and create more confusion.
 * Unlike the original this one allows only one client, making stuff a little bit
 * simpler.
 * @author Tim Jansen
 */
class RFBConnection : public QObject, public Server  {
	Q_OBJECT
public: 
	RFBConnection(Display *dpy, int fd, 
		      const QString &cpassword, 
		      bool allowInput, bool showMouse);
	~RFBConnection();
	virtual void handleKeyEvent(KeyEvent &keyEvent);
	virtual void handlePointerEvent(PointerEvent &pointerEvent);
	virtual void getServerInitialisation( ServerInitialisation &_serverInitialisation );
	void scanUpdates();

	BufferedConnection *bufferedConnection;	

private:	
	void createFramebuffer();
	void destroyFramebuffer();

	int fd;
	int buttonMask;
	bool allowInput;
	bool showMousePointer;

	XUpdateScanner *scanner;

	Display *dpy;
	XImage *framebufferImage;
};

/*
 * Class to calls XTestDiscard at idle time (because otherwise
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
