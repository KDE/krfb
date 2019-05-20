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

#ifndef KRFB_EVENTSMANAGER_H
#define KRFB_EVENTSMANAGER_H

#include "events.h"

#include "krfbprivate_export.h"

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QWeakPointer>

#include <QWidget>

class EventsPlugin;
class KPluginFactory;

class KRFBPRIVATE_EXPORT EventsManager : public QObject
{
    Q_OBJECT
    friend class EventsManagerStatic;

public:
    static EventsManager *instance();

    virtual ~EventsManager();

    QSharedPointer<EventHandler> eventHandler();

private:
    Q_DISABLE_COPY(EventsManager)

    EventsManager();

    void loadPlugins();

    QMap<QString, EventsPlugin *> m_plugins;
    QList<QWeakPointer<EventHandler> > m_eventHandlers;
};


#endif  // Header guard
