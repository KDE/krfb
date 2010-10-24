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

#include <KDebug>
#include <KGlobal>
#include <KServiceTypeTrader>

#include <QtCore/QSharedPointer>

class FrameBufferManagerStatic
{
public:
    FrameBufferManager instance;
};

K_GLOBAL_STATIC(FrameBufferManagerStatic, frameBufferManagerStatic)

FrameBufferManager::FrameBufferManager()
{
    kDebug();

    loadPlugins();
}

FrameBufferManager::~FrameBufferManager()
{
    kDebug();
}

FrameBufferManager *FrameBufferManager::instance()
{
    kDebug();

    return &frameBufferManagerStatic->instance;
}

void FrameBufferManager::loadPlugins()
{
    kDebug();

    // Load the all the plugin factories here, for use later.
    KService::List offers = KServiceTypeTrader::self()->query("krfb/framebuffer");

    KService::List::const_iterator iter;

    for (iter = offers.constBegin(); iter < offers.constEnd(); ++iter) {
        QString error;
        KService::Ptr service = *iter;

        KPluginFactory *factory = KPluginLoader(service->library()).factory();

        if (!factory) {
            kWarning() << "KPluginFactory could not load the plugin:" << service->library();
            continue;
        }

        FrameBufferPlugin *plugin = factory->create<FrameBufferPlugin>(this);

        if (plugin) {
            kDebug() << "Loaded plugin:" << service->name();
            m_plugins.insert(service->library(), plugin);
        } else {
            kDebug() << error;
        }
    }
}

QSharedPointer<FrameBuffer> FrameBufferManager::frameBuffer(WId id)
{
    kDebug();

    // See if there is still an existing framebuffer to this WId.
    if (m_frameBuffers.contains(id)) {
        QWeakPointer<FrameBuffer> weakFrameBuffer = m_frameBuffers.value(id);

        if (weakFrameBuffer) {
            kDebug() << "Found cached frame buffer.";
            return weakFrameBuffer.toStrongRef();
        } else {
            kDebug() << "Found deleted cached frame buffer. Don't use.";
            m_frameBuffers.remove(id);
        }
    }

    // We don't already have that frame buffer.
    QMap<QString, FrameBufferPlugin *>::const_iterator iter = m_plugins.constBegin();

    while (iter != m_plugins.constEnd()) {

        if (iter.key() == KrfbConfig::preferredFrameBufferPlugin()) {
            kDebug() << "Using FrameBuffer:" << KrfbConfig::preferredFrameBufferPlugin();

            QSharedPointer<FrameBuffer> frameBuffer(iter.value()->frameBuffer(id));

            if (frameBuffer) {
                m_frameBuffers.insert(id, frameBuffer.toWeakRef());

                return frameBuffer;
            }
        }

        ++iter;
    }

    // No valid framebuffer plugin found.
    kDebug() << "No valid framebuffer found. returning null.";
    return QSharedPointer<FrameBuffer>();
}


#include "framebuffermanager.moc"

