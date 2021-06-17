/* This file is part of the KDE project
   Copyright (C) 2018-2021 Jan Grulich <jgrulich@redhat.com>
   Copyright (C) 2018 Oleg Chernovskiy <kanedias@xaker.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
*/

#include "config-krfb.h"

// system
#include <sys/mman.h>
#include <cstring>

// Qt
#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QSocketNotifier>
#include <QDebug>
#include <QRandomGenerator>

// pipewire
#include <sys/ioctl.h>

#include <spa/param/format-utils.h>
#include <spa/param/video/format-utils.h>
#include <spa/param/props.h>
#include <spa/utils/result.h>

#include <pipewire/pipewire.h>

#include <climits>

#include "pw_framebuffer.h"
#include "xdp_dbus_screencast_interface.h"
#include "xdp_dbus_remotedesktop_interface.h"
#include "krfb_fb_pipewire_debug.h"

#if HAVE_DMA_BUF
#include <fcntl.h>
#include <unistd.h>

#include <gbm.h>
#include <epoxy/egl.h>
#include <epoxy/gl.h>
#endif /* HAVE_DMA_BUF */

static const int BYTES_PER_PIXEL = 4;
static const uint MIN_SUPPORTED_XDP_KDE_SC_VERSION = 1;

Q_DECLARE_METATYPE(PWFrameBuffer::Stream);
Q_DECLARE_METATYPE(PWFrameBuffer::Streams);

const QDBusArgument &operator >> (const QDBusArgument &arg, PWFrameBuffer::Stream &stream)
{
    arg.beginStructure();
    arg >> stream.nodeId;

    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant map;
        arg.beginMapEntry();
        arg >> key >> map;
        arg.endMapEntry();
        stream.map.insert(key, map);
    }
    arg.endMap();
    arg.endStructure();

    return arg;
}

#if HAVE_DMA_BUF
const char * formatGLError(GLenum err)
{
    switch(err) {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return (QLatin1String("0x") + QString::number(err, 16)).toLocal8Bit().constData();
    }
}
#endif /* HAVE_DMA_BUF */

/**
 * @brief The PWFrameBuffer::Private class - private counterpart of PWFramebuffer class. This is the entity where
 *        whole logic resides, for more info search for "d-pointer pattern" information.
 */
class PWFrameBuffer::Private {
public:
    Private(PWFrameBuffer *q);
    ~Private();

private:
    friend class PWFrameBuffer;

    static void onCoreError(void *data, uint32_t id, int seq, int res, const char *message);
    static void onStreamParamChanged(void *data, uint32_t id, const struct spa_pod *format);
    static void onStreamStateChanged(void *data, pw_stream_state old, pw_stream_state state, const char *error_message);
    static void onStreamProcess(void *data);

    void initDbus();
    void initPw();

    // dbus handling
    void handleSessionCreated(quint32 &code, QVariantMap &results);
    void handleDevicesSelected(quint32 &code, QVariantMap &results);
    void handleSourcesSelected(quint32 &code, QVariantMap &results);
    void handleRemoteDesktopStarted(quint32 &code, QVariantMap &results);

    // pw handling
    pw_stream *createReceivingStream();
    void handleFrame(pw_buffer *pwBuffer);

    // link to public interface
    PWFrameBuffer *q;

    // pipewire stuff
    struct pw_context *pwContext = nullptr;
    struct pw_core *pwCore = nullptr;
    struct pw_stream *pwStream = nullptr;
    struct pw_thread_loop *pwMainLoop = nullptr;

    // wayland-like listeners
    // ...of events that happen in pipewire server
    spa_hook coreListener = {};
    spa_hook streamListener = {};

    // event handlers
    pw_core_events pwCoreEvents = {};
    pw_stream_events pwStreamEvents = {};

    uint pwStreamNodeId = 0;

    // negotiated video format
    spa_video_info_raw *videoFormat = nullptr;

    // requests a session from XDG Desktop Portal
    // auto-generated and compiled from xdp_dbus_interface.xml file
    QScopedPointer<OrgFreedesktopPortalScreenCastInterface> dbusXdpScreenCastService;
    QScopedPointer<OrgFreedesktopPortalRemoteDesktopInterface> dbusXdpRemoteDesktopService;

    // XDP screencast session handle
    QDBusObjectPath sessionPath;
    // Pipewire file descriptor
    QDBusUnixFileDescriptor pipewireFd;

