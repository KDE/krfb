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

#include <QDebug>
#include <QGlobalStatic>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KPluginMetaData>

#include <QSharedPointer>

class FrameBufferManagerStatic
{
public:
    FrameBufferManager instance;
};

Q_GLOBAL_STATIC(FrameBufferManagerStatic, frameBufferManagerStatic)

FrameBufferManager::FrameBufferManager()
{
    //qDebug();

    loadPlugins();
}

FrameBufferManager::~FrameBufferManager()
{
    //qDebug();
}

FrameBufferManager *FrameBufferManager::instance()
{
    //qDebug();

    return &frameBufferManagerStatic->instance;
}

void FrameBufferManager::loadPlugins()
{
    //qDebug();

    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("krfb"), [](const KPluginMetaData & md) {
            return md.serviceTypes().contains(QStringLiteral("krfb/framebuffer"));
        });

    QVectorIterator<KPluginMetaData> i(plugins);
    i.toBack();
    QSet<QString> unique;
    while (i.hasPrevious()) {
        const KPluginMetaData &data = i.previous();
        // only load plugins once, even if found multiple times!
        if (unique.contains(data.name()))
            continue;
        KPluginFactory *factory = KPluginLoader(data.fileName()).factory();

        if (!factory) {
            qDebug() << "KPluginFactory could not load the plugin:" << data.fileName();
            continue;
        } else {
            qDebug() << "found plugin at " << data.fileName();
        }

        FrameBufferPlugin *plugin = factory->create<FrameBufferPlugin>(this);
        if (plugin) {
            m_plugins.insert(data.pluginId(), plugin);
            qDebug() << "Loaded plugin with name " << data.pluginId();
        } else {
            qDebug() << "unable to load pluign for " << data.fileName();
        }
        unique.insert (data.name());
    }
}

QSharedPointer<FrameBuffer> FrameBufferManager::frameBuffer(WId id)
{
    //qDebug();

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

    // We don't already have that frame buffer.
    QMap<QString, FrameBufferPlugin *>::const_iterator iter = m_plugins.constBegin();

    while (iter != m_plugins.constEnd()) {

        if (iter.key() == KrfbConfig::preferredFrameBufferPlugin()) {
            qDebug() << "Using FrameBuffer:" << KrfbConfig::preferredFrameBufferPlugin();

            QSharedPointer<FrameBuffer> frameBuffer(iter.value()->frameBuffer(id));

            if (frameBuffer) {
                m_frameBuffers.insert(id, frameBuffer.toWeakRef());

                return frameBuffer;
            }
        }

        ++iter;
    }

    // No valid framebuffer plugin found.
    qDebug() << "No valid framebuffer found. returning null.";
    return QSharedPointer<FrameBuffer>();
}
