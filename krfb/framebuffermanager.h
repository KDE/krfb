/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Collabora Ltd <info@collabora.co.uk>
   SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRFB_FRAMEBUFFERMANAGER_H
#define KRFB_FRAMEBUFFERMANAGER_H

#include "framebuffer.h"

#include "krfbprivate_export.h"

#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include <QWeakPointer>

#include <QWidget>

class FrameBufferPlugin;
class KPluginFactory;

class KRFBPRIVATE_EXPORT FrameBufferManager : public QObject
{
    Q_OBJECT
    friend class FrameBufferManagerStatic;

public:
    static FrameBufferManager *instance();

    ~FrameBufferManager() override;

    QSharedPointer<FrameBuffer> frameBuffer(WId id, const QVariantMap &args);

private:
    Q_DISABLE_COPY(FrameBufferManager)

    FrameBufferManager();

    QMap<QString, FrameBufferPlugin *> m_plugins;
    QMap<WId, QWeakPointer<FrameBuffer>> m_frameBuffers;
};

#endif // Header guard