    // screen geometry holder
    QSize streamSize;
    QSize videoSize;

    // Allowed devices
    uint devices = 0;

    // sanity indicator
    bool isValid = true;

#if HAVE_DMA_BUF
    struct EGLStruct {
        QList<QByteArray> extensions;
        EGLDisplay display = EGL_NO_DISPLAY;
        EGLContext context = EGL_NO_CONTEXT;
    };

    bool m_eglInitialized = false;
    qint32 m_drmFd = 0; // for GBM buffer mmap
    gbm_device *m_gbmDevice = nullptr; // for passed GBM buffer retrieval

    EGLStruct m_egl;
#endif /* HAVE_DMA_BUF */
};

PWFrameBuffer::Private::Private(PWFrameBuffer *q) : q(q)
{
    pwCoreEvents.version = PW_VERSION_CORE_EVENTS;
    pwCoreEvents.error = &onCoreError;

    pwStreamEvents.version = PW_VERSION_STREAM_EVENTS;
    pwStreamEvents.state_changed = &onStreamStateChanged;
    pwStreamEvents.param_changed = &onStreamParamChanged;
    pwStreamEvents.process = &onStreamProcess;

#if HAVE_DMA_BUF
    m_drmFd = open("/dev/dri/renderD128", O_RDWR);

    if (m_drmFd < 0) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Failed to open drm render node: " << strerror(errno);
        return;
    }

    m_gbmDevice = gbm_create_device(m_drmFd);

    if (!m_gbmDevice) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Cannot create GBM device: " << strerror(errno);
        return;
    }

    // Get the list of client extensions
    const char* clientExtensionsCString = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    const QByteArray clientExtensionsString = QByteArray::fromRawData(clientExtensionsCString, qstrlen(clientExtensionsCString));
    if (clientExtensionsString.isEmpty()) {
        // If eglQueryString() returned NULL, the implementation doesn't support
        // EGL_EXT_client_extensions. Expect an EGL_BAD_DISPLAY error.
        qCWarning(KRFB_FB_PIPEWIRE) << "No client extensions defined! " << formatGLError(eglGetError());
        return;
    }

    m_egl.extensions = clientExtensionsString.split(' ');

    // Use eglGetPlatformDisplayEXT() to get the display pointer
    // if the implementation supports it.
    if (!m_egl.extensions.contains(QByteArrayLiteral("EGL_EXT_platform_base")) ||
            !m_egl.extensions.contains(QByteArrayLiteral("EGL_MESA_platform_gbm"))) {
        qCWarning(KRFB_FB_PIPEWIRE) << "One of required EGL extensions is missing";
        return;
    }

    m_egl.display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA, m_gbmDevice, nullptr);

    if (m_egl.display == EGL_NO_DISPLAY) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Error during obtaining EGL display: " << formatGLError(eglGetError());
        return;
    }

    EGLint major, minor;
    if (eglInitialize(m_egl.display, &major, &minor) == EGL_FALSE) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Error during eglInitialize: " << formatGLError(eglGetError());
        return;
    }

    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
        qCWarning(KRFB_FB_PIPEWIRE) << "bind OpenGL API failed";
        return;
    }

    m_egl.context = eglCreateContext(m_egl.display, nullptr, EGL_NO_CONTEXT, nullptr);

    if (m_egl.context == EGL_NO_CONTEXT) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Couldn't create EGL context: " << formatGLError(eglGetError());
        return;
    }

    qCDebug(KRFB_FB_PIPEWIRE) << "Egl initialization succeeded";
    qCDebug(KRFB_FB_PIPEWIRE) << QStringLiteral("EGL version: %1.%2").arg(major).arg(minor);

    m_eglInitialized = true;
#endif /* HAVE_DMA_BUF */
}

/**
 * @brief PWFrameBuffer::Private::initDbus - initialize D-Bus connectivity with XDG Desktop Portal.
 *        Based on XDG_CURRENT_DESKTOP environment variable it will give us implementation that we need,
 *        in case of KDE it is xdg-desktop-portal-kde binary.
 */
