/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Collabora Ltd <info@collabora.co.uk>
   SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>

   SPDX-License-Identifier: GPL-2.0-or-later
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
