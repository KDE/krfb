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

#include "eventsmanager.h"

#include "eventsplugin.h"
#include "krfbconfig.h"
#include "rfbservermanager.h"
#include "krfbdebug.h"

#include <QGlobalStatic>

#include <KPluginFactory>
#include <KPluginMetaData>


class EventsManagerStatic
{
public:
    EventsManager instance;
};

Q_GLOBAL_STATIC(EventsManagerStatic, eventsManagerStatic)

EventsManager::EventsManager()
{
    const QList<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("krfb/events"), {}, KPluginMetaData::AllowEmptyMetaData);
    for (const KPluginMetaData &data : plugins) {
        const KPluginFactory::Result<EventsPlugin> result = KPluginFactory::instantiatePlugin<EventsPlugin>(data);
        if (result.plugin) {
            m_plugins.insert(data.pluginId(), result.plugin);
            qCDebug(KRFB) << "Loaded plugin with name " << data.pluginId();
        } else {
            qCDebug(KRFB) << "unable to load plugin for " << data.fileName() << result.errorString;
        }
    }
}

EventsManager::~EventsManager() = default;

EventsManager *EventsManager::instance()
{
    return &eventsManagerStatic->instance;
}

QSharedPointer<EventHandler> EventsManager::eventHandler()
{
    for (auto it = m_plugins.cbegin(); it != m_plugins.constEnd(); it++) {
        QSharedPointer<EventHandler> eventHandler(it.value()->eventHandler());
        if (eventHandler) {
            eventHandler->setFrameBufferPlugin(RfbServerManager::instance()->framebuffer());
            return eventHandler;
        }
    }

    // No valid events plugin found.
    qCDebug(KRFB) << "No valid event handlers found. returning null.";
    return QSharedPointer<EventHandler>();
}