void PWFrameBuffer::Private::initDbus()
{
    qInfo() << "Initializing D-Bus connectivity with XDG Desktop Portal";
    dbusXdpScreenCastService.reset(new OrgFreedesktopPortalScreenCastInterface(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                                     QStringLiteral("/org/freedesktop/portal/desktop"),
                                                                     QDBusConnection::sessionBus()));
    dbusXdpRemoteDesktopService.reset(new OrgFreedesktopPortalRemoteDesktopInterface(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                                     QStringLiteral("/org/freedesktop/portal/desktop"),
                                                                     QDBusConnection::sessionBus()));
    auto version = dbusXdpScreenCastService->version();
    if (version < MIN_SUPPORTED_XDP_KDE_SC_VERSION) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Unsupported XDG Portal screencast interface version:" << version;
        isValid = false;
        return;
    }

    // create session
    auto sessionParameters = QVariantMap {
        { QStringLiteral("session_handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) },
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
    };
    auto sessionReply = dbusXdpRemoteDesktopService->CreateSession(sessionParameters);
    sessionReply.waitForFinished();
    if (!sessionReply.isValid()) {
        qWarning("Couldn't initialize XDP-KDE screencast session");
        isValid = false;
        return;
    }

    qInfo() << "DBus session created: " << sessionReply.value().path();
    QDBusConnection::sessionBus().connect(QString(),
                                          sessionReply.value().path(),
                                          QStringLiteral("org.freedesktop.portal.Request"),
                                          QStringLiteral("Response"),
                                          this->q,
                                          SLOT(handleXdpSessionCreated(uint, QVariantMap)));
}

void PWFrameBuffer::handleXdpSessionCreated(quint32 code, QVariantMap results)
{
    d->handleSessionCreated(code, results);
}

/**
 * @brief PWFrameBuffer::Private::handleSessionCreated - handle creation of ScreenCast session.
 *        XDG Portal answers with session path if it was able to successfully create the screencast.
 *
 * @param code return code for dbus call. Zero is success, non-zero means error
 * @param results map with results of call.
 */
void PWFrameBuffer::Private::handleSessionCreated(quint32 &code, QVariantMap &results)
{
    if (code != 0) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Failed to create session: " << code;
        isValid = false;
        return;
    }

    sessionPath = QDBusObjectPath(results.value(QStringLiteral("session_handle")).toString());

    // select sources for the session
    auto selectionOptions = QVariantMap {
        // We have to specify it's an uint, otherwise xdg-desktop-portal will not forward it to backend implementation
        { QStringLiteral("types"), QVariant::fromValue<uint>(7) }, // request all (KeyBoard, Pointer, TouchScreen)
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
    };
    auto selectorReply = dbusXdpRemoteDesktopService->SelectDevices(sessionPath, selectionOptions);
    selectorReply.waitForFinished();
    if (!selectorReply.isValid()) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Couldn't select devices for the remote-desktop session";
        isValid = false;
        return;
    }
    QDBusConnection::sessionBus().connect(QString(),
                                          selectorReply.value().path(),
                                          QStringLiteral("org.freedesktop.portal.Request"),
                                          QStringLiteral("Response"),
                                          this->q,
                                          SLOT(handleXdpDevicesSelected(uint, QVariantMap)));
}

void PWFrameBuffer::handleXdpDevicesSelected(quint32 code, QVariantMap results)
{
    d->handleDevicesSelected(code, results);
}

/**
 * @brief PWFrameBuffer::Private::handleDevicesCreated - handle selection of devices we want to use for remote desktop
 *
 * @param code return code for dbus call. Zero is success, non-zero means error
 * @param results map with results of call.
 */
void PWFrameBuffer::Private::handleDevicesSelected(quint32 &code, QVariantMap &results)
{
    Q_UNUSED(results)
    if (code != 0) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Failed to select devices: " << code;
        isValid = false;
        return;
    }

    // select sources for the session
    auto selectionOptions = QVariantMap {
        { QStringLiteral("types"), QVariant::fromValue<uint>(1) }, // only MONITOR is supported
        { QStringLiteral("multiple"), false },
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
    };
    auto selectorReply = dbusXdpScreenCastService->SelectSources(sessionPath, selectionOptions);
    selectorReply.waitForFinished();
    if (!selectorReply.isValid()) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Couldn't select sources for the screen-casting session";
        isValid = false;
        return;
    }
    QDBusConnection::sessionBus().connect(QString(),
                                          selectorReply.value().path(),
                                          QStringLiteral("org.freedesktop.portal.Request"),
                                          QStringLiteral("Response"),
                                          this->q,
                                          SLOT(handleXdpSourcesSelected(uint, QVariantMap)));
}

