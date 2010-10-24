/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "x11framebuffer.h"
#include "x11framebuffer.moc"

#include <config-krfb.h>

#include <QX11Info>
#include <QApplication>
#include <QDesktopWidget>

#include <KApplication>
#include <KDebug>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef HAVE_XDAMAGE
#include <X11/extensions/Xdamage.h>
#endif

#ifdef HAVE_XSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

class X11FrameBuffer::P
{

public:
#ifdef HAVE_XDAMAGE
    Damage damage;
#endif
#ifdef HAVE_XSHM
    XShmSegmentInfo shminfo;
#endif

    XImage *framebufferImage;
    XImage *updateTile;
    EvWidget *ev;
    bool useShm;
    int xdamageBaseEvent;
    bool running;
};

X11FrameBuffer::X11FrameBuffer(WId id, QObject *parent)
    : FrameBuffer(id, parent), d(new X11FrameBuffer::P)
{
#ifdef HAVE_XSHM
    d->useShm = XShmQueryExtension(QX11Info::display());
    kDebug() << "shm: " << d->useShm;
#else
    d->useShm = false;
#endif
    d->running = false;
    d->framebufferImage = XGetImage(QX11Info::display(),
                                    id,
                                    0,
                                    0,
                                    QApplication::desktop()->width(), //arg, must get a widget ???
                                    QApplication::desktop()->height(),
                                    AllPlanes,
                                    ZPixmap);

    if (d->useShm) {
#ifdef HAVE_XSHM
        d->updateTile = XShmCreateImage(QX11Info::display(),
                                        DefaultVisual(QX11Info::display(), 0),
                                        d->framebufferImage->bits_per_pixel,
                                        ZPixmap,
                                        NULL,
                                        &d->shminfo,
                                        32,
                                        32);
        d->shminfo.shmid = shmget(IPC_PRIVATE,
                                  d->updateTile->bytes_per_line * d->updateTile->height,
                                  IPC_CREAT | 0777);

        d->shminfo.shmaddr = d->updateTile->data = (char *)
                             shmat(d->shminfo.shmid, 0, 0);
        d->shminfo.readOnly = False;

        XShmAttach(QX11Info::display(), &d->shminfo);
#endif
    } else {
        ;
    }

    kDebug() << "Got image. bpp: " << d->framebufferImage->bits_per_pixel
             << ", depth: " << d->framebufferImage->depth
             << ", padded width: " << d->framebufferImage->bytes_per_line
             << " (sent: " << d->framebufferImage->width * 4 << ")"
             << endl;

    fb = d->framebufferImage->data;
#ifdef HAVE_XDAMAGE
    d->ev = new EvWidget(this);
    kapp->installX11EventFilter(d->ev);
#endif
}


