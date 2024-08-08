/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "rfb.h"

#include "krfbprivate_export.h"

#include <QList>
#include <QObject>
#include <QRect>
#include <QVariant>
#include <QWidget>

class FrameBuffer;
/**
    @author Alessandro Praduroux <pradu@pradu.it>
*/
class KRFBPRIVATE_EXPORT FrameBuffer : public QObject
{
    Q_OBJECT
public:
    explicit FrameBuffer(QObject *parent = nullptr);

    ~FrameBuffer() override;

    char *data();

    virtual QList<QRect> modifiedTiles();
    virtual int paddedWidth();
    virtual int width();
    virtual int height();
    virtual int depth();
    virtual void startMonitor();
    virtual void stopMonitor();
    virtual QPoint cursorPosition();

    virtual void getServerFormat(rfbPixelFormat &format);

    virtual QVariant customProperty(const QString &property) const;

Q_SIGNALS:
    void frameBufferChanged();

protected:
    char *fb = nullptr;
    QList<QRect> tiles;

private:
    Q_DISABLE_COPY(FrameBuffer)
};

#endif