void PWFrameBuffer::handleXdpSourcesSelected(quint32 code, QVariantMap results)
{
    d->handleSourcesSelected(code, results);
}

/**
 * @brief PWFrameBuffer::Private::handleSourcesSelected - handle Screencast sources selection.
 *        XDG Portal shows a dialog at this point which allows you to select monitor from the list.
 *        This function is called after you make a selection.
 *
 * @param code return code for dbus call. Zero is success, non-zero means error
 * @param results map with results of call.
 */
void PWFrameBuffer::Private::handleSourcesSelected(quint32 &code, QVariantMap &)
{
    if (code != 0) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Failed to select sources: " << code;
        isValid = false;
        return;
    }

    // start session
    auto startParameters = QVariantMap {
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
    };
    auto startReply = dbusXdpRemoteDesktopService->Start(sessionPath, QString(), startParameters);
    startReply.waitForFinished();
    QDBusConnection::sessionBus().connect(QString(),
                                          startReply.value().path(),
                                          QStringLiteral("org.freedesktop.portal.Request"),
                                          QStringLiteral("Response"),
                                          this->q,
                                          SLOT(handleXdpRemoteDesktopStarted(uint, QVariantMap)));
}


void PWFrameBuffer::handleXdpRemoteDesktopStarted(quint32 code, QVariantMap results)
{
    d->handleRemoteDesktopStarted(code, results);
}

/**
 * @brief PWFrameBuffer::Private::handleScreencastStarted - handle Screencast start.
 *        At this point there shall be ready pipewire stream to consume.
 *
 * @param code return code for dbus call. Zero is success, non-zero means error
 * @param results map with results of call.
 */
void PWFrameBuffer::Private::handleRemoteDesktopStarted(quint32 &code, QVariantMap &results)
{
    if (code != 0) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Failed to start screencast: " << code;
        isValid = false;
        return;
    }

    // there should be only one stream
    Streams streams = qdbus_cast<Streams>(results.value(QStringLiteral("streams")));
    if (streams.isEmpty()) {
        // maybe we should check deeper with qdbus_cast but this suffices for now
        qCWarning(KRFB_FB_PIPEWIRE) << "Failed to get screencast streams";
        isValid = false;
        return;
    }

    auto streamReply = dbusXdpScreenCastService->OpenPipeWireRemote(sessionPath, QVariantMap());
    streamReply.waitForFinished();
    if (!streamReply.isValid()) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Couldn't open pipewire remote for the screen-casting session";
        isValid = false;
        return;
    }

    pipewireFd = streamReply.value();
    if (!pipewireFd.isValid()) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Couldn't get pipewire connection file descriptor";
        isValid = false;
        return;
    }

    devices = results.value(QStringLiteral("types")).toUInt();

    pwStreamNodeId = streams.first().nodeId;

    initPw();
}

/**
 * @brief PWFrameBuffer::Private::initPw - initialize Pipewire socket connectivity.
 *        pipewireFd should be pointing to existing file descriptor that was passed by D-Bus at this point.
 */
void PWFrameBuffer::Private::initPw() {
    qInfo() << "Initializing Pipewire connectivity";

    // init pipewire (required)
    pw_init(nullptr, nullptr); // args are not used anyways

    pwMainLoop = pw_thread_loop_new("pipewire-main-loop", nullptr);
    pw_thread_loop_lock(pwMainLoop);

    pwContext = pw_context_new(pw_thread_loop_get_loop(pwMainLoop), nullptr, 0);
    if (!pwContext) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Failed to create PipeWire context";
        return;
    }

    pwCore = pw_context_connect(pwContext, nullptr, 0);
    if (!pwCore) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Failed to connect PipeWire context";
        return;
    }

    pw_core_add_listener(pwCore, &coreListener, &pwCoreEvents, this);

    pwStream = createReceivingStream();
    if (!pwStream) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Failed to create PipeWire stream";
        return;
    }

    if (pw_thread_loop_start(pwMainLoop) < 0) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Failed to start main PipeWire loop";
        isValid = false;
    }

    pw_thread_loop_unlock(pwMainLoop);
}

void PWFrameBuffer::Private::onCoreError(void *data, uint32_t id, int seq, int res, const char *message)
{
    Q_UNUSED(data);
    Q_UNUSED(id);
    Q_UNUSED(seq);
    Q_UNUSED(res);

    qInfo() << "core error: " << message;
}

