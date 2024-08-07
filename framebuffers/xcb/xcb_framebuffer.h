/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2017 Alexey Min <alexey.min@gmail.com>

   SPDX-License-Identifier: GPL-2.0-or-later
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
