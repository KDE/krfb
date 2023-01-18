/* This file is part of the KDE project
   Copyright (C) 2009 Collabora Ltd <info@collabora.co.uk>
    @author George Goldberg <george.goldberg@collabora.co.uk>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
    const QVector<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("krfb/framebuffer"), platformFilter, KPluginMetaData::AllowEmptyMetaData);
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
