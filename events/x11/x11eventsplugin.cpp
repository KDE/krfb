/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2016 Oleg Chernovskiy <kanedias@xaker.ru>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "x11eventsplugin.h"

#include "x11events.h"

#include <KPluginFactory>

#include <QtGui/private/qtx11extras_p.h>

K_PLUGIN_CLASS(X11EventsPlugin)

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

