/***************************************************************************
                              rfbcontroller.cpp
                             -------------------
    begin                : Sun Dec 9 2001
    copyright            : (C) 2001-2002 by Tim Jansen
    email                : tim@tjansen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rfbcontroller.h"

#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>

#include <kapp.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kextsock.h>
#include <qcursor.h>
#include <qwindowdefs.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qglobal.h>
#include <qlabel.h>

#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#ifndef ASSERT
#define ASSERT(x) Q_ASSERT(x)
#endif

#define IDLE_PAUSE (1000/50)

static XTestDisabler disabler;

// only one controller exists, so we can do this workaround for functions:
static RFBController *self;

class AppLocker
{
public:
	AppLocker() {
		KApplication::kApplication()->lock();
	}
	
	~AppLocker() {
		KApplication::kApplication()->unlock();
	}
};

static void keyboardHook(Bool down, KeySym keySym, rfbClientPtr)
{
	AppLocker a;
	self->handleKeyEvent(down?true:false, keySym);
}

static void pointerHook(int bm, int x, int y, rfbClientPtr)
{
	AppLocker a;
	self->handlePointerEvent(bm, x, y);
}

static enum rfbNewClientAction newClientHook(struct _rfbClientRec *cl) 
{
	AppLocker a;
	return self->handleNewClient(cl);
}

static Bool passwordCheck(rfbClientPtr cl, 
			  char* encryptedPassword,
			  int len)
{
	AppLocker a;
	self->handleCheckPassword(encryptedPassword, len);
}

static void clientGoneHook(rfbClientPtr cl) 
{
	AppLocker a;
	self->handleClientGone();
}


void ConnectionDialog::closeEvent(QCloseEvent *) 
{
	emit closed();
}



RFBController::RFBController(Configuration *c) :
	allowRemoteControl(false),
	connectionNum(0),
	configuration(c)
{
	self = this;
	connect(dialog.acceptConnectionButton, SIGNAL(clicked()),
		SLOT(dialogAccepted()));
	connect(dialog.refuseConnectionButton, SIGNAL(clicked()),
		SLOT(dialogRefused()));
	connect(&dialog, SIGNAL(closed()), SLOT(dialogRefused()));
	connect(&idleTimer, SIGNAL(timeout()), SLOT(idleSlot()));

	startServer();
}

RFBController::~RFBController() 
{
	stopServer();
}



void RFBController::startServer(bool xtestGrab) 
{
	framebufferImage = XGetImage(qt_xdisplay(),
				     QApplication::desktop()->winId(),
				     0,
				     0,
				     QApplication::desktop()->width(),
				     QApplication::desktop()->height(),
				     AllPlanes,
				     ZPixmap);

	int w = framebufferImage->width;
	int h = framebufferImage->height;
	int bpp = framebufferImage->depth;
	char *fb = framebufferImage->data;
	
	int red_max, green_max, blue_max;
	int red_shift, green_shift, blue_shift;

	if (bpp == 8) {
		red_max   = 3;
		green_max = 7;
		blue_max  = 3;
		red_shift = 0;
		green_shift = 2;
		blue_shift = 5;
	} else {
		red_shift = 0;
		if ( framebufferImage->red_mask )
			while (!(framebufferImage->red_mask & (1 << red_shift)))
				red_shift++;
		green_shift = 0;
		if (framebufferImage->green_mask)
			while (!(framebufferImage->green_mask & (1 << green_shift)))
				green_shift++;
		blue_shift = 0;
		if (framebufferImage->blue_mask)
			while (!(framebufferImage->blue_mask & (1 << blue_shift)))
				blue_shift++;
		red_max = framebufferImage->red_mask   >> red_shift;
		green_max = framebufferImage->green_mask >> green_shift;
		blue_max = framebufferImage->blue_mask  >> blue_shift;
	}

	server = rfbGetScreen2(0, 0, w, h, bpp/8,
			       red_max, green_max, blue_max,
			       red_shift, green_shift, blue_shift);
	server->frameBuffer = fb;
	server->rfbPort = configuration->port();
	//server->udpPort = configuration->port();
	
	server->kbdAddEvent = keyboardHook;
	server->ptrAddEvent = pointerHook;
	server->newClientHook = newClientHook;
	server->passwordCheck = passwordCheck;

	scanner = new XUpdateScanner(qt_xdisplay(), 
				     QApplication::desktop()->winId(), 
				     (unsigned char*)fb, 
				     w, h, bpp, (bpp/8)*w);

	rfbInitServer(server);
	state = RFB_WAITING;

	if (xtestGrab) {
		disabler.disable = false;
		XTestGrabControl(qt_xdisplay(), true); 
	}

	rfbRunEventLoop(server, -1, TRUE);
}

void RFBController::stopServer(bool xtestUngrab) 
{
	rfbScreenCleanup(server);
	state = RFB_STOPPED;
	delete scanner;

	XDestroyImage(framebufferImage);

	if (xtestUngrab) {
		disabler.disable = true;
		QTimer::singleShot(0, &disabler, SLOT(exec()));
	}
}

void RFBController::rebind() 
{
	stopServer(false);
	startServer(false);
}

void RFBController::connectionAccepted(bool aRC)
{
	if (state != RFB_CONNECTING)
		return;

	allowRemoteControl = aRC;
	connectionNum++;
	idleTimer.start(IDLE_PAUSE);

	client->clientGoneHook = clientGoneHook;
	state = RFB_CONNECTED;
	emit sessionEstablished();
}

void RFBController::acceptConnection(bool aRC) 
{
	if (state != RFB_CONNECTING)
		return;

	connectionAccepted(aRC);
	rfbStartOnHoldClient(client);
}

void RFBController::refuseConnection() 
{
	if (state != RFB_CONNECTING)
		return;
	rfbRefuseOnHoldClient(client);
	state = RFB_WAITING;
}

void RFBController::connectionClosed() 
{
	idleTimer.stop();
	connectionNum--;
	state = RFB_WAITING;
	client = 0;
	emit sessionFinished();
}

void RFBController::closeConnection() 
{
	if (state == RFB_CONNECTED) {
		rfbCloseClient(client);
		connectionClosed();
	}
	else if (state == RFB_CONNECTING)
		refuseConnection();
}

void RFBController::idleSlot() 
{
	rfbUndrawCursor(server);

	QList<Hint> v;
	v.setAutoDelete(true);
	scanner->searchUpdates(v);

	Hint *h;

	for (h = v.first(); h != 0; h = v.next()) {
		rfbMarkRectAsModified(server, h->left(),
				      h->top(),
				      h->right(),
				      h->bottom());
	}
	QPoint p = QCursor::pos();
	defaultPtrAddEvent(0, p.x(),p.y(), client);
}

void RFBController::dialogAccepted() 
{
	dialog.hide();
	acceptConnection(dialog.allowRemoteControlCB->isChecked());
}

void RFBController::dialogRefused() 
{
	refuseConnection();
	dialog.hide();
	emit sessionRefused();
}

bool RFBController::handleCheckPassword(const char *, int) 
{
	return TRUE;
	// TODO
}

enum rfbNewClientAction RFBController::handleNewClient(rfbClientPtr cl) 
{
	int socket = cl->sock;

	if ((connectionNum > 0) ||
	    (state != RFB_WAITING)) 
		return RFB_CLIENT_REFUSE;

	client = cl;

	state = RFB_CONNECTING;

	if (!configuration->askOnConnect()) {
		connectionAccepted(configuration->allowDesktopControl());
		return RFB_CLIENT_ACCEPT;
	}

	dialog.allowRemoteControlCB->setChecked(configuration->allowDesktopControl());
	// TODO: get & set client host name

	dialog.show();

	return RFB_CLIENT_ON_HOLD;
}

void RFBController::handleClientGone()
{
	connectionClosed();
}



#define LEFTSHIFT 1
#define RIGHTSHIFT 2
#define ALTGR 4
char ModifierState = 0;

/* this function adjusts the modifiers according to mod (as from modifiers) and ModifierState */

