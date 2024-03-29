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

#ifndef LIB_KRFB_EVENTSPLUGIN_H
#define LIB_KRFB_EVENTSPLUGIN_H

#include "krfbprivate_export.h"

#include <QtCore/QVariantList>
#include <QWidget>

class EventHandler;

class KRFBPRIVATE_EXPORT EventsPlugin : public QObject
{
    Q_OBJECT
public:
    EventsPlugin(QObject *parent, const QVariantList &args);
    ~EventsPlugin() override;

    virtual EventHandler *eventHandler() = 0;
};

#endif  // Header guard

