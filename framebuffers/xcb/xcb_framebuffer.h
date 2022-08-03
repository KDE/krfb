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
    explicit XCBFrameBuffer(QObject *parent = nullptr);
    ~XCBFrameBuffer() override;

public:
    QList<QRect> modifiedTiles() override;
    int  depth() override;
    int  height() override;
    int  width() override;
    int  paddedWidth() override;
    void getServerFormat(rfbPixelFormat &format) override;
    void startMonitor() override;
    void stopMonitor() override;

public:
    void handleXDamageNotify(xcb_generic_event_t *xevent);

private:
    void cleanupRects();

    class P;
    P *const d;
};

#endif