/**
 * @brief PWFrameBuffer::Private::onStreamStateChanged - called whenever stream state changes on pipewire server
 * @param data pointer that you have set in pw_stream_add_listener call's last argument
 * @param state new state that stream has changed to
 * @param error_message optional error message, is set to non-null if state is error
 */
void PWFrameBuffer::Private::onStreamStateChanged(void *data, pw_stream_state /*old*/, pw_stream_state state, const char *error_message)
{
    Q_UNUSED(data);

    qInfo() << "Stream state changed: " << pw_stream_state_as_string(state);

    switch (state) {
    case PW_STREAM_STATE_ERROR:
        qCWarning(KRFB_FB_PIPEWIRE) << "pipewire stream error: " << error_message;
        break;
    case PW_STREAM_STATE_PAUSED:
    case PW_STREAM_STATE_STREAMING:
    case PW_STREAM_STATE_UNCONNECTED:
    case PW_STREAM_STATE_CONNECTING:
        break;
    }
}

/**
 * @brief PWFrameBuffer::Private::onStreamFormatChanged - being executed after stream is set to active
 *        and after setup has been requested to connect to it. The actual video format is being negotiated here.
 * @param data pointer that you have set in pw_stream_add_listener call's last argument
 * @param format format that's being proposed
 */
void PWFrameBuffer::Private::onStreamParamChanged(void *data, uint32_t id, const struct spa_pod *format)
{
    qInfo() << "Stream format changed";
    auto *d = static_cast<PWFrameBuffer::Private *>(data);

    if (!format || id != SPA_PARAM_Format) {
        return;
    }

    d->videoFormat = new spa_video_info_raw();
    spa_format_video_raw_parse(format, d->videoFormat);
    auto width = d->videoFormat->size.width;
    auto height = d->videoFormat->size.height;
    auto stride = SPA_ROUND_UP_N(width * BYTES_PER_PIXEL, 4);
    auto size = height * stride;
    d->streamSize = QSize(width, height);

    uint8_t buffer[1024];
    auto builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    // setup buffers and meta header for new format
    const struct spa_pod *params[3];

#if HAVE_DMA_BUF
    const auto bufferTypes = d->m_eglInitialized ? (1 << SPA_DATA_DmaBuf) | (1 << SPA_DATA_MemFd) | (1 << SPA_DATA_MemPtr) :
                                                   (1 << SPA_DATA_MemFd) | (1 << SPA_DATA_MemPtr);
#else
    const auto bufferTypes = (1 << SPA_DATA_MemFd) | (1 << SPA_DATA_MemPtr);
#endif /* HAVE_DMA_BUF */

    params[0] = reinterpret_cast<spa_pod *>(spa_pod_builder_add_object(&builder,
                SPA_TYPE_OBJECT_ParamBuffers, SPA_PARAM_Buffers,
                SPA_PARAM_BUFFERS_size, SPA_POD_Int(size),
                SPA_PARAM_BUFFERS_stride, SPA_POD_Int(stride),
                SPA_PARAM_BUFFERS_buffers, SPA_POD_CHOICE_RANGE_Int(8, 1, 32),
                SPA_PARAM_BUFFERS_blocks, SPA_POD_Int(1),
                SPA_PARAM_BUFFERS_align, SPA_POD_Int(16),
                SPA_PARAM_BUFFERS_dataType, SPA_POD_CHOICE_FLAGS_Int(bufferTypes)));
    params[1] = reinterpret_cast<spa_pod *>(spa_pod_builder_add_object(&builder,
                SPA_TYPE_OBJECT_ParamMeta, SPA_PARAM_Meta,
                SPA_PARAM_META_type, SPA_POD_Id(SPA_META_Header),
                SPA_PARAM_META_size, SPA_POD_Int(sizeof(struct spa_meta_header))));
    params[2] = reinterpret_cast<spa_pod*>(spa_pod_builder_add_object(&builder,
                SPA_TYPE_OBJECT_ParamMeta, SPA_PARAM_Meta, SPA_PARAM_META_type,
                SPA_POD_Id(SPA_META_VideoCrop), SPA_PARAM_META_size,
                SPA_POD_Int(sizeof(struct spa_meta_region))));
    pw_stream_update_params(d->pwStream, params, 3);
}

/**
 * @brief PWFrameBuffer::Private::onNewBuffer - called when new buffer is available in pipewire stream
 * @param data pointer that you have set in pw_stream_add_listener call's last argument
 * @param id
 */
