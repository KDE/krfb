/* This file is part of the KDE project
   Copyright (C) 2018-2021 Jan Grulich <jgrulich@redhat.com>
   Copyright (C) 2018 Oleg Chernovskiy <kanedias@xaker.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
*/

// system
#include <sys/mman.h>
#include <cstring>

// Qt
#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QSocketNotifier>
#include <QDebug>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif

// pipewire
#ifdef HAVE_LINUX_DMABUF_H
#include <linux/dma-buf.h>
#endif
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
    struct {
        quint32 width;
        quint32 height;
    } screenGeometry = {};

    // Allowed devices
    uint devices = 0;

    // sanity indicator
    bool isValid = true;
};

PWFrameBuffer::Private::Private(PWFrameBuffer *q) : q(q)
{
    pwCoreEvents.version = PW_VERSION_CORE_EVENTS;
    pwCoreEvents.error = &onCoreError;

    pwStreamEvents.version = PW_VERSION_STREAM_EVENTS;
    pwStreamEvents.state_changed = &onStreamStateChanged;
    pwStreamEvents.param_changed = &onStreamParamChanged;
    pwStreamEvents.process = &onStreamProcess;
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        { QStringLiteral("session_handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) },
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
#else
        { QStringLiteral("session_handle_token"), QStringLiteral("krfb%1").arg(qrand()) },
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(qrand()) }
#endif
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
#else
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(qrand()) }
#endif
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
#else
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(qrand()) }
#endif
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
#else
        { QStringLiteral("handle_token"), QStringLiteral("krfb%1").arg(qrand()) }
#endif
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

    QSize streamResolution = qdbus_cast<QSize>(streams.first().map.value(QStringLiteral("size")));
    screenGeometry.width = streamResolution.width();
    screenGeometry.height = streamResolution.height();

    devices = results.value(QStringLiteral("types")).toUInt();

    pwStreamNodeId = streams.first().nodeId;

    // Reallocate our buffer with actual needed size
    q->fb = static_cast<char*>(malloc(screenGeometry.width * screenGeometry.height * 4));

    if (!q->fb) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Failed to allocate buffer";
        isValid = false;
        return;
    }

    Q_EMIT q->frameBufferChanged();

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
    qInfo() << "Stream state changed: " << pw_stream_state_as_string(state);

    auto *d = static_cast<PWFrameBuffer::Private *>(data);

    switch (state) {
    case PW_STREAM_STATE_ERROR:
        qCWarning(KRFB_FB_PIPEWIRE) << "pipewire stream error: " << error_message;
        break;
    case PW_STREAM_STATE_PAUSED:
            pw_stream_set_active(d->pwStream, true);
        break;
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

    const int bpp = 4;

    if (!format || id != SPA_PARAM_Format) {
        return;
    }

    d->videoFormat = new spa_video_info_raw();
    spa_format_video_raw_parse(format, d->videoFormat);
    auto width = d->videoFormat->size.width;
    auto height = d->videoFormat->size.height;
    auto stride = SPA_ROUND_UP_N(width * bpp, 4);
    auto size = height * stride;

    uint8_t buffer[1024];
    auto builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    // setup buffers and meta header for new format
    const struct spa_pod *params[2];

    params[0] = reinterpret_cast<spa_pod *>(spa_pod_builder_add_object(&builder,
                SPA_TYPE_OBJECT_ParamBuffers, SPA_PARAM_Buffers,
                SPA_PARAM_BUFFERS_size, SPA_POD_Int(size),
                SPA_PARAM_BUFFERS_stride, SPA_POD_Int(stride),
                SPA_PARAM_BUFFERS_buffers, SPA_POD_CHOICE_RANGE_Int(8, 1, 32),
                SPA_PARAM_BUFFERS_blocks, SPA_POD_Int(1),
                SPA_PARAM_BUFFERS_align, SPA_POD_Int(16)));
    params[1] = reinterpret_cast<spa_pod *>(spa_pod_builder_add_object(&builder,
                SPA_TYPE_OBJECT_ParamMeta, SPA_PARAM_Meta,
                SPA_PARAM_META_type, SPA_POD_Id(SPA_META_Header),
                SPA_PARAM_META_size, SPA_POD_Int(sizeof(struct spa_meta_header))));
    pw_stream_update_params(d->pwStream, params, 2);
}

/**
 * @brief PWFrameBuffer::Private::onNewBuffer - called when new buffer is available in pipewire stream
 * @param data pointer that you have set in pw_stream_add_listener call's last argument
 * @param id
 */
void PWFrameBuffer::Private::onStreamProcess(void *data)
{
    auto *d = static_cast<PWFrameBuffer::Private *>(data);

    pw_buffer *buf;
    if (!(buf = pw_stream_dequeue_buffer(d->pwStream))) {
        return;
    }

    d->handleFrame(buf);

    pw_stream_queue_buffer(d->pwStream, buf);
}

