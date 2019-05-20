/* This file is part of the KDE project
   Copyright (C) 2016 Oleg Chernovskiy <kanedias@xaker.ru>

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

#ifndef KRFB_EVENTS_X11_X11EVENTSPLUGIN_H
#define KRFB_EVENTS_X11_X11EVENTSPLUGIN_H

#include "eventsplugin.h"

#include <QWidget>

class EventHandler;

class X11EventsPlugin : public EventsPlugin
{
    Q_OBJECT
public:
    X11EventsPlugin(QObject *parent, const QVariantList &args);
    virtual ~X11EventsPlugin() = default;

    EventHandler *eventHandler() override;

private:
    Q_DISABLE_COPY(X11EventsPlugin)
};


#endif  // Header guard
