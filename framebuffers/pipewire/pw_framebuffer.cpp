/* This file is part of the KDE project
   Copyright (C) 2018 Oleg Chernovskiy <kanedias@xaker.ru>
   Copyright (C) 2018 Jan Grulich <jgrulich@redhat.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
*/

// system
#include <sys/mman.h>
#include <cstring>

// Qt
#include <QX11Info>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QSocketNotifier>
#include <QDebug>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif

// pipewire
#include <pipewire/version.h>

#if PW_CHECK_VERSION(0, 2, 90)
#include <spa/utils/result.h>
#include <linux/dma-buf.h>
#include <sys/ioctl.h>
#endif

#include <spa/param/format-utils.h>
#include <spa/param/video/format-utils.h>
#include <spa/param/props.h>

#include <pipewire/pipewire.h>

#include <limits.h>

#include "pw_framebuffer.h"
#include "xdp_dbus_screencast_interface.h"
#include "xdp_dbus_remotedesktop_interface.h"

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

#if !PW_CHECK_VERSION(0, 2, 90)
/**
 * @brief The PwType class - helper class to contain pointers to raw C pipewire media mappings
 */
class PwType {
public:
    spa_type_media_type media_type;
    spa_type_media_subtype media_subtype;
    spa_type_format_video format_video;
    spa_type_video_format video_format;
};
#endif

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

#if PW_CHECK_VERSION(0, 2, 90)
    static void onCoreError(void *data, uint32_t id, int seq, int res, const char *message);
    static void onStreamParamChanged(void *data, uint32_t id, const struct spa_pod *format);
#else
    static void onStateChanged(void *data, pw_remote_state old, pw_remote_state state, const char *error);
    static void onStreamFormatChanged(void *data, const struct spa_pod *format);
#endif
    static void onStreamStateChanged(void *data, pw_stream_state old, pw_stream_state state, const char *error_message);
    static void onStreamProcess(void *data);

    void initDbus();
    void initPw();
#if !PW_CHECK_VERSION(0, 2, 90)
    void initializePwTypes();
#endif

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
#if PW_CHECK_VERSION(0, 2, 90)
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
#else
    pw_core *pwCore = nullptr;
    pw_loop *pwLoop = nullptr;
    pw_thread_loop *pwMainLoop = nullptr;
    pw_stream *pwStream = nullptr;
    pw_remote *pwRemote = nullptr;
    pw_type *pwCoreType = nullptr;
    PwType *pwType = nullptr;

    spa_hook remoteListener = {};
    spa_hook streamListener = {};

    // event handlers
    pw_remote_events pwRemoteEvents = {};
    pw_stream_events pwStreamEvents = {};
#endif

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
    } screenGeometry;

    // Allowed devices
    uint devices;

    // sanity indicator
    bool isValid = true;
};

PWFrameBuffer::Private::Private(PWFrameBuffer *q) : q(q)
{
#if PW_CHECK_VERSION(0, 2, 90)
    pwCoreEvents.version = PW_VERSION_CORE_EVENTS;
    pwCoreEvents.error = &onCoreError;

    pwStreamEvents.version = PW_VERSION_STREAM_EVENTS;
    pwStreamEvents.state_changed = &onStreamStateChanged;
    pwStreamEvents.param_changed = &onStreamParamChanged;
    pwStreamEvents.process = &onStreamProcess;
#else
    // initialize event handlers, remote end and stream-related
    pwRemoteEvents.version = PW_VERSION_REMOTE_EVENTS;
    pwRemoteEvents.state_changed = &onStateChanged;

    pwStreamEvents.version = PW_VERSION_STREAM_EVENTS;
    pwStreamEvents.state_changed = &onStreamStateChanged;
    pwStreamEvents.format_changed = &onStreamFormatChanged;
    pwStreamEvents.process = &onStreamProcess;
#endif
}

