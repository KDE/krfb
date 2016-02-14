/* This file is part of the KDE project
   Copyright (C) 2016 Oleg Chernovskiy <adonai@xaker.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KRFB_FRAMEBUFFER_GBM_GBMFRAMEBUFFER_H
#define KRFB_FRAMEBUFFER_GBM_GBMFRAMEBUFFER_H

#include "framebuffer.h"
// Qt
#include <QImage>
// system
#include <gbm.h>
#include <epoxy/egl.h>
#include <epoxy/gl.h>

namespace KWayland
{
namespace Client
{
class RemoteAccessManager;
class RemoteBuffer;
}
}

/**
    @author Oleg Chernovskiy <adonai@xaker.ru>
*/
class GbmFrameBuffer : public FrameBuffer
{
    Q_OBJECT
public:
    explicit GbmFrameBuffer(WId id, QObject *parent = 0);

    ~GbmFrameBuffer();

    int depth() override;
    int height() override;
    int width() override;
    int paddedWidth() override;
    void getServerFormat(rfbPixelFormat &format) override;
    void startMonitor() override;
    void stopMonitor() override;
    inline bool isValid() { return m_valid; };

public Q_SLOTS:
    void updateFrameBuffer();

private:
    void initWaylandConnection();
    void setupDrm();
    void initClientEglExtensions();
    void initEgl();
    void obtainBuffer(KWayland::Client::RemoteBuffer *rbuf);
    void updateHandle(qint32 gbmHandle, quint32 width, quint32 height, quint32 stride, quint32 format);

    qint32 m_drmFd = 0; // for GBM buffer mmap
    gbm_device *m_gbmDevice = nullptr; // for passed GBM buffer retrieval
    struct {
        QList<QByteArray> extensions;
        EGLDisplay display = EGL_NO_DISPLAY;
        EGLContext context = EGL_NO_CONTEXT;
    } m_egl;

    KWayland::Client::RemoteAccessManager *m_interface = nullptr;
    QImage *m_img;

    bool m_valid = true;
};

#endif
