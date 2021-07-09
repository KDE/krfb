/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "qtframebuffer.h"

#include <QTimer>
#include <QRegion>
#include <QPixmap>
#include <QBitmap>
#include <QGuiApplication>
#include <QScreen>


const int UPDATE_TIME = 500;

QtFrameBuffer::QtFrameBuffer(WId id, QObject *parent)
    : FrameBuffer(id, parent)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        primaryScreen = screen;
        fbImage = screen->grabWindow(win).toImage();
        fb = new char[fbImage.sizeInBytes()];
    } else {
        fb = nullptr;
        primaryScreen = nullptr;
    }

    t = new QTimer(this);
    connect(t, &QTimer::timeout, this, &QtFrameBuffer::updateFrameBuffer);
}


QtFrameBuffer::~QtFrameBuffer()
{
    if (fb)
        delete [] fb;
    fb = nullptr;
}

int QtFrameBuffer::depth()
{
    return fbImage.depth();
}

int QtFrameBuffer::height()
{
    return fbImage.height();
}

int QtFrameBuffer::width()
{
    return fbImage.width();
}

void QtFrameBuffer::getServerFormat(rfbPixelFormat &format)
{
    format.bitsPerPixel = 32;
    format.depth = 32;
    format.trueColour = true;

    format.bigEndian = false;
    format.redShift = 16;
    format.greenShift = 8;
    format.blueShift = 0;
    format.redMax   = 0xff;
    format.greenMax = 0xff;
    format.blueMax  = 0xff;
}

void QtFrameBuffer::updateFrameBuffer()
{
    if (!fb || !primaryScreen) return;
    QImage img = primaryScreen->grabWindow(win).toImage();
#if 0 // This is actually slower than updating the whole desktop...
    QSize imgSize = img.size();


    // verify what part of the image need to be marked as changed
    // fbImage is the previous version of the image,
    // img is the current one

    QImage map(imgSize, QImage::Format_Mono);
    map.fill(0);

    for (int x = 0; x < imgSize.width(); x++) {
        for (int y = 0; y < imgSize.height(); y++) {
            if (img.pixel(x, y) != fbImage.pixel(x, y)) {
                map.setPixel(x, y, 1);
            }
        }
    }

    QRegion r(QBitmap::fromImage(map));
    tiles = tiles + r.rects();

#else
    tiles.append(img.rect());
#endif

    memcpy(fb, img.bits(), static_cast<size_t>(img.sizeInBytes()));
    fbImage = img;

}

int QtFrameBuffer::paddedWidth()
{
    return fbImage.width() * 4;
}

void QtFrameBuffer::startMonitor()
{
    t->start(UPDATE_TIME);
}

void QtFrameBuffer::stopMonitor()
{
    t->stop();
}