void PWFrameBuffer::Private::onStreamProcess(void *data)
{
    auto *d = static_cast<PWFrameBuffer::Private *>(data);

    pw_buffer* next_buffer;
    pw_buffer* buffer = nullptr;

    next_buffer = pw_stream_dequeue_buffer(d->pwStream);
    while (next_buffer) {
        buffer = next_buffer;
        next_buffer = pw_stream_dequeue_buffer(d->pwStream);

        if (next_buffer) {
            pw_stream_queue_buffer(d->pwStream, buffer);
        }
    }

    if (!buffer) {
        return;
    }

    d->handleFrame(buffer);

    pw_stream_queue_buffer(d->pwStream, buffer);
}

void PWFrameBuffer::Private::handleFrame(pw_buffer *pwBuffer)
{
    auto *spaBuffer = pwBuffer->buffer;
    uint8_t *src = nullptr;

    if (spaBuffer->datas[0].chunk->size == 0) {
        qCDebug(KRFB_FB_PIPEWIRE)  << "discarding null buffer";
        return;
    }

    std::function<void()> cleanup;
    const qint64 srcStride = spaBuffer->datas[0].chunk->stride;
    if (spaBuffer->datas->type == SPA_DATA_MemFd) {
        uint8_t *map = static_cast<uint8_t*>(mmap(
            nullptr, spaBuffer->datas->maxsize + spaBuffer->datas->mapoffset,
            PROT_READ, MAP_PRIVATE, spaBuffer->datas->fd, 0));

        if (map == MAP_FAILED) {
            qCWarning(KRFB_FB_PIPEWIRE) << "Failed to mmap the memory: " << strerror(errno);
            return;
        }
        src = SPA_MEMBER(map, spaBuffer->datas[0].mapoffset, uint8_t);

        cleanup = [map, spaBuffer] {
            munmap(map, spaBuffer->datas->maxsize + spaBuffer->datas->mapoffset);
        };
    } else if (spaBuffer->datas[0].type == SPA_DATA_MemPtr) {
        src = static_cast<uint8_t*>(spaBuffer->datas[0].data);
    }
#if HAVE_DMA_BUF
    else if (spaBuffer->datas->type == SPA_DATA_DmaBuf) {
        if (!m_eglInitialized) {
            // Shouldn't reach this
            qCWarning(KRFB_FB_PIPEWIRE) << "Failed to process DMA buffer.";
            return;
        }

        gbm_import_fd_data importInfo = {static_cast<int>(spaBuffer->datas->fd), static_cast<uint32_t>(streamSize.width()),
                                         static_cast<uint32_t>(streamSize.height()), static_cast<uint32_t>(spaBuffer->datas[0].chunk->stride), GBM_BO_FORMAT_ARGB8888};
        gbm_bo *imported = gbm_bo_import(m_gbmDevice, GBM_BO_IMPORT_FD, &importInfo, GBM_BO_USE_SCANOUT);
        if (!imported) {
            qCWarning(KRFB_FB_PIPEWIRE) << "Failed to process buffer: Cannot import passed GBM fd - " << strerror(errno);
            return;
        }

        // bind context to render thread
        eglMakeCurrent(m_egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_egl.context);

        // create EGL image from imported BO
        EGLImageKHR image = eglCreateImageKHR(m_egl.display, nullptr, EGL_NATIVE_PIXMAP_KHR, imported, nullptr);

        if (image == EGL_NO_IMAGE_KHR) {
            qCWarning(KRFB_FB_PIPEWIRE) << "Failed to record frame: Error creating EGLImageKHR - " << formatGLError(glGetError());
            gbm_bo_destroy(imported);
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

        src = static_cast<uint8_t*>(malloc(srcStride * streamSize.height()));

        GLenum glFormat = GL_BGRA;
        switch (videoFormat->format) {
            case SPA_VIDEO_FORMAT_RGBx:
                glFormat = GL_RGBA;
                break;
            case SPA_VIDEO_FORMAT_RGBA:
                glFormat = GL_RGBA;
                break;
            case SPA_VIDEO_FORMAT_BGRx:
                glFormat = GL_BGRA;
                break;
            case SPA_VIDEO_FORMAT_RGB:
                glFormat = GL_RGB;
                break;
            case SPA_VIDEO_FORMAT_BGR:
                glFormat = GL_BGR;
                break;
            default:
                glFormat = GL_BGRA;
                break;
        }
        glGetTexImage(GL_TEXTURE_2D, 0, glFormat, GL_UNSIGNED_BYTE, src);

        if (!src) {
            qCWarning(KRFB_FB_PIPEWIRE) << "Failed to get image from DMA buffer.";
            gbm_bo_destroy(imported);
            return;
        }

        cleanup = [src] {
            free(src);
        };

        glDeleteTextures(1, &texture);
        eglDestroyImageKHR(m_egl.display, image);

        gbm_bo_destroy(imported);
    }
#endif /* HAVE_DMA_BUF */

    struct spa_meta_region* videoMetadata =
    static_cast<struct spa_meta_region*>(spa_buffer_find_meta_data(
        spaBuffer, SPA_META_VideoCrop, sizeof(*videoMetadata)));

    if (videoMetadata && (videoMetadata->region.size.width > static_cast<uint32_t>(streamSize.width()) ||
                          videoMetadata->region.size.height > static_cast<uint32_t>(streamSize.height()))) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Stream metadata sizes are wrong!";
        return;
    }

    // Use video metadata when video size from metadata is set and smaller than
    // video stream size, so we need to adjust it.
    bool videoFullWidth = true;
    bool videoFullHeight = true;
    if (videoMetadata && videoMetadata->region.size.width != 0 &&
        videoMetadata->region.size.height != 0) {
        if (videoMetadata->region.size.width < static_cast<uint32_t>(streamSize.width())) {
            videoFullWidth = false;
        } else if (videoMetadata->region.size.height < static_cast<uint32_t>(streamSize.height())) {
            videoFullHeight = false;
        }
    }

    QSize prevVideoSize = videoSize;
    if (!videoFullHeight || !videoFullWidth) {
        videoSize = QSize(videoMetadata->region.size.width, videoMetadata->region.size.height);
    } else {
        videoSize = streamSize;
    }

    if (!q->fb || videoSize != prevVideoSize) {
        if (q->fb) {
            free(q->fb);
        }
        q->fb = static_cast<char*>(malloc(videoSize.width() * videoSize.height() * BYTES_PER_PIXEL));

        if (!q->fb) {
            qCWarning(KRFB_FB_PIPEWIRE) << "Failed to allocate buffer";
            isValid = false;
            return;
        }

        Q_EMIT q->frameBufferChanged();
    }

    const qint32 dstStride = videoSize.width() * BYTES_PER_PIXEL;
    Q_ASSERT(dstStride <= srcStride);

    if (!videoFullHeight && (videoMetadata->region.position.y + videoSize.height() <=  streamSize.height())) {
        src += srcStride * videoMetadata->region.position.y;
    }

    const int xOffset = !videoFullWidth && (videoMetadata->region.position.x + videoSize.width() <= streamSize.width())
                            ? videoMetadata->region.position.x * BYTES_PER_PIXEL : 0;

    char *dst = q->fb;
    for (int i = 0; i < videoSize.height(); ++i) {
        // Adjust source content based on crop video position if needed
        src += xOffset;
        std::memcpy(dst, src, dstStride);

        if (videoFormat->format == SPA_VIDEO_FORMAT_BGRA || videoFormat->format == SPA_VIDEO_FORMAT_BGRx) {
            for (int j = 0; j < dstStride; j += 4) {
                std::swap(dst[j], dst[j + 2]);
            }
        }

        src += srcStride - xOffset;
        dst += dstStride;
    }

    if (spaBuffer->datas->type == SPA_DATA_MemFd ||
        spaBuffer->datas->type == SPA_DATA_DmaBuf) {
        cleanup();
    }

    if (videoFormat->format != SPA_VIDEO_FORMAT_RGB) {
        const QImage::Format format = videoFormat->format == SPA_VIDEO_FORMAT_BGR  ? QImage::Format_BGR888
                                    : videoFormat->format == SPA_VIDEO_FORMAT_RGBx ? QImage::Format_RGBX8888
                                                                                   : QImage::Format_RGB32;

        QImage img((uchar*) q->fb, videoSize.width(), videoSize.height(), dstStride, format);
        img.convertTo(QImage::Format_RGB888);
    }

    q->tiles.append(QRect(0, 0, videoSize.width(), videoSize.height()));
}