void RFBController::tweakModifiers(char mod, bool down)
{
	Display *dpy = qt_xdisplay();

	bool isShift = ModifierState & (LEFTSHIFT|RIGHTSHIFT);
	if(mod < 0) 
		return;
	
	if(isShift && mod != 1) {
		if(ModifierState & LEFTSHIFT)
			XTestFakeKeyEvent(dpy, leftShiftCode,
					  !down, CurrentTime);
		if(ModifierState & RIGHTSHIFT)
			XTestFakeKeyEvent(dpy, rightShiftCode,
					  !down, CurrentTime);
	}
	
	if(!isShift && mod==1)
		XTestFakeKeyEvent(dpy, leftShiftCode,
				  down, CurrentTime);

	if(ModifierState&ALTGR && mod != 2)
		XTestFakeKeyEvent(dpy, altGrCode,
				  !down, CurrentTime);
	if(!(ModifierState&ALTGR) && mod==2)
		XTestFakeKeyEvent(dpy, altGrCode,
				  down, CurrentTime);
}

void RFBController::initKeycodes()
{
	Display *dpy = qt_xdisplay();
	KeySym key,*keymap;
	int i,j,minkey,maxkey,syms_per_keycode;
	
	memset(modifiers,-1,sizeof(modifiers));
	
	XDisplayKeycodes(dpy,&minkey,&maxkey);
	keymap=XGetKeyboardMapping(dpy,minkey,(maxkey - minkey + 1),&syms_per_keycode);

	for (i = minkey; i <= maxkey; i++)
		for(j=0;j<syms_per_keycode;j++) {
			key=keymap[(i-minkey)*syms_per_keycode+j];
			if(key>=' ' && key<0x100 && i==XKeysymToKeycode(dpy,key)) {
				keycodes[key]=i;
				modifiers[key]=j;
			}
		}
	
	leftShiftCode = XKeysymToKeycode(dpy,XK_Shift_L);
	rightShiftCode = XKeysymToKeycode(dpy,XK_Shift_R);
	altGrCode = XKeysymToKeycode(dpy,XK_Mode_switch);

	XFree ((char *)keymap);
}

