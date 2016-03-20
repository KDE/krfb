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

#include "fakeinputeventsplugin.h"

#include "fakeinputevents.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(FakeInputEventsPluginFactory, "krfb_events_fakeinput.json",
               registerPlugin<FakeInputEventsPlugin>();)

FakeInputEventsPlugin::FakeInputEventsPlugin(QObject *parent, const QVariantList &args)
    : EventsPlugin(parent, args)
{
}

EventHandler *FakeInputEventsPlugin::eventHandler()
{
    // works only under Wayland
    return new FakeInputEventHandler();
}

#include "fakeinputeventsplugin.moc"
