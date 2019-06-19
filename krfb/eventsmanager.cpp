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

#include <QDebug>
#include <QGlobalStatic>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KPluginMetaData>

#include <QtCore/QSharedPointer>

class EventsManagerStatic
{
public:
    EventsManager instance;
};

Q_GLOBAL_STATIC(EventsManagerStatic, eventsManagerStatic)

EventsManager::EventsManager()
{
    //qDebug();

    loadPlugins();
}

EventsManager::~EventsManager()
{
    //qDebug();
}

EventsManager *EventsManager::instance()
{
    //qDebug();

    return &eventsManagerStatic->instance;
}

void EventsManager::loadPlugins()
{
    //qDebug();

    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("krfb"), [](const KPluginMetaData & md) {
            return md.serviceTypes().contains(QStringLiteral("krfb/events"));
        });

    QVectorIterator<KPluginMetaData> i(plugins);
    i.toBack();
    QSet<QString> unique;
    while (i.hasPrevious()) {
    KPluginMetaData data = i.previous();
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

        EventsPlugin *plugin = factory->create<EventsPlugin>(this);
        if (plugin) {
            m_plugins.insert(data.pluginId(), plugin);
            qDebug() << "Loaded plugin with name " << data.pluginId();
        } else {
            qDebug() << "unable to load plugin for " << data.fileName();
        }
        unique.insert (data.name());
    }
}

QSharedPointer<EventHandler> EventsManager::eventHandler()
{
    QMap<QString, EventsPlugin *>::const_iterator iter = m_plugins.constBegin();

    while (iter != m_plugins.constEnd()) {

        QSharedPointer<EventHandler> eventHandler(iter.value()->eventHandler());

        if (eventHandler) {
            eventHandler->setFrameBufferPlugin(RfbServerManager::instance()->framebuffer());
            return eventHandler;
        }

        ++iter;
    }

    // No valid events plugin found.
    qDebug() << "No valid event handlers found. returning null.";
    return QSharedPointer<EventHandler>();
}