/**
 * @brief PWFrameBuffer::Private::createReceivingStream - create a stream that will consume Pipewire buffers
 *        and copy the framebuffer to the existing image that we track. The state of the stream and configuration
 *        are later handled by the corresponding listener.
 */
pw_stream *PWFrameBuffer::Private::createReceivingStream()
{
    spa_rectangle pwMinScreenBounds = SPA_RECTANGLE(1, 1);
    spa_rectangle pwMaxScreenBounds = SPA_RECTANGLE(UINT32_MAX, UINT32_MAX);

    spa_fraction pwFramerateMin = SPA_FRACTION(0, 1);
    spa_fraction pwFramerateMax = SPA_FRACTION(60, 1);

    pw_properties* reuseProps = pw_properties_new_string("pipewire.client.reuse=1");

    auto stream = pw_stream_new(pwCore, "krfb-fb-consume-stream", reuseProps);

    uint8_t buffer[1024] = {};
    const spa_pod *params[1];
    auto builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    params[0] = reinterpret_cast<spa_pod *>(spa_pod_builder_add_object(&builder,
                SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat,
                SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video),
                SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
                SPA_FORMAT_VIDEO_format, SPA_POD_CHOICE_ENUM_Id(6,
                                                    SPA_VIDEO_FORMAT_RGBx, SPA_VIDEO_FORMAT_RGBA,
                                                    SPA_VIDEO_FORMAT_BGRx, SPA_VIDEO_FORMAT_BGRA,
                                                    SPA_VIDEO_FORMAT_RGB, SPA_VIDEO_FORMAT_BGR),
                SPA_FORMAT_VIDEO_size, SPA_POD_CHOICE_RANGE_Rectangle(&pwMaxScreenBounds, &pwMinScreenBounds, &pwMaxScreenBounds),
                SPA_FORMAT_VIDEO_framerate, SPA_POD_Fraction(&pwFramerateMin),
                SPA_FORMAT_VIDEO_maxFramerate, SPA_POD_CHOICE_RANGE_Fraction(&pwFramerateMax, &pwFramerateMin, &pwFramerateMax)));

    pw_stream_add_listener(stream, &streamListener, &pwStreamEvents, this);

    if (pw_stream_connect(stream, PW_DIRECTION_INPUT, pwStreamNodeId, PW_STREAM_FLAG_AUTOCONNECT, params, 1) != 0) {
        isValid = false;
    }

    return stream;
}