void RFBController::handleKeyEvent(bool down, KeySym keySym) {
	if (!allowRemoteControl)
		return;

	Display *dpy = qt_xdisplay();

#define ADJUSTMOD(sym,state) \
  if(keySym==sym) { if(down) ModifierState|=state; else ModifierState&=~state; }
	
	ADJUSTMOD(XK_Shift_L,LEFTSHIFT);
	ADJUSTMOD(XK_Shift_R,RIGHTSHIFT);
	ADJUSTMOD(XK_Mode_switch,ALTGR);

	if(keySym>=' ' && keySym<0x100) {
		KeyCode k;
		if (down)
			tweakModifiers(modifiers[keySym],True);
		//tweakModifiers(modifiers[keySym],down);
		//k = XKeysymToKeycode( dpy,keySym );
		k = keycodes[keySym];
		if(k!=NoSymbol) 
			XTestFakeKeyEvent(dpy,k,down,CurrentTime);

		/*XTestFakeKeyEvent(dpy,keycodes[keySym],down,CurrentTime);*/
		if (down)
			tweakModifiers(modifiers[keySym],False);
	} else {
		KeyCode k = XKeysymToKeycode( dpy,keySym );
		if(k!=NoSymbol)
			XTestFakeKeyEvent(dpy,k,down,CurrentTime);
	}
}

void RFBController::handlePointerEvent(int button_mask, int x, int y) {
	if (!allowRemoteControl)
		return;

	Display *dpy = qt_xdisplay();
  	XTestFakeMotionEvent(dpy, 0, x, y, CurrentTime);
	for(int i = 0; i < 5; i++) 
		if ((buttonMask&(1<<i))!=(button_mask&(1<<i)))
			XTestFakeButtonEvent(dpy,
					     i+1,
					     (button_mask&(1<<i))?True:False,
					     CurrentTime);

	buttonMask = button_mask;
}


XTestDisabler::XTestDisabler() :
	disable(false) {
}

void XTestDisabler::exec() {
	if (disable)
		XTestDiscard(qt_xdisplay());
}

#include "rfbcontroller.moc"
