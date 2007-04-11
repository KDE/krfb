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
#include <QImage>
#include <QRect>
#include <QVector>

#include <rfb/rfb.h>

/**
	@author Alessandro Praduroux <pradu@pradu.it>
*/
class FrameBuffer : public QObject
{
Q_OBJECT
public:
    explicit FrameBuffer(WId id, QObject *parent = 0);

    ~FrameBuffer();

    char * data();

    QVector<QRect> &modifiedTiles();
    int width();
    int height();
    int depth();

    void getServerFormat(rfbPixelFormat &format);

public Q_SLOTS:
    void updateFrameBuffer();

private:
    WId win;
    char *fb;
    QImage fbImage;
    QVector<QRect> tiles;

};

#endif