PWFrameBuffer::Private::~Private()
{
    if (pwMainLoop) {
        pw_thread_loop_stop(pwMainLoop);
    }

    if (pwStream) {
        pw_stream_destroy(pwStream);
    }

    if (pwCore) {
        pw_core_disconnect(pwCore);
    }

    if (pwContext) {
        pw_context_destroy(pwContext);
    }

    if (pwMainLoop) {
        pw_thread_loop_destroy(pwMainLoop);
    }
}

PWFrameBuffer::PWFrameBuffer(WId winid, QObject *parent)
    : FrameBuffer (winid, parent),
      d(new Private(this))
{
    // D-Bus is most important in init chain, no toys for us if something is wrong with XDP
    // PipeWire connectivity is initialized after D-Bus session is started
    d->initDbus();

    fb = nullptr;
}

PWFrameBuffer::~PWFrameBuffer()
{
    free(fb);
    fb = nullptr;
}

int PWFrameBuffer::depth()
{
    return 32;
}

int PWFrameBuffer::height()
{
    return d->videoSize.height();
}

int PWFrameBuffer::width()
{
    return d->videoSize.width();
}

int PWFrameBuffer::paddedWidth()
{
    return width() * 4;
}

void PWFrameBuffer::getServerFormat(rfbPixelFormat &format)
{
    format.bitsPerPixel = 32;
    format.depth = 32;
    format.trueColour = true;
    format.bigEndian = false;
}

void PWFrameBuffer::startMonitor()
{

}

void PWFrameBuffer::stopMonitor()
{

}

QVariant PWFrameBuffer::customProperty(const QString &property) const
{
    if (property == QLatin1String("stream_node_id")) {
        return QVariant::fromValue<uint>(d->pwStreamNodeId);
    } if (property == QLatin1String("session_handle")) {
        return QVariant::fromValue<QDBusObjectPath>(d->sessionPath);
    }

    return FrameBuffer::customProperty(property);
}

bool PWFrameBuffer::isValid() const
{
    return d->isValid;
}
