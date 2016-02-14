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


#include "gbmframebuffer.h"
#include "logging.h"
#include "gbmframebuffer.moc"
// KWayland
#include <KWayland/Client/registry.h>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/remote_access.h>
// Qt
#include <QApplication>
#include <QScreen>
#include <QtCore/QThread>
// system
#include <fcntl.h>
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_mode.h>

static QString formatGLError(GLenum err)
{
    switch(err) {
    case GL_NO_ERROR:          return QStringLiteral("GL_NO_ERROR");
    case GL_INVALID_ENUM:      return QStringLiteral("GL_INVALID_ENUM");
    case GL_INVALID_VALUE:     return QStringLiteral("GL_INVALID_VALUE");
    case GL_INVALID_OPERATION: return QStringLiteral("GL_INVALID_OPERATION");
    case GL_STACK_OVERFLOW:    return QStringLiteral("GL_STACK_OVERFLOW");
    case GL_STACK_UNDERFLOW:   return QStringLiteral("GL_STACK_UNDERFLOW");
    case GL_OUT_OF_MEMORY:     return QStringLiteral("GL_OUT_OF_MEMORY");
    default: return QLatin1String("0x") + QString::number(err, 16);
    }
}

GbmFrameBuffer::GbmFrameBuffer(WId id, QObject *parent)
    : FrameBuffer(id, parent)
{

    // TODO: check out the case when new resolution was applied on KWin side
    // TODO: check possibility of multi-screen configuration
    QSize size = QApplication::screens().first()->size();
    m_img = new QImage(size, QImage::Format_ARGB32);
    fb = reinterpret_cast<char*>(m_img->bits());

    // open DRM device
    setupDrm();

    // get EGL client extensions
    initClientEglExtensions();

    // initialize EGL context and display
    initEgl();

    // get KWin connection
    initWaylandConnection();
}

void GbmFrameBuffer::setupDrm()
{
    m_drmFd = open("/dev/dri/card0", O_RDWR);
    m_gbmDevice = gbm_create_device(m_drmFd);
    if(!m_gbmDevice) {
        qCCritical(KRFB_GBM) << "Cannot create GBM device:" << strerror(errno);
        m_valid = false;
        return;
    }
}

// cloned from KWin AbstractEglBackend
void GbmFrameBuffer::initClientEglExtensions()
{
    if(!m_valid)
        return;

    // Get the list of client extensions
    const char* clientExtensionsCString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    const QByteArray clientExtensionsString = QByteArray::fromRawData(clientExtensionsCString, qstrlen(clientExtensionsCString));
    if (clientExtensionsString.isEmpty()) {
        // If eglQueryString() returned NULL, the implementation doesn't support
        // EGL_EXT_client_extensions. Expect an EGL_BAD_DISPLAY error.
        qCCritical(KRFB_GBM) << "No client extensions defined!" << formatGLError(eglGetError());
        m_valid = false;
    }

    m_egl.extensions = clientExtensionsString.split(' ');
}

void GbmFrameBuffer::initEgl()
{
    if(!m_valid)
        return;

    // Use eglGetPlatformDisplayEXT() to get the display pointer
    // if the implementation supports it.
    if (!m_egl.extensions.contains(QByteArrayLiteral("EGL_EXT_platform_base")) ||
            !m_egl.extensions.contains(QByteArrayLiteral("EGL_MESA_platform_gbm"))) {
        qCCritical(KRFB_GBM) << "One of required EGL extensions is missing";
        m_valid = false;
        return;
    }

    m_egl.display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA, m_gbmDevice, nullptr);
    if (m_egl.display == EGL_NO_DISPLAY) {
        qCCritical(KRFB_GBM) << "Error during obtaining EGL display:" << formatGLError(eglGetError());
        m_valid = false;
        return;
    }

    EGLint major, minor;
    if (eglInitialize(m_egl.display, &major, &minor) == EGL_FALSE) {
        qCCritical(KRFB_GBM) << "Error during eglInitialize:" << formatGLError(eglGetError());
        m_valid = false;
        return;
    }

    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
        qCCritical(KRFB_GBM) << "bind OpenGL API failed";
        m_valid = false;
        return;
    }
    qCDebug(KRFB_GBM) << "Egl Initialize succeeded";
    qCDebug(KRFB_GBM) << QString("EGL version: %1.%2").arg(major).arg(minor);

    m_egl.context = eglCreateContext(m_egl.display, nullptr, EGL_NO_CONTEXT, nullptr);
    if(m_egl.context == EGL_NO_CONTEXT) {
        qCCritical(KRFB_GBM) << "Couldn't create EGL context:" << formatGLError(eglGetError());
        m_valid = false;
    }
}

