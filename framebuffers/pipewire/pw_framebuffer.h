/* This file is part of the KDE project
   Copyright (C) 2018 Oleg Chernovskiy <kanedias@xaker.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
*/
#ifndef KRFB_FRAMEBUFFER_XCB_XCB_FRAMEBUFFER_H
#define KRFB_FRAMEBUFFER_XCB_XCB_FRAMEBUFFER_H

#include "framebuffer.h"
#include <QWidget>
#include <QVariantMap>

/**
 * @brief The PWFrameBuffer class - framebuffer implementation based on XDG Desktop Portal ScreenCast interface.
 *        The design relies heavily on a presence of XDG D-Bus service and PipeWire daemon.
 *
 * @author Oleg Chernovskiy <kanedias@xaker.ru>
 */
class PWFrameBuffer: public FrameBuffer
{
    Q_OBJECT
public:
    PWFrameBuffer(WId winid, QObject *parent = nullptr);
    virtual ~PWFrameBuffer() override;

    int  depth() override;
    int  height() override;
    int  width() override;
    void getServerFormat(rfbPixelFormat &format) override;
    void startMonitor() override;
    void stopMonitor() override;

    bool isValid() const;

private slots:
    void handleXdpSessionCreated(quint32 code, QVariantMap results);
    void handleXdpSourcesSelected(quint32 code, QVariantMap results);
    void handleXdpScreenCastStarted(quint32 code, QVariantMap results);

private:
    void processPwEvents();

    class Private;
    const QScopedPointer<Private> d;
};

#endif
