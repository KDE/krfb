/* This file is part of the KDE project
   Copyright (C) 2017 Alexey Min <alexey.min@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef KRFB_FRAMEBUFFER_XCB_XCB_FRAMEBUFFER_H
#define KRFB_FRAMEBUFFER_XCB_XCB_FRAMEBUFFER_H

#include "framebuffer.h"
#include <QWidget>
#include <xcb/xcb.h>



/**
    @author Alexey Min <alexey.min@gmail.com>
*/
class XCBFrameBuffer: public FrameBuffer
{
    Q_OBJECT
public:
    XCBFrameBuffer(WId winid, QObject *parent = 0);
    ~XCBFrameBuffer();

public:
    QList<QRect> modifiedTiles() Q_DECL_OVERRIDE;
    int  depth() Q_DECL_OVERRIDE;
    int  height() Q_DECL_OVERRIDE;
    int  width() Q_DECL_OVERRIDE;
    int  paddedWidth() Q_DECL_OVERRIDE;
    void getServerFormat(rfbPixelFormat &format) Q_DECL_OVERRIDE;
    void startMonitor() Q_DECL_OVERRIDE;
    void stopMonitor() Q_DECL_OVERRIDE;

public:
    void handleXDamageNotify(xcb_generic_event_t *xevent);

private:
    void cleanupRects();

    class P;
    P *const d;
};

#endif
