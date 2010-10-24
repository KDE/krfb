/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "framebuffer.h"

#include "config-krfb.h"

#include <X11/Xutil.h>


FrameBuffer::FrameBuffer(WId id, QObject *parent)
    : QObject(parent), win(id)
{
}

FrameBuffer::~FrameBuffer()
{
    delete fb;
}

char *FrameBuffer::data()
{
    return fb;
}

QList< QRect > FrameBuffer::modifiedTiles()
{
    QList<QRect> ret = tiles;
    tiles.clear();
    return ret;
}

int FrameBuffer::width()
{
    return 0;
}

int FrameBuffer::height()
{
    return 0;
}

void FrameBuffer::getServerFormat(rfbPixelFormat &)
{
}

int FrameBuffer::depth()
{
    return 32;
}

int FrameBuffer::paddedWidth()
{
    return width() * depth() / 8;
}

void FrameBuffer::startMonitor()
{
}

void FrameBuffer::stopMonitor()
{
}


#include "framebuffer.moc"

