/***************************************************************************
                          rfbserver.cpp  -  description
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

RFBServer::RFBServer(int fd) :
	Server(),
	fd(fd),
	buttonMask(0)	
{
	InitBlocks(32, 32);
}

RFBServer::~RFBServer() {
 	DeleteBlocks();
  shutdown(fd, 2);
  close(fd);
}

void RFBServer::handleKeyEvent(KeyEvent &keyEvent) {
	KeyCode kc = XKeysymToKeycode(dpy, keyEvent.key);
 	if (kc != NoSymbol)
  	XTestFakeKeyEvent(dpy,
                      XKeysymToKeycode( dpy, keyEvent.key ),
                      keyEvent.down_flag,
                      CurrentTime);
}

void RFBServer::handlePointerEvent(PointerEvent &pointerEvent) {
  XTestFakeMotionEvent(dpy,
                       0,
                       pointerEvent.x_position,
                       pointerEvent.y_position,
                       CurrentTime);
  int i = 1;
  while (i <= 5) {
    if ( (buttonMask & (1 << (i-1))) != (pointerEvent.button_mask & (1 << (i-1))) )
      XTestFakeButtonEvent( dpy, i, (pointerEvent.button_mask & (1 << (i-1)))? True : False, CurrentTime );
    i++;
  }
  buttonMask = pointerEvent.button_mask;
}