#if defined(HAVE_LINUX_DMABUF_H)
static void syncDmaBuf(int fd, uint64_t start_or_end)
{
    struct dma_buf_sync sync = { 0 };
    sync.flags = start_or_end | DMA_BUF_SYNC_READ;

    while(true) {
        int ret;
        ret = ioctl (fd, DMA_BUF_IOCTL_SYNC, &sync);
        if (ret == -1 && errno == EINTR) {
            continue;
        } else if (ret == -1) {
            qCWarning(KRFB_FB_PIPEWIRE) << "Failed to synchronize DMA buffer: " << strerror(errno);
            break;
        } else {
            break;
        }
    }
}
#endif

void PWFrameBuffer::Private::handleFrame(pw_buffer *pwBuffer)
{
    auto *spaBuffer = pwBuffer->buffer;
    void *src = spaBuffer->datas[0].data;

    if (!src && spaBuffer->datas->type != SPA_DATA_DmaBuf) {
        qCDebug(KRFB_FB_PIPEWIRE)  << "discarding null buffer";
        return;
    }

    const quint32 maxSize = spaBuffer->datas[0].maxsize;

    std::function<void()> cleanup;
#ifdef HAVE_LINUX_DMABUF_H
    if (spaBuffer->datas->type == SPA_DATA_DmaBuf) {
        const int fd = spaBuffer->datas[0].fd;
        auto map = mmap(
            nullptr, spaBuffer->datas[0].maxsize + spaBuffer->datas[0].mapoffset,
            PROT_READ, MAP_PRIVATE, fd, 0);
        if (map == MAP_FAILED) {
            qCWarning(KRFB_FB_PIPEWIRE) << "Failed to mmap the dmabuf: " << strerror(errno);
            return;
        }

        syncDmaBuf(fd, DMA_BUF_SYNC_START);
        src = SPA_MEMBER(map, spaBuffer->datas[0].mapoffset, uint8_t);

        cleanup = [map, spaBuffer, fd] {
            syncDmaBuf(fd, DMA_BUF_SYNC_END);
            munmap(map, spaBuffer->datas[0].maxsize + spaBuffer->datas[0].mapoffset);
        };
    } else
#endif
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
    }

    const qint32 srcStride = spaBuffer->datas[0].chunk->stride;
    if (srcStride != q->paddedWidth()) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Got buffer with stride different from screen stride" << srcStride << "!=" << q->paddedWidth();
        return;
    }

    q->tiles.append(QRect(0, 0, q->width(), q->height()));
    std::memcpy(q->fb, src, maxSize);
    cleanup();

    if (videoFormat->format == SPA_VIDEO_FORMAT_BGRA || videoFormat->format == SPA_VIDEO_FORMAT_BGRx) {
        for (uint y = 0; y < videoFormat->size.height; y++) {
            for (uint x = 0; x < videoFormat->size.width; x++) {
                uint offset = y * spaBuffer->datas->chunk->stride + x * 4;
                std::swap(q->fb[offset], q->fb[offset + 2]);
            }
        }
    } else if (videoFormat->format != SPA_VIDEO_FORMAT_RGB) {
        const QImage::Format format = videoFormat->format == SPA_VIDEO_FORMAT_BGR  ? QImage::Format_BGR888
                                    : videoFormat->format == SPA_VIDEO_FORMAT_RGBx ? QImage::Format_RGBX8888
                                                                                   : QImage::Format_RGB32;

        QImage img((uchar*) q->fb, videoFormat->size.width, videoFormat->size.height, spaBuffer->datas->chunk->stride, format);
        img.convertTo(QImage::Format_RGB888);
    }
}

/**
 * @brief PWFrameBuffer::Private::createReceivingStream - create a stream that will consume Pipewire buffers
 *        and copy the framebuffer to the existing image that we track. The state of the stream and configuration
 *        are later handled by the corresponding listener.
 */
pw_stream *PWFrameBuffer::Private::createReceivingStream()
{
    spa_rectangle pwMinScreenBounds = SPA_RECTANGLE(1, 1);
    spa_rectangle pwMaxScreenBounds = SPA_RECTANGLE(screenGeometry.width, screenGeometry.height);

    spa_fraction pwFramerateMin = SPA_FRACTION(0, 1);
    spa_fraction pwFramerateMax = SPA_FRACTION(60, 1);

    auto stream = pw_stream_new_simple(pw_thread_loop_get_loop(pwMainLoop), "krfb-fb-consume-stream",
                                       pw_properties_new(PW_KEY_MEDIA_TYPE, "Video",
                                                         PW_KEY_MEDIA_CATEGORY, "Capture",
                                                         PW_KEY_MEDIA_ROLE, "Screen",
                                                         nullptr),
                                       &pwStreamEvents, this);
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

    auto flags = static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_INACTIVE | PW_STREAM_FLAG_MAP_BUFFERS);
    if (pw_stream_connect(stream, PW_DIRECTION_INPUT, PW_ID_ANY, flags, params, 1) != 0) {
        qCWarning(KRFB_FB_PIPEWIRE) << "Could not connect receiving stream";
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

    // FIXME: for now use some initial size, later on we will reallocate this with the actual size we get from portal
    d->screenGeometry.width = 800;
    d->screenGeometry.height = 600;
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
    return static_cast<qint32>(d->screenGeometry.height);
}

int PWFrameBuffer::width()
{
    return static_cast<qint32>(d->screenGeometry.width);
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
