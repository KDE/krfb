/***************************************************************************
                          rfbserver.h  -  description
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

#ifndef RFBSERVER_H
#define RFBSERVER_H

#include "../include/rfbServer.h"
#include <X11/extensions/XTest.h>


using namespace rfb;

/**
  * KDE-specific implementation of the Server base class
  * @author Tim Jansen
  */
class RFBServer : public Server  {
public: 
	RFBServer(Display *dpy, int fd);
	~RFBServer();
	virtual void handleKeyEvent(KeyEvent &keyEvent);
	virtual void handlePointerEvent(PointerEvent &pointerEvent);
	
private:	
	int fd;
	int buttonMask;
	BufferedConnection *bufferedConnection;
	Display *dpy;
};

#endif
