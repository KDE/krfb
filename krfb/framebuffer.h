/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "rfb.h"

#include <kdemacros.h>

#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtCore/QList>

#include <QtGui/QWidget>

class FrameBuffer;
/**
    @author Alessandro Praduroux <pradu@pradu.it>
*/
class KDE_EXPORT FrameBuffer : public QObject
{
    Q_OBJECT
public:
    explicit FrameBuffer(WId id, QObject *parent = 0);

    virtual ~FrameBuffer();

    char *data();

    virtual QList<QRect> modifiedTiles();
    virtual int paddedWidth();
    virtual int width();
    virtual int height();
    virtual int depth();
    virtual void startMonitor();
    virtual void stopMonitor();

    virtual void getServerFormat(rfbPixelFormat &format);

protected:
    WId win;
    char *fb;
    QList<QRect> tiles;

private:
    Q_DISABLE_COPY(FrameBuffer)

};

#endif
