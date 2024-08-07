/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2018 Oleg Chernovskiy <kanedias@xaker.ru>
   SPDX-FileCopyrightText: 2018-2020 Jan Grulich <jgrulich@redhat.com>

   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef KRFB_FRAMEBUFFER_XCB_XCB_FRAMEBUFFER_H
#define KRFB_FRAMEBUFFER_XCB_XCB_FRAMEBUFFER_H

#include "framebuffer.h"
#include <QWidget>
#include <QVariantMap>

/**
 * @brief The PWFrameBuffer class - framebuffer implementation based on XDG Desktop Portal ScreenCast interface.
 *        The design relies heavily on a presence of XDG D-Bus service and PipeWire daemon.
 *
 * @author Oleg Chernovskiy <kanedias@xaker.ru>
 */
class PWFrameBuffer: public FrameBuffer
{
    Q_OBJECT
public:
    using Stream = struct {
        uint nodeId;
        QVariantMap map;
    };
    using Streams = QList<Stream>;

    PWFrameBuffer(QObject *parent = nullptr);
    virtual ~PWFrameBuffer() override;

    void initDBus();
    void startVirtualMonitor(const QString &name, const QSize &resolution, qreal dpr);

    int  depth() override;
    int  height() override;
    int  width() override;
    int  paddedWidth() override;
    void getServerFormat(rfbPixelFormat &format) override;
    void startMonitor() override;
    void stopMonitor() override;
    QPoint cursorPosition() override;

    QVariant customProperty(const QString &property) const override;

    bool isValid() const;

private Q_SLOTS:
    void handleXdpSessionCreated(quint32 code, const QVariantMap &results);
    void handleXdpDevicesSelected(quint32 code, const QVariantMap &results);
    void handleXdpSourcesSelected(quint32 code, const QVariantMap &results);
    void handleXdpRemoteDesktopStarted(quint32 code, const QVariantMap &results);

private:
    class Private;
    const QScopedPointer<Private> d;
};

#endif
