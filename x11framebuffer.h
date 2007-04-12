/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/
#ifndef X11FRAMEBUFFER_H
#define X11FRAMEBUFFER_H

#include <framebuffer.h>
#include <QWidget>

class X11FrameBuffer;

class EvWidget: public QWidget {
    Q_OBJECT

public:
    EvWidget(X11FrameBuffer *x11fb);

protected:
    bool x11Event ( XEvent * event );

private:
    X11FrameBuffer *fb;
    int xdamageBaseEvent;
};

/**
	@author Alessandro Praduroux <pradu@pradu.it>
*/
class X11FrameBuffer : public FrameBuffer
{
Q_OBJECT
public:
    X11FrameBuffer(WId id, QObject* parent);

    ~X11FrameBuffer();

    virtual int depth();
    virtual int height();
    virtual int width();
    virtual int paddedWidth();
    virtual void getServerFormat(rfbPixelFormat& format);

    void handleXDamage( XEvent *event);
private:
    class P;
    P * const d;
};

#endif
