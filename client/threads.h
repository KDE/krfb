/***************************************************************************
                          threads.h  -  threads for kvncview
                             -------------------
    begin                : Thu May 09 16:01:42 CET 2002
    copyright            : (C) 2002 by Tim Jansen
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

#ifndef THREADS_H
#define THREADS_H

#ifndef VNCVIEWER_H
#define VNCVIEWER_H
#include "vncviewer.h"
#endif

#include <qthread.h>
#include <qregion.h>
#include <qrect.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qevent.h>
#include <qvaluelist.h>

class KVncView;

enum RemoteViewStatus {
	REMOTE_VIEW_CONNECTING,
	REMOTE_VIEW_AUTHENTICATING,
	REMOTE_VIEW_PREPARING,
	REMOTE_VIEW_CONNECTED,
	REMOTE_VIEW_DISCONNECTED
};



const int ScreenResizeEventType = 781001;

class ScreenResizeEvent : public QCustomEvent
{
private:
	int m_width, m_height;
public:
	ScreenResizeEvent(int w, int h) : 
		QCustomEvent(ScreenResizeEventType), 
		m_width(w),
		m_height(h) 
	{};
	int width() const { return m_width; };
	int height() const { return m_height; };
};

const int StatusChangeEventType = 781002;

class StatusChangeEvent : public QCustomEvent
{
private:
	RemoteViewStatus m_status;
public:
	StatusChangeEvent(RemoteViewStatus s) :
		QCustomEvent(StatusChangeEventType),
		m_status(s)
	{};
	RemoteViewStatus status() const { return m_status; };
};

const int PasswordRequiredEventType = 781003;

class PasswordRequiredEvent : public QCustomEvent
{
public:
	PasswordRequiredEvent() : 
		QCustomEvent(PasswordRequiredEventType)
	{};
};


struct MouseEvent {
	int x, y, buttons;
};

struct KeyEvent {
	unsigned int k;
	bool down;
};


class WriterThread : public QThread {
private:
	QMutex m_lock;
	QWaitCondition m_waiter;
	volatile bool &m_quitFlag;

	// all things that can be send follow:
	int m_incrementalUpdateRQ; // for sending an incremental request
	QRegion m_updateRegionRQ;  // for sending updates, null if it is done
	QValueList<MouseEvent> m_mouseEvents; // list of unsent mouse events
	QValueList<KeyEvent> m_keyEvents;     // list of unsent key events
	
public:
	WriterThread(volatile bool &quitFlag);
	
	void queueIncrementalUpdateRequest();
	void queueUpdateRequest(const QRegion &r);
	void queueMouseEvent(int x, int y, int buttonMask);
	void queueKeyEvent(unsigned int k, bool down);
	void kick();
	
protected:
	void run();
	bool sendIncrementalUpdateRequest();
	bool sendUpdateRequest(const QRegion &r);
	bool sendMouseEvents(const QValueList<MouseEvent> &events);
	bool sendKeyEvents(const QValueList<KeyEvent> &events);
};



class ControllerThread : public QThread { 
private:
	KVncView *m_view;
	enum RemoteViewStatus m_status;
	WriterThread &m_wthread;
	volatile bool &m_quitFlag;

	void changeStatus(RemoteViewStatus s);
public:

	ControllerThread(KVncView *v, WriterThread &wt, volatile bool &quitFlag);
	enum RemoteViewStatus status();	
protected:
	void run();
};



#endif
