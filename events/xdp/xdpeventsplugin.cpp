/*
   This file is part of the KDE project

   Copyright (C) 2018-2019 Jan Grulich <jgrulich@redhat.com>

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

#include "xdpeventsplugin.h"

#include "xdpevents.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(XdpEventsPluginFactory, "krfb_events_xdp.json",
               registerPlugin<XdpEventsPlugin>();)

XdpEventsPlugin::XdpEventsPlugin(QObject *parent, const QVariantList &args)
    : EventsPlugin(parent, args)
{
}

EventHandler *XdpEventsPlugin::eventHandler()
{
    // works only under Wayland
    return new XdpEventHandler();
}

#include "xdpeventsplugin.moc"

