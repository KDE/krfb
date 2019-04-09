/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KRFB_FRAMEBUFFER_QT_QTFRAMEBUFFER_H
#define KRFB_FRAMEBUFFER_QT_QTFRAMEBUFFER_H

#include <QImage>
#include "framebuffer.h"

class QTimer;
class QScreen;
/**
    @author Alessandro Praduroux <pradu@pradu.it>
*/
class QtFrameBuffer : public FrameBuffer
{
    Q_OBJECT
public:
    explicit QtFrameBuffer(WId id, QObject *parent = nullptr);

    ~QtFrameBuffer() override;

    int depth() override;
    int height() override;
    int width() override;
    int paddedWidth() override;
    void getServerFormat(rfbPixelFormat &format) override;
    void startMonitor() override;
    void stopMonitor() override;

public Q_SLOTS:
    void updateFrameBuffer();

private:
    QImage fbImage;
    QTimer *t;
    QScreen *primaryScreen;
};

#endif
