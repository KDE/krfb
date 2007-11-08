/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "framebuffer.h"
#include "framebuffer.moc"

#include <config-krfb.h>

#include <QX11Info>

#include "qtframebuffer.h"
#include "x11framebuffer.h"

#include <X11/Xutil.h>

#ifdef HAVE_XDAMAGE
#include <X11/extensions/Xdamage.h>
#endif


FrameBuffer::FrameBuffer(WId id, QObject *parent)
 : QObject(parent), win(id)
{
    //TODO: implement reference counting to avoid update the screen
    // while no client is connected.
}


FrameBuffer::~FrameBuffer()
{
    delete fb;
}

char * FrameBuffer::data()
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

FrameBuffer * FrameBuffer::getFrameBuffer(WId id, QObject * parent)
{
#ifdef HAVE_XDAMAGE
    int tmp, er;
    if (XDamageQueryExtension(QX11Info::display(), &tmp, &er)) {
        return new X11FrameBuffer(id, parent);
    }
#endif
    return new QtFrameBuffer(id, parent);

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


