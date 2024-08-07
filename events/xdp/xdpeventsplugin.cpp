/*
   This file is part of the KDE project

   SPDX-FileCopyrightText: 2018-2019 Jan Grulich <jgrulich@redhat.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xdpeventsplugin.h"

#include "xdpevents.h"

#include <KPluginFactory>

K_PLUGIN_CLASS(XdpEventsPlugin)

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

