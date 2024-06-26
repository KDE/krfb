/*
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "screencasting.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 1)
#include "qwayland-zkde-screencast-unstable-v1.h"
#else
#include "qwayland-screencast.h"
#endif
#include <KWayland/Client/registry.h>
#include <QDebug>
#include <QRect>
#include <QPointer>

using namespace KWayland::Client;

class ScreencastingStreamPrivate : public QtWayland::zkde_screencast_stream_unstable_v1
{
public:
    ScreencastingStreamPrivate(ScreencastingStream *q)
        : q(q)
    {
    }
    ~ScreencastingStreamPrivate() override
    {
        close();
        q->deleteLater();
    }

    void zkde_screencast_stream_unstable_v1_created(uint32_t node) override
    {
        m_nodeId = node;
        Q_EMIT q->created(node);
    }

    void zkde_screencast_stream_unstable_v1_closed() override
    {
        Q_EMIT q->closed();
    }

    void zkde_screencast_stream_unstable_v1_failed(const QString &error) override
    {
        Q_EMIT q->failed(error);
    }

    uint m_nodeId = 0;
    QPointer<ScreencastingStream> q;
};

ScreencastingStream::ScreencastingStream(QObject *parent)
    : QObject(parent)
    , d(new ScreencastingStreamPrivate(this))
{
}

ScreencastingStream::~ScreencastingStream() = default;

quint32 ScreencastingStream::nodeId() const
{
    return d->m_nodeId;
}

class ScreencastingPrivate : public QtWayland::zkde_screencast_unstable_v1
{
public:
    ScreencastingPrivate(Registry *registry, int id, int version, Screencasting *q)
        : QtWayland::zkde_screencast_unstable_v1(*registry, id, version)
        , q(q)
    {
    }

    ScreencastingPrivate(::zkde_screencast_unstable_v1 *screencasting, Screencasting *q)
        : QtWayland::zkde_screencast_unstable_v1(screencasting)
        , q(q)
    {
    }

    ~ScreencastingPrivate() override
    {
        destroy();
    }

    Screencasting *const q;
};

Screencasting::Screencasting(QObject *parent)
    : QObject(parent)
{
}

Screencasting::Screencasting(Registry *registry, int id, int version, QObject *parent)
    : QObject(parent)
    , d(new ScreencastingPrivate(registry, id, version, this))
{
}

Screencasting::~Screencasting() = default;

ScreencastingStream * Screencasting::createVirtualMonitorStream(const QString& name, const QSize& resolution, qreal dpr, Screencasting::CursorMode mode)
{
    auto stream = new ScreencastingStream(this);
    stream->d->init(d->stream_virtual_output(name, resolution.width(), resolution.height(), wl_fixed_from_double(dpr), mode));
    return stream;
}

void Screencasting::setup(::zkde_screencast_unstable_v1 *screencasting)
{
    d.reset(new ScreencastingPrivate(screencasting, this));
}

void Screencasting::destroy()
{
    d.reset(nullptr);
}
