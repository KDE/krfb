/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Collabora Ltd <info@collabora.co.uk>
   SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LIB_KRFB_EVENTSPLUGIN_H
#define LIB_KRFB_EVENTSPLUGIN_H

#include "krfbprivate_export.h"

#include <QWidget>
#include <QtCore/QVariantList>

class EventHandler;

class KRFBPRIVATE_EXPORT EventsPlugin : public QObject
{
    Q_OBJECT
public:
    EventsPlugin(QObject *parent, const QVariantList &args);
    ~EventsPlugin() override;

    virtual EventHandler *eventHandler() = 0;
};

#endif // Header guard