void GbmFrameBuffer::initWaylandConnection()
{
    if(!m_valid)
        return;

    using namespace KWayland::Client;
    ConnectionThread *conn = ConnectionThread::fromApplication(this);
    if(!conn) { // trying to instantiate wayland not having a wayland platform?
        m_valid = false;
        return;
    }

    // what do we do if server dies?
    // since we use foreign connection, the whole application will be disconnected,
    // not only plugin, let's assume application will die anyway

    Registry *registry = new Registry(this);
    registry->create(conn);
    connect(registry, &Registry::remoteAccessManagerAnnounced, this,
        [this, registry] (qint32 name, qint32 version) {
            m_interface = registry->createRemoteAccessManager(name, version, this);
            connect(m_interface, &RemoteAccessManager::bufferReady, this, &GbmFrameBuffer::obtainBuffer);
        }
    );
    registry->setup();
}

void GbmFrameBuffer::obtainBuffer(KWayland::Client::RemoteBuffer *rbuf)
{
    using namespace KWayland::Client;
    connect(rbuf, &RemoteBuffer::paramsObtained, this,
        [this, rbuf] (qint32 fd, quint32 width, quint32 height, quint32 stride, quint32 format) {
            updateHandle(fd, width, height, stride, format);
            // deleteLater() is not working due to QTBUG-18434 (or similar)
            // TODO: try with Qt 5.8.0
            delete rbuf;
        }
    );
}

void GbmFrameBuffer::updateHandle(qint32 gbmHandle, quint32 width, quint32 height, quint32 stride, quint32 format)
{
    qCDebug(KRFB_GBM) << QString("Incoming GBM fd %1, %2x%3, stride %4, fourcc 0x%5")
        .arg(gbmHandle).arg(width).arg(height).arg(stride).arg(QString::number(format, 16));

    if(!gbm_device_is_format_supported(m_gbmDevice, format, GBM_BO_USE_SCANOUT)) {
        qCCritical(KRFB_GBM) << "GBM format is not supported by device!";
        return;
    }

    if(this->width() != (int) width || this->height() != (int) height) {
        qCCritical(KRFB_GBM) << QString("Size of GBM buffer (%3x%4) is not equal to expected (%1x%2)!")
            .arg(this->width()).arg(this->height()).arg(width).arg(height);
        return;
    }

    // import GBM buffer that was passed from KWin
    gbm_import_fd_data importInfo = {gbmHandle, width, height, stride, format};
    gbm_bo *imported = gbm_bo_import(m_gbmDevice, GBM_BO_IMPORT_FD, &importInfo, GBM_BO_USE_SCANOUT);
    if(!imported) {
        qCCritical(KRFB_GBM) << "Cannot import passed GBM fd:" << strerror(errno);
        return;
    }

    // bind context to render thread
    eglMakeCurrent(m_egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_egl.context);

    // create EGL image from imported BO
    EGLImageKHR image = eglCreateImageKHR(m_egl.display, NULL, EGL_NATIVE_PIXMAP_KHR, imported, NULL);
    if (image == EGL_NO_IMAGE_KHR) {
        qCCritical(KRFB_GBM) << "Error creating EGLImageKHR" << formatGLError(glGetError());
        return;
    }
    // create GL 2D texture for framebuffer
    GLuint texture;
    glGenTextures(1, &texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

    // bind framebuffer to copy pixels from
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        qCCritical(KRFB_GBM) << "glCheckFramebufferStatus failed:" << formatGLError(glGetError());
        glDeleteTextures(1, &texture);
        glDeleteFramebuffers(1, &framebuffer);
        eglDestroyImageKHR(m_egl.display, image);
        return;
    }

    glViewport(0, 0, width, height);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_img->bits());
    tiles.append(m_img->rect());

    // unbind all
    glDeleteTextures(1, &texture);
    glDeleteFramebuffers(1, &framebuffer);
    eglDestroyImageKHR(m_egl.display, image);

    // from this point buffer object can be safely freed
    close(gbmHandle);
}

GbmFrameBuffer::~GbmFrameBuffer()
{
    fb = nullptr;

    delete m_img;
    m_img = nullptr;

    if(m_egl.context != EGL_NO_CONTEXT) {
        eglDestroyContext(m_egl.display, m_egl.context);
    }

    if(m_gbmDevice) {
        gbm_device_destroy(m_gbmDevice);
    }

    if(m_drmFd) {
        close(m_drmFd);
    }
}

int GbmFrameBuffer::depth()
{
    return m_img->depth();
}

int GbmFrameBuffer::height()
{
    return m_img->height();
}

int GbmFrameBuffer::width()
{
    return m_img->width();
}

void GbmFrameBuffer::getServerFormat(rfbPixelFormat &format)
{
    format.bitsPerPixel = 32;
    format.depth = 32;
    format.trueColour = true;

    format.bigEndian = false;

    // GL images have different shift
    format.redShift = 0;
    format.greenShift = 8;
    format.blueShift = 16;
    format.redMax   = 0xff;
    format.greenMax = 0xff;
    format.blueMax  = 0xff;
}

void GbmFrameBuffer::updateFrameBuffer()
{
    // do nothing
}

int GbmFrameBuffer::paddedWidth()
{
    return m_img->bytesPerLine();
}

void GbmFrameBuffer::startMonitor()
{
    // not needed - we get events as a signals from Wayland interface
    // no other updates are possible
}

void GbmFrameBuffer::stopMonitor()
{

}
