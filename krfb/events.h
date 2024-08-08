/*
   This file is part of the KDE project

   SPDX-FileCopyrightText: 2010 Collabora Ltd.
   SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
   SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>
   SPDX-FileCopyrightText: 2001-2003 Tim Jansen <tim@tjansen.de>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EVENTS_H
#define EVENTS_H

#include "framebuffer.h"
#include "krfbprivate_export.h"
#include "rfb.h"

#include <QObject>

class KRFBPRIVATE_EXPORT EventHandler : public QObject
{
    Q_OBJECT
public:
    explicit EventHandler(QObject *parent = nullptr);
    ~EventHandler() override = default;
    virtual void handleKeyboard(bool down, rfbKeySym key) = 0;
    virtual void handlePointer(int buttonMask, int x, int y) = 0;

    void setFrameBufferPlugin(const QSharedPointer<FrameBuffer> &frameBuffer);
    QSharedPointer<FrameBuffer> frameBuffer();

private:
    // Used to track framebuffer plugin which we need for xdp event plugin
    QSharedPointer<FrameBuffer> fb;
};

#endif
