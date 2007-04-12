/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "x11framebuffer.h"
#include "x11framebuffer.moc"
#include "config-krfb.h"

#include <QX11Info>
#include <QApplication>
#include <QDesktopWidget>

#include <KApplication>

#include <X11/Xutil.h>

#ifdef HAVE_XDAMAGE
#include <X11/extensions/Xdamage.h>
#endif

class X11FrameBuffer::P {

public:
#ifdef HAVE_XDAMAGE
    Damage damage;
#endif
    XImage *framebufferImage;
    EvWidget *ev;
};

X11FrameBuffer::X11FrameBuffer(WId id, QObject* parent)
    :FrameBuffer(id, parent), d(new X11FrameBuffer::P)
{
    d->framebufferImage = XGetImage(QX11Info::display(),
                                    id,
                                    0,
                                    0,
                                    QApplication::desktop()->width(), //arg, must get a widget ???
                                    QApplication::desktop()->height(),
                                    AllPlanes,
                                    ZPixmap);

    kDebug() << "Got image. bpp: " << d->framebufferImage->bits_per_pixel
            << ", depth: " << d->framebufferImage->depth
            << ", padded width: " << d->framebufferImage->bytes_per_line
            << " (sent: " << d->framebufferImage->width * 4 << ")"
            << endl;

    fb = d->framebufferImage->data;
#ifdef HAVE_XDAMAGE
    d->ev = new EvWidget(this);
    d->damage = XDamageCreate(QX11Info::display(), id, XDamageReportRawRectangles);
    XDamageSubtract(QX11Info::display(),d->damage, None, None);
    kapp->installX11EventFilter(d->ev);
#endif
}


X11FrameBuffer::~X11FrameBuffer()
{
    XDestroyImage(d->framebufferImage);
#ifdef HAVE_XDAMAGE
    kapp->removeX11EventFilter(d->ev);
    XDamageDestroy(QX11Info::display(),d->damage);
#endif
    delete d;
}


int X11FrameBuffer::depth()
{
    return d->framebufferImage->depth;
}

int X11FrameBuffer::height()
{
    return d->framebufferImage->height;
}

int X11FrameBuffer::width()
{
    return d->framebufferImage->width;
}

int X11FrameBuffer::paddedWidth()
{
    return d->framebufferImage->bytes_per_line;
}

void X11FrameBuffer::getServerFormat(rfbPixelFormat& format)
{
    format.bitsPerPixel = d->framebufferImage->bits_per_pixel;
    format.depth = d->framebufferImage->depth;
    format.trueColour = true;
    format.bigEndian = ((d->framebufferImage->bitmap_bit_order == MSBFirst) ? true : false);

    if ( format.bitsPerPixel == 8 ) {
        format.redShift = 0;
        format.greenShift = 3;
        format.blueShift = 6;
        format.redMax   = 7;
        format.greenMax = 7;
        format.blueMax  = 3;
    } else {
        format.redShift = 0;
        if ( d->framebufferImage->red_mask )
            while ( ! ( d->framebufferImage->red_mask & (1 << format.redShift) ) )
                format.redShift++;
        format.greenShift = 0;
        if ( d->framebufferImage->green_mask )
            while ( ! ( d->framebufferImage->green_mask & (1 << format.greenShift) ) )
                format.greenShift++;
        format.blueShift = 0;
        if ( d->framebufferImage->blue_mask )
            while ( ! ( d->framebufferImage->blue_mask & (1 << format.blueShift) ) )
                format.blueShift++;
        format.redMax   = d->framebufferImage->red_mask   >> format.redShift;
        format.greenMax = d->framebufferImage->green_mask >> format.greenShift;
        format.blueMax  = d->framebufferImage->blue_mask  >> format.blueShift;
    }
}

void X11FrameBuffer::handleXDamage(XEvent * event)
{
#ifdef HAVE_XDAMAGE
    XDamageNotifyEvent *dev = (XDamageNotifyEvent *)event;
    QRect r(dev->area.x, dev->area.y, dev->area.width, dev->area.height);
    tiles.append(r);

    XGetSubImage(QX11Info::display(),
                 win,
                 dev->area.x,
                 dev->area.y,
                 dev->area.width,
                 dev->area.height,
                 AllPlanes,
                 ZPixmap,
                 d->framebufferImage,
                 dev->area.x,
                 dev->area.y);

    if (!dev->more) {
        XDamageSubtract(QX11Info::display(),d->damage, None, None);
    }
#endif
}


EvWidget::EvWidget(X11FrameBuffer * x11fb)
    :QWidget(0), fb(x11fb)
{
#ifdef HAVE_XDAMAGE
    int er;
    XDamageQueryExtension(QX11Info::display(), &xdamageBaseEvent, &er);
#endif
}

bool EvWidget::x11Event(XEvent * event)
{
#ifdef HAVE_XDAMAGE
    if (event->type == xdamageBaseEvent+XDamageNotify) {
        fb->handleXDamage(event);
        return true;
    }
#endif
    return false;
}



