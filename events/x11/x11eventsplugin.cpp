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

#include "x11eventsplugin.h"

#include "x11events.h"

#include <KPluginFactory>
#include <QX11Info>

K_PLUGIN_CLASS_WITH_JSON(X11EventsPlugin, "krfb_events_x11.json")

X11EventsPlugin::X11EventsPlugin(QObject *parent, const QVariantList &args)
    : EventsPlugin(parent, args)
{
}

EventHandler *X11EventsPlugin::eventHandler()
{
    // works only under X11
    if(!QX11Info::isPlatformX11())
        return nullptr;

    return new X11EventHandler();
}

#include "x11eventsplugin.moc"

