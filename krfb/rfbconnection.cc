/***************************************************************************
                              rfbconnection.cpp
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

#include "rfbconnection.h"

#include <kdebug.h>
#include <qapplication.h>
#include <qtimer.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static XTestDisabler disabler;

RFBConnection::RFBConnection(Display *_dpy, 
			     int _fd, 
			     const QString &cpassword,
			     bool _allowInput) :
	Server(),
	fd(_fd),
	buttonMask(0),
	allowInput(_allowInput),
	dpy(_dpy)
{
	memcpy(password, "\0\0\0\0\0\0\0\0", 8);
	if (!cpassword.isNull())
		strncpy(password, cpassword.latin1(), 
			(8 <= cpassword.length()) ? 8 : cpassword.length());

  	connection = new BufferedConnection(fd, 32768, 16384);

	XTestGrabControl(dpy, true); 
	disabler.disable = false;

	createFramebuffer();

	sendFirstHandshake(connection);
}

RFBConnection::~RFBConnection() {
	destroyFramebuffer();
 	delete connection;

	disabler.disable = true;
	disabler.dpy = dpy;
	QTimer::singleShot(0, &disabler, SLOT(exec()));
}

void RFBConnection::handleKeyEvent(KeyEvent &keyEvent) {
	if (!allowInput)
		return;
	KeyCode kc = XKeysymToKeycode(dpy, keyEvent.key);
 	if (kc != NoSymbol)
  		XTestFakeKeyEvent(dpy,
                      XKeysymToKeycode( dpy, keyEvent.key ),
                      keyEvent.down_flag,
                      CurrentTime);
}

void RFBConnection::handlePointerEvent(PointerEvent &pointerEvent) {
	if (!allowInput)
		return;
  	XTestFakeMotionEvent(dpy,
   			0,
   			pointerEvent.x_position,
                       	pointerEvent.y_position,
                       	CurrentTime);
	int i = 1;
	while (i <= 5) {
		if ( (buttonMask & (1 << (i-1))) != (pointerEvent.button_mask & (1 << (i-1))) )
			XTestFakeButtonEvent( dpy, i,
			(pointerEvent.button_mask & (1 << (i-1)))? True : False,
			CurrentTime );
		i++;
	}
	buttonMask = pointerEvent.button_mask;
}

void RFBConnection::createFramebuffer()
{
	framebufferImage = XGetImage(dpy,
				     QApplication::desktop()->winId(),
				     0,
				     0,
				     QApplication::desktop()->width(),
				     QApplication::desktop()->height(),
				     AllPlanes,
				     ZPixmap);
	framebuffer = new Framebuffer();
	framebuffer->width = framebufferImage->width;
	framebuffer->height = framebufferImage->height;
	framebuffer->bytesPerLine = framebufferImage->bytes_per_line;
	framebuffer->data = (unsigned char*) framebufferImage->data;

	framebuffer->pixelFormat.bits_per_pixel = 
		framebufferImage->bits_per_pixel;
	framebuffer->pixelFormat.depth = framebufferImage->depth;
	framebuffer->pixelFormat.big_endian_flag = 
		(framebufferImage->bitmap_bit_order == MSBFirst);
	framebuffer->pixelFormat.true_colour_flag = true;

	if (framebuffer->pixelFormat.bits_per_pixel == 8) {
		framebuffer->pixelFormat.red_shift = 0;
		framebuffer->pixelFormat.green_shift = 2;
		framebuffer->pixelFormat.blue_shift = 5;
		framebuffer->pixelFormat.red_max   = 3;
		framebuffer->pixelFormat.green_max = 7;
		framebuffer->pixelFormat.blue_max  = 3;
	} else {
		framebuffer->pixelFormat.red_shift = 0;
		if ( framebufferImage->red_mask )
			while (!(framebufferImage->red_mask & (1 << framebuffer->pixelFormat.red_shift)))
				framebuffer->pixelFormat.red_shift++;
		framebuffer->pixelFormat.green_shift = 0;
		if (framebufferImage->green_mask)
			while (!(framebufferImage->green_mask & (1 << framebuffer->pixelFormat.green_shift)))
				framebuffer->pixelFormat.green_shift++;
		framebuffer->pixelFormat.blue_shift = 0;
		if (framebufferImage->blue_mask)
			while (!(framebufferImage->blue_mask & (1 << framebuffer->pixelFormat.blue_shift)))
				framebuffer->pixelFormat.blue_shift++;
		framebuffer->pixelFormat.red_max = framebufferImage->red_mask   >> framebuffer->pixelFormat.red_shift;
		framebuffer->pixelFormat.green_max = framebufferImage->green_mask >> framebuffer->pixelFormat.green_shift;
		framebuffer->pixelFormat.blue_max = framebufferImage->blue_mask  >> framebuffer->pixelFormat.blue_shift;
	}
	scanner = new XUpdateScanner( dpy, QApplication::desktop()->winId(), framebuffer );
}

void RFBConnection::destroyFramebuffer()
{
	delete scanner;
	delete framebuffer;
	XDestroyImage(framebufferImage);
}

void RFBConnection::scanUpdates()
{
  list<Hint> hintList;
  
  scanner->searchUpdates(hintList);
  list<Hint>::iterator i;
  for (i = hintList.begin(); i != hintList.end(); i++)
	  handleHint(*i);
};

void RFBConnection::getServerInitialisation( ServerInitialisation &_serverInit )
{
  Server::getServerInitialisation( _serverInit );
  _serverInit.name_length = strlen( getenv("HOSTNAME") );
  _serverInit.name_string = (CARD8 *) malloc( _serverInit.name_length + 1 );
  strcpy( (char*) _serverInit.name_string, getenv( "HOSTNAME" ) );
}

XTestDisabler::XTestDisabler() :
	disable(false) {
}

void XTestDisabler::exec() {
	if (disable)
		XTestDiscard(dpy);
}

#include "rfbconnection.moc"
