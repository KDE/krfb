/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef QTFRAMEBUFFER_H
#define QTFRAMEBUFFER_H

#include <QImage>
#include "framebuffer.h"

class QTimer;
/**
	@author Alessandro Praduroux <pradu@pradu.it>
*/
class QtFrameBuffer : public FrameBuffer
{
Q_OBJECT
public:
    QtFrameBuffer(WId id, QObject *parent = 0);

    ~QtFrameBuffer();

    virtual int depth();
    virtual int height();
    virtual int width();
    virtual int paddedWidth();
    virtual void getServerFormat(rfbPixelFormat& format);
    virtual void startMonitor();
    virtual void stopMonitor();

public Q_SLOTS:
    void updateFrameBuffer();

private:
    QImage fbImage;
    QTimer *t;
};

#endif