/**
 * @brief PWFrameBuffer::Private::initDbus - initialize D-Bus connectivity with XDG Desktop Portal.
 *        Based on XDG_CURRENT_DESKTOP environment variable it will give us implementation that we need,
 *        in case of KDE it is xdg-desktop-portal-kde binary.
 */
void PWFrameBuffer::Private::initDbus()
{
    qInfo() << "Initializing D-Bus connectivity with XDG Desktop Portal";
    dbusXdpScreenCastService.reset(new OrgFreedesktopPortalScreenCastInterface(QLatin1String("org.freedesktop.portal.Desktop"),
                                                                     QLatin1String("/org/freedesktop/portal/desktop"),
                                                                     QDBusConnection::sessionBus()));
    dbusXdpRemoteDesktopService.reset(new OrgFreedesktopPortalRemoteDesktopInterface(QLatin1String("org.freedesktop.portal.Desktop"),
                                                                     QLatin1String("/org/freedesktop/portal/desktop"),
                                                                     QDBusConnection::sessionBus()));
    auto version = dbusXdpScreenCastService->version();
    if (version < MIN_SUPPORTED_XDP_KDE_SC_VERSION) {
        qWarning() << "Unsupported XDG Portal screencast interface version:" << version;
        isValid = false;
        return;
    }

    // create session
    auto sessionParameters = QVariantMap {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        { QLatin1String("session_handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) },
        { QLatin1String("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
#else
        { QLatin1String("session_handle_token"), QStringLiteral("krfb%1").arg(qrand()) },
        { QLatin1String("handle_token"), QStringLiteral("krfb%1").arg(qrand()) }
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
                                          QLatin1String("org.freedesktop.portal.Request"),
                                          QLatin1String("Response"),
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
        qWarning() << "Failed to create session: " << code;
        isValid = false;
        return;
    }

    sessionPath = QDBusObjectPath(results.value(QLatin1String("session_handle")).toString());

    // select sources for the session
    auto selectionOptions = QVariantMap {
        // We have to specify it's an uint, otherwise xdg-desktop-portal will not forward it to backend implementation
        { QLatin1String("types"), QVariant::fromValue<uint>(7) }, // request all (KeyBoard, Pointer, TouchScreen)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        { QLatin1String("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
#else
        { QLatin1String("handle_token"), QStringLiteral("krfb%1").arg(qrand()) }
#endif
    };
    auto selectorReply = dbusXdpRemoteDesktopService->SelectDevices(sessionPath, selectionOptions);
    selectorReply.waitForFinished();
    if (!selectorReply.isValid()) {
        qWarning() << "Couldn't select devices for the remote-desktop session";
        isValid = false;
        return;
    }
    QDBusConnection::sessionBus().connect(QString(),
                                          selectorReply.value().path(),
                                          QLatin1String("org.freedesktop.portal.Request"),
                                          QLatin1String("Response"),
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
        qWarning() << "Failed to select devices: " << code;
        isValid = false;
        return;
    }

    // select sources for the session
    auto selectionOptions = QVariantMap {
        { QLatin1String("types"), QVariant::fromValue<uint>(1) }, // only MONITOR is supported
        { QLatin1String("multiple"), false },
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        { QLatin1String("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
#else
        { QLatin1String("handle_token"), QStringLiteral("krfb%1").arg(qrand()) }
#endif
    };
    auto selectorReply = dbusXdpScreenCastService->SelectSources(sessionPath, selectionOptions);
    selectorReply.waitForFinished();
    if (!selectorReply.isValid()) {
        qWarning() << "Couldn't select sources for the screen-casting session";
        isValid = false;
        return;
    }
    QDBusConnection::sessionBus().connect(QString(),
                                          selectorReply.value().path(),
                                          QLatin1String("org.freedesktop.portal.Request"),
                                          QLatin1String("Response"),
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
        qWarning() << "Failed to select sources: " << code;
        isValid = false;
        return;
    }

    // start session
    auto startParameters = QVariantMap {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        { QLatin1String("handle_token"), QStringLiteral("krfb%1").arg(QRandomGenerator::global()->generate()) }
#else
        { QLatin1String("handle_token"), QStringLiteral("krfb%1").arg(qrand()) }
#endif
    };
    auto startReply = dbusXdpRemoteDesktopService->Start(sessionPath, QString(), startParameters);
    startReply.waitForFinished();
    QDBusConnection::sessionBus().connect(QString(),
                                          startReply.value().path(),
                                          QLatin1String("org.freedesktop.portal.Request"),
                                          QLatin1String("Response"),
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
        qWarning() << "Failed to start screencast: " << code;
        isValid = false;
        return;
    }

    // there should be only one stream
    Streams streams = qdbus_cast<Streams>(results.value(QLatin1String("streams")));
    if (streams.isEmpty()) {
        // maybe we should check deeper with qdbus_cast but this suffices for now
        qWarning() << "Failed to get screencast streams";
        isValid = false;
        return;
    }

    auto streamReply = dbusXdpScreenCastService->OpenPipeWireRemote(sessionPath, QVariantMap());
    streamReply.waitForFinished();
    if (!streamReply.isValid()) {
        qWarning() << "Couldn't open pipewire remote for the screen-casting session";
        isValid = false;
        return;
    }

    pipewireFd = streamReply.value();
    if (!pipewireFd.isValid()) {
        qWarning() << "Couldn't get pipewire connection file descriptor";
        isValid = false;
        return;
    }

    QSize streamResolution = qdbus_cast<QSize>(streams.first().map.value(QLatin1String("size")));
    screenGeometry.width = streamResolution.width();
    screenGeometry.height = streamResolution.height();

    devices = results.value(QLatin1String("types")).toUInt();

    pwStreamNodeId = streams.first().nodeId;

    // Reallocate our buffer with actual needed size
    q->fb = static_cast<char*>(malloc(screenGeometry.width * screenGeometry.height * 4));

    if (!q->fb) {
        qWarning() << "Failed to allocate buffer";
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

#if PW_CHECK_VERSION(0, 2, 90)
    pwMainLoop = pw_thread_loop_new("pipewire-main-loop", nullptr);
    pwContext = pw_context_new(pw_thread_loop_get_loop(pwMainLoop), nullptr, 0);
    if (!pwContext) {
        qWarning() << "Failed to create PipeWire context";
        return;
    }

    pwCore = pw_context_connect(pwContext, nullptr, 0);
    if (!pwCore) {
        qWarning() << "Failed to connect PipeWire context";
        return;
    }

    pw_core_add_listener(pwCore, &coreListener, &pwCoreEvents, this);

    pwStream = createReceivingStream();
    if (!pwStream) {
        qWarning() << "Failed to create PipeWire stream";
        return;
    }
#else
    // initialize our source
    pwLoop = pw_loop_new(nullptr);
    pwMainLoop = pw_thread_loop_new(pwLoop, "pipewire-main-loop");
    // create PipeWire core object (required)
    pwCore = pw_core_new(pwLoop, nullptr);
    pwCoreType = pw_core_get_type(pwCore);

    initializePwTypes();

    // pw_remote should be initialized before type maps or connection error will happen
    pwRemote = pw_remote_new(pwCore, nullptr, 0);
    // init PipeWire remote, add listener to handle events
    pw_remote_add_listener(pwRemote, &remoteListener, &pwRemoteEvents, this);
    pw_remote_connect_fd(pwRemote, pipewireFd.fileDescriptor());
#endif

    if (pw_thread_loop_start(pwMainLoop) < 0) {
        qWarning() << "Failed to start main PipeWire loop";
        isValid = false;
    }
}

#if !PW_CHECK_VERSION(0, 2, 9)
/**
 * @brief PWFrameBuffer::Private::initializePwTypes - helper method to initialize and map all needed
 *        Pipewire types from core to type structure.
 */
void PWFrameBuffer::Private::initializePwTypes()
{
    // raw C-like PipeWire type map
    spa_type_map *map = pwCoreType->map;
    pwType = new PwType();

    spa_type_media_type_map(map, &pwType->media_type);
    spa_type_media_subtype_map(map, &pwType->media_subtype);
    spa_type_format_video_map(map, &pwType->format_video);
    spa_type_video_format_map(map, &pwType->video_format);
}
#endif


#if PW_CHECK_VERSION(0, 2, 90)
void PWFrameBuffer::Private::onCoreError(void *data, uint32_t id, int seq, int res, const char *message)
{
    Q_UNUSED(data);
    Q_UNUSED(id);
    Q_UNUSED(seq);
    Q_UNUSED(res);

    qInfo() << "core error: " << message;
}
#else
/**
 * @brief PWFrameBuffer::Private::onStateChanged - global state tracking for pipewire connection
 * @param data pointer that you have set in pw_remote_add_listener call's last argument
 * @param state new state that connection has changed to
 * @param error optional error message, is set to non-null if state is error
 */
void PWFrameBuffer::Private::onStateChanged(void *data, pw_remote_state /*old*/, pw_remote_state state, const char *error)
{
    qInfo() << "remote state: " << pw_remote_state_as_string(state);

    PWFrameBuffer::Private *d = static_cast<PWFrameBuffer::Private*>(data);

    switch (state) {
    case PW_REMOTE_STATE_ERROR:
        qWarning() << "remote error: " << error;
        break;
    case PW_REMOTE_STATE_CONNECTED:
        d->pwStream = d->createReceivingStream();
        break;
    default:
        qInfo() << "remote state: " << pw_remote_state_as_string(state);
        break;
    }
}
#endif

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

#if PW_CHECK_VERSION(0, 2, 90)
    switch (state) {
    case PW_STREAM_STATE_ERROR:
        qWarning() << "pipewire stream error: " << error_message;
        break;
    case PW_STREAM_STATE_PAUSED:
            pw_stream_set_active(d->pwStream, true);
        break;
    case PW_STREAM_STATE_STREAMING:
    case PW_STREAM_STATE_UNCONNECTED:
    case PW_STREAM_STATE_CONNECTING:
        break;
    }
#else
    switch (state) {
    case PW_STREAM_STATE_ERROR:
        qWarning() << "pipewire stream error: " << error_message;
        break;
    case PW_STREAM_STATE_CONFIGURE:
        pw_stream_set_active(d->pwStream, true);
        break;
    default:
        break;
    }
#endif
}

/**
 * @brief PWFrameBuffer::Private::onStreamFormatChanged - being executed after stream is set to active
 *        and after setup has been requested to connect to it. The actual video format is being negotiated here.
 * @param data pointer that you have set in pw_stream_add_listener call's last argument
 * @param format format that's being proposed
 */
#if PW_CHECK_VERSION(0, 2, 90)
void PWFrameBuffer::Private::onStreamParamChanged(void *data, uint32_t id, const struct spa_pod *format)
#else
void PWFrameBuffer::Private::onStreamFormatChanged(void *data, const struct spa_pod *format)
#endif
{
    qInfo() << "Stream format changed";
    auto *d = static_cast<PWFrameBuffer::Private *>(data);

    const int bpp = 4;

#if PW_CHECK_VERSION(0, 2, 90)
    if (!format || id != SPA_PARAM_Format) {
#else
    if (!format) {
        pw_stream_finish_format(d->pwStream, 0, nullptr, 0);
#endif
        return;
    }

    d->videoFormat = new spa_video_info_raw();
#if PW_CHECK_VERSION(0, 2, 90)
    spa_format_video_raw_parse(format, d->videoFormat);
#else
    spa_format_video_raw_parse(format, d->videoFormat, &d->pwType->format_video);
#endif
    auto width = d->videoFormat->size.width;
    auto height = d->videoFormat->size.height;
    auto stride = SPA_ROUND_UP_N(width * bpp, 4);
    auto size = height * stride;

    uint8_t buffer[1024];
    auto builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    // setup buffers and meta header for new format
    const struct spa_pod *params[2];

#if PW_CHECK_VERSION(0, 2, 90)
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
#else
    params[0] = reinterpret_cast<spa_pod *>(spa_pod_builder_object(&builder,
                d->pwCoreType->param.idBuffers, d->pwCoreType->param_buffers.Buffers,
                ":", d->pwCoreType->param_buffers.size, "i", size,
                ":", d->pwCoreType->param_buffers.stride, "i", stride,
                ":", d->pwCoreType->param_buffers.buffers, "iru", 8, SPA_POD_PROP_MIN_MAX(1, 32),
                ":", d->pwCoreType->param_buffers.align, "i", 16));
    params[1] = reinterpret_cast<spa_pod *>(spa_pod_builder_object(&builder,
                d->pwCoreType->param.idMeta, d->pwCoreType->param_meta.Meta,
                ":", d->pwCoreType->param_meta.type, "I", d->pwCoreType->meta.Header,
                ":", d->pwCoreType->param_meta.size, "i", sizeof(struct spa_meta_header)));
    pw_stream_finish_format(d->pwStream, 0, params, 2);
#endif
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

#if PW_CHECK_VERSION(0, 2, 90)
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
            qWarning() << "Failed to synchronize DMA buffer: " << strerror(errno);
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
    if (!src) {
        qDebug() << "discarding null buffer";
        return;
    }

#if PW_CHECK_VERSION(0, 2, 90)
    if (spaBuffer->datas->type != SPA_DATA_DmaBuf) {
        qDebug() << "discarding null buffer";
        return;
    }
#endif

    const quint32 maxSize = spaBuffer->datas[0].maxsize;

    std::function<void()> cleanup;
#if PW_CHECK_VERSION(0, 2, 90)
    if (spaBuffer->datas->type == SPA_DATA_DmaBuf) {
        const int fd = spaBuffer->datas[0].fd;
        auto map = mmap(
            nullptr, spaBuffer->datas[0].maxsize + spaBuffer->datas[0].mapoffset,
            PROT_READ, MAP_PRIVATE, fd, 0);
        if (map == MAP_FAILED) {
            qWarning() << "Failed to mmap the dmabuf: " << strerror(errno);
            return;
        }

        syncDmaBuf(fd, DMA_BUF_SYNC_START);
        src = SPA_MEMBER(map, spaBuffer->datas[0].mapoffset, uint8_t);

        cleanup = [map, spaBuffer, fd] {
            syncDmaBuf(fd, DMA_BUF_SYNC_END);
            munmap(map, spaBuffer->datas[0].maxsize + spaBuffer->datas[0].mapoffset);
        };
    } else if (spaBuffer->datas->type == SPA_DATA_MemFd) {
        uint8_t *map = static_cast<uint8_t*>(mmap(
            nullptr, spaBuffer->datas->maxsize + spaBuffer->datas->mapoffset,
            PROT_READ, MAP_PRIVATE, spaBuffer->datas->fd, 0));

        if (map == MAP_FAILED) {
            qWarning() << "Failed to mmap the memory: " << strerror(errno);
            return;
        }
        src = SPA_MEMBER(map, spaBuffer->datas[0].mapoffset, uint8_t);

        cleanup = [map, spaBuffer] {
            munmap(map, spaBuffer->datas->maxsize + spaBuffer->datas->mapoffset);
        };
    }
#endif

    const qint32 srcStride = spaBuffer->datas[0].chunk->stride;
    if (srcStride != q->paddedWidth()) {
        qWarning() << "Got buffer with stride different from screen stride" << srcStride << "!=" << q->paddedWidth();
        return;
    }

    q->tiles.append(QRect(0, 0, q->width(), q->height()));
    std::memcpy(q->fb, src, maxSize);
    cleanup();

#if PW_CHECK_VERSION(0, 2, 90)
    if (videoFormat->format != SPA_VIDEO_FORMAT_RGB) {
        const QImage::Format format = videoFormat->format == SPA_VIDEO_FORMAT_BGR  ? QImage::Format_BGR888
                                    : videoFormat->format == SPA_VIDEO_FORMAT_RGBx ? QImage::Format_RGBX8888
                                                                                   : QImage::Format_RGB32;

        QImage img((uchar*) q->fb, videoFormat->size.width, videoFormat->size.height, spaBuffer->datas->chunk->stride, format);
        img.convertTo(QImage::Format_RGB888);
    }
#endif
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

#if PW_CHECK_VERSION(0, 2, 90)
    auto stream = pw_stream_new_simple(pw_thread_loop_get_loop(pwMainLoop), "krfb-fb-consume-stream",
                                       pw_properties_new(PW_KEY_MEDIA_TYPE, "Video",
                                                         PW_KEY_MEDIA_CATEGORY, "Capture",
                                                         PW_KEY_MEDIA_ROLE, "Screen",
                                                         nullptr),
                                       &pwStreamEvents, this);

#else
    auto reuseProps = pw_properties_new("pipewire.client.reuse", "1", nullptr); // null marks end of varargs
    auto stream = pw_stream_new(pwRemote, "krfb-fb-consume-stream", reuseProps);
#endif
    uint8_t buffer[1024] = {};
    const spa_pod *params[1];
    auto builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

#if PW_CHECK_VERSION(0, 2, 90)
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
#else
    params[0] = reinterpret_cast<spa_pod *>(spa_pod_builder_object(&builder,
                pwCoreType->param.idEnumFormat, pwCoreType->spa_format,
                "I", pwType->media_type.video,
                "I", pwType->media_subtype.raw,
                ":", pwType->format_video.format, "I", pwType->video_format.RGBx,
                ":", pwType->format_video.size, "Rru", &pwMaxScreenBounds, SPA_POD_PROP_MIN_MAX(&pwMinScreenBounds, &pwMaxScreenBounds),
                ":", pwType->format_video.framerate, "F", &pwFramerateMin,
                ":", pwType->format_video.max_framerate, "Fru", &pwFramerateMax, 2, &pwFramerateMin, &pwFramerateMax));
    pw_stream_add_listener(stream, &streamListener, &pwStreamEvents, this);
#endif

    auto flags = static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_INACTIVE | PW_STREAM_FLAG_MAP_BUFFERS);
#if PW_CHECK_VERSION(0, 2, 90)
    if (pw_stream_connect(stream, PW_DIRECTION_INPUT, PW_ID_ANY, flags, params, 1) != 0) {
#else
    if (pw_stream_connect(stream, PW_DIRECTION_INPUT, nullptr, flags, params, 1) != 0) {
#endif
        qWarning() << "Could not connect receiving stream";
        isValid = false;
    }

    return stream;
}

PWFrameBuffer::Private::~Private()
{
    if (pwMainLoop) {
        pw_thread_loop_stop(pwMainLoop);
    }

#if !PW_CHECK_VERSION(0, 2, 9)
    if (pwType) {
        delete pwType;
    }
#endif

    if (pwStream) {
        pw_stream_destroy(pwStream);
    }

#if !PW_CHECK_VERSION(0, 2, 90)
    if (pwRemote) {
        pw_remote_destroy(pwRemote);
    }
#endif

#if PW_CHECK_VERSION(0, 2, 90)
    if (pwCore) {
        pw_core_disconnect(pwCore);
    }

    if (pwContext) {
        pw_context_destroy(pwContext);
    }
#else
    if (pwCore) {
        pw_core_destroy(pwCore);
    }
#endif

    if (pwMainLoop) {
        pw_thread_loop_destroy(pwMainLoop);
    }

#if !PW_CHECK_VERSION(0, 2, 90)
    if (pwLoop) {
        pw_loop_leave(pwLoop);
        pw_loop_destroy(pwLoop);
    }
#endif
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
