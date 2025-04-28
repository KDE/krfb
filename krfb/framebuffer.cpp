/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "framebuffer.h"

#include <config-krfb.h>
#include <QCursor>
#include <QGuiApplication>
#include <QScreen>


FrameBuffer::FrameBuffer(QObject *parent)
    : QObject(parent)
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

QVariant FrameBuffer::customProperty(const QString &property) const
{
    Q_UNUSED(property)
    return QVariant();
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

QPoint FrameBuffer::cursorPosition()
{
    QPoint cursorPos = QCursor::pos();
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        qreal scaleFactor = primaryScreen->devicePixelRatio();
        cursorPos.setX(qRound(cursorPos.x() * scaleFactor));
        cursorPos.setY(qRound(cursorPos.y() * scaleFactor));
    } else {
        qWarning() << "cursorPosition: ERROR: Failed to get application's primary screen info!";
    }
    return cursorPos;
}
