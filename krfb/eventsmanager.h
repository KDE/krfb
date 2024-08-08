/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Collabora Ltd <info@collabora.co.uk>
   SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>

   SPDX-License-Identifier: GPL-2.0-or-later
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

    ~EventsManager() override;

    QSharedPointer<EventHandler> eventHandler();

private:
    Q_DISABLE_COPY(EventsManager)

    EventsManager();

    QMap<QString, EventsPlugin *> m_plugins;
    QList<QWeakPointer<EventHandler>> m_eventHandlers;
};

#endif // Header guard
