/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Collabora Ltd <info@collabora.co.uk>
   SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "framebuffermanager.h"

#include "framebufferplugin.h"
#include "krfbconfig.h"
#include "krfbdebug.h"

#include <QGuiApplication>
#include <QGlobalStatic>

#include <KPluginFactory>
#include <KPluginMetaData>


class FrameBufferManagerStatic
{
public:
    FrameBufferManager instance;
};

Q_GLOBAL_STATIC(FrameBufferManagerStatic, frameBufferManagerStatic)

FrameBufferManager::FrameBufferManager()
{
    const auto platformFilter = [] (const KPluginMetaData &pluginData) {
        return pluginData.value(QStringLiteral("X-KDE-OnlyShowOnQtPlatforms"), QStringList()).contains(QGuiApplication::platformName());
    };
    const QList<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("krfb/framebuffer"), platformFilter, KPluginMetaData::AllowEmptyMetaData);
    for (const KPluginMetaData &data : plugins) {
        const KPluginFactory::Result<FrameBufferPlugin> result = KPluginFactory::instantiatePlugin<FrameBufferPlugin>(data);
        if (result.plugin) {
            m_plugins.insert(data.pluginId(), result.plugin);
            qCDebug(KRFB) << "Loaded plugin with name " << data.pluginId();
        } else {
            qCDebug(KRFB) << "unable to load plugin for " << data.fileName() << result.errorString;
        }
    }
}

FrameBufferManager::~FrameBufferManager()
{
}

FrameBufferManager *FrameBufferManager::instance()
{
    return &frameBufferManagerStatic->instance;
}

QSharedPointer<FrameBuffer> FrameBufferManager::frameBuffer(WId id, const QVariantMap &args)
{
    // See if there is still an existing framebuffer to this WId.
    if (m_frameBuffers.contains(id)) {
        QWeakPointer<FrameBuffer> weakFrameBuffer = m_frameBuffers.value(id);

        if (weakFrameBuffer) {
            //qDebug() << "Found cached frame buffer.";
            return weakFrameBuffer.toStrongRef();
        } else {
            //qDebug() << "Found deleted cached frame buffer. Don't use.";
            m_frameBuffers.remove(id);
        }
    }

    if (auto preferredPlugin = m_plugins.value(KrfbConfig::preferredFrameBufferPlugin())) {
        if (auto frameBuffer = QSharedPointer<FrameBuffer>(preferredPlugin->frameBuffer(args))) {
            qCDebug(KRFB) << "Using FrameBuffer:" << KrfbConfig::preferredFrameBufferPlugin();
            m_frameBuffers.insert(id, frameBuffer.toWeakRef());
            return frameBuffer;
        }
    }

    // We don't already have that frame buffer.
    for (auto it = m_plugins.cbegin(); it != m_plugins.constEnd(); it++) {
        QSharedPointer<FrameBuffer> frameBuffer(it.value()->frameBuffer(args));
        if (frameBuffer) {
            qCDebug(KRFB) << "Using FrameBuffer:" << it.key();
            m_frameBuffers.insert(id, frameBuffer.toWeakRef());
            return frameBuffer;
        }
    }

    // No valid framebuffer plugin found.
    qCDebug(KRFB) << "No valid framebuffer found. returning null.";
    return QSharedPointer<FrameBuffer>();
}
