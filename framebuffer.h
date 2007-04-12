/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <QObject>
#include <QRect>
#include <QVector>
#include <QWidget>

#include <rfb/rfb.h>

class FrameBuffer;
/**
	@author Alessandro Praduroux <pradu@pradu.it>
*/
class FrameBuffer : public QObject
{
Q_OBJECT
public:

    static FrameBuffer* getFrameBuffer(WId id, QObject *parent);

    virtual ~FrameBuffer();

    char * data();

    QVector<QRect> &modifiedTiles();
    virtual int paddedWidth();
    virtual int width();
    virtual int height();
    virtual int depth();

    virtual void getServerFormat(rfbPixelFormat &format);

protected:
    explicit FrameBuffer(WId id, QObject *parent = 0);

    WId win;
    char *fb;
    QVector<QRect> tiles;

};

#endif