X11FrameBuffer::~X11FrameBuffer()
{
    XDestroyImage(d->framebufferImage);
#ifdef HAVE_XDAMAGE
    kapp->removeX11EventFilter(d->ev);
#endif
#ifdef HAVE_XSHM
    XShmDetach(QX11Info::display(), &d->shminfo);
    XDestroyImage(d->updateTile);
    shmdt(d->shminfo.shmaddr);
    shmctl(d->shminfo.shmid, IPC_RMID, 0);
#endif
    delete d;
    fb = 0; // already deleted by XDestroyImage
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

void X11FrameBuffer::getServerFormat(rfbPixelFormat &format)
{
    format.bitsPerPixel = d->framebufferImage->bits_per_pixel;
    format.depth = d->framebufferImage->depth;
    format.trueColour = true;
    format.bigEndian = ((d->framebufferImage->bitmap_bit_order == MSBFirst) ? true : false);

    if (format.bitsPerPixel == 8) {
        format.redShift = 0;
        format.greenShift = 3;
        format.blueShift = 6;
        format.redMax   = 7;
        format.greenMax = 7;
        format.blueMax  = 3;
    } else {
        format.redShift = 0;

        if (d->framebufferImage->red_mask)
            while (!(d->framebufferImage->red_mask & (1 << format.redShift))) {
                format.redShift++;
            }

        format.greenShift = 0;

        if (d->framebufferImage->green_mask)
            while (!(d->framebufferImage->green_mask & (1 << format.greenShift))) {
                format.greenShift++;
            }

        format.blueShift = 0;

        if (d->framebufferImage->blue_mask)
            while (!(d->framebufferImage->blue_mask & (1 << format.blueShift))) {
                format.blueShift++;
            }

        format.redMax   = d->framebufferImage->red_mask   >> format.redShift;
        format.greenMax = d->framebufferImage->green_mask >> format.greenShift;
        format.blueMax  = d->framebufferImage->blue_mask  >> format.blueShift;
    }
}

void X11FrameBuffer::handleXDamage(XEvent *event)
{
#ifdef HAVE_XDAMAGE
    XDamageNotifyEvent *dev = (XDamageNotifyEvent *)event;
    QRect r(dev->area.x, dev->area.y, dev->area.width, dev->area.height);
    tiles.append(r);

    /*if (!dev->more) {
        XDamageSubtract(QX11Info::display(),d->damage, None, None);
    }*/
#endif
}


void X11FrameBuffer::cleanupRects()
{

    QList<QRect> cpy = tiles;
    bool inserted = false;
    tiles.clear();
//     kDebug() << "before cleanup: " << cpy.size();
    foreach(const QRect & r, cpy) {
        if (tiles.size() > 0) {
            for (int i = 0; i < tiles.size(); i++) {
                //             kDebug() << r << tiles[i];
                if (r.intersects(tiles[i])) {
                    tiles[i] |= r;
                    inserted = true;
                    break;
                    //                 kDebug() << "merged into " << tiles[i];
                }
            }

            if (!inserted) {
                tiles.append(r);
                //             kDebug() << "appended " << r;
            }
        } else {
            //         kDebug() << "appended " << r;
            tiles.append(r);
        }
    }

    for (int i = 0; i < tiles.size(); i++) {
        tiles[i].adjust(-30, -30, 30, 30);

        if (tiles[i].top() < 0) {
            tiles[i].setTop(0);
        }

        if (tiles[i].left() < 0) {
            tiles[i].setLeft(0);
        }

        if (tiles[i].bottom() > d->framebufferImage->height) {
            tiles[i].setBottom(d->framebufferImage->height);
        }

        if (tiles[i].right() > d->framebufferImage->width) {
            tiles[i].setRight(d->framebufferImage->width);
        }
    }

//     kDebug() << "after cleanup: " << tiles.size();
}

void X11FrameBuffer::acquireEvents()
{

    XEvent ev;

    while (XCheckTypedEvent(QX11Info::display(), d->xdamageBaseEvent + XDamageNotify, &ev)) {
        handleXDamage(&ev);
    }

    XDamageSubtract(QX11Info::display(), d->damage, None, None);
}

QList< QRect > X11FrameBuffer::modifiedTiles()
{
    QList<QRect> ret;

    if (!d->running) {
        return ret;
    }

    kapp->processEvents(); // try to make sure every damage event goes trough;
    cleanupRects();
    QRect gl;

    //d->useShm = false;
    if (tiles.size()  > 0) {
        if (d->useShm) {
#ifdef HAVE_XSHM

            foreach(const QRect & r, tiles) {
//                 kDebug() << r;
                gl |= r;
                int y = r.y();
                int x = r.x();

                while (x < r.right()) {
                    while (y < r.bottom()) {
                        if (y + d->updateTile->height > d->framebufferImage->height) {
                            y = d->framebufferImage->height - d->updateTile->height;
                        }

                        if (x + d->updateTile->width > d->framebufferImage->width) {
                            x = d->framebufferImage->width - d->updateTile->width;
                        }

//                         kDebug() << "x: " << x << " (" << r.x() << ") y: " << y << " (" << r.y() << ") " << r;
                        XShmGetImage(QX11Info::display(), win, d->updateTile, x, y, AllPlanes);
                        int pxsize =  d->framebufferImage->bits_per_pixel / 8;
                        char *dest = fb + ((d->framebufferImage->bytes_per_line * y) + (x * pxsize));
                        char *src = d->updateTile->data;

                        for (int i = 0; i < d->updateTile->height; i++) {
                            memcpy(dest, src, d->updateTile->bytes_per_line);
                            dest += d->framebufferImage->bytes_per_line;
                            src += d->updateTile->bytes_per_line;
                        }

                        y += d->updateTile->height;
                    }

                    x += d->updateTile->width;
                    y = r.y();
                }
            }
#endif
        } else {
            foreach(const QRect & r, tiles) {
                XGetSubImage(QX11Info::display(),
                             win,
                             r.left(),
                             r.top(),
                             r.width(),
                             r.height(),
                             AllPlanes,
                             ZPixmap,
                             d->framebufferImage,
                             r.left(),
                             r.top()
                            );
            }
        }
    }

//     kDebug() << "tot: " << gl;
//     kDebug() << tiles.size();
    ret = tiles;
    tiles.clear();
    return ret;
}

void X11FrameBuffer::startMonitor()
{
    d->running = true;
#ifdef HAVE_XDAMAGE
    d->damage = XDamageCreate(QX11Info::display(), win, XDamageReportRawRectangles);
    XDamageSubtract(QX11Info::display(), d->damage, None, None);
#endif
}

void X11FrameBuffer::stopMonitor()
{
    d->running = false;
#ifdef HAVE_XDAMAGE
    XDamageDestroy(QX11Info::display(), d->damage);
#endif
}



EvWidget::EvWidget(X11FrameBuffer *x11fb)
    : QWidget(0), fb(x11fb)
{
#ifdef HAVE_XDAMAGE
    int er;
    XDamageQueryExtension(QX11Info::display(), &xdamageBaseEvent, &er);
#endif
}

bool EvWidget::x11Event(XEvent *event)
{
#ifdef HAVE_XDAMAGE

    if (event->type == xdamageBaseEvent + XDamageNotify) {
        fb->handleXDamage(event);
        return true;
    }

#endif
    return false;
}



