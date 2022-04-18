/* This file is part of the KDE project
   Copyright (C) 2018 Oleg Chernovskiy <kanedias@xaker.ru>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; see the file COPYING.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*/


#include "pw_framebufferplugin.h"
#include "pw_framebuffer.h"
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(PWFrameBufferPlugin, "krfb_framebuffer_pw.json")

PWFrameBufferPlugin::PWFrameBufferPlugin(QObject *parent, const QVariantList &args)
    : FrameBufferPlugin(parent, args)
{
}


FrameBuffer *PWFrameBufferPlugin::frameBuffer(WId id, const QVariantMap &args)
{
    //NOTE WId is irrelevant in Wayland

    auto pwfb = new PWFrameBuffer(id);
    if (args.contains(QLatin1String("name"))) {
        pwfb->startVirtualMonitor(args[QStringLiteral("name")].toString(), args[QStringLiteral("resolution")].toSize(), args[QStringLiteral("scale")].toDouble());
    } else {
        // D-Bus is most important in XDG-Desktop-Portals init chain, no toys for us if something is wrong with XDP
        // PipeWire connectivity is initialized after D-Bus session is started
        pwfb->initDBus();
    }

    // sanity check for dbus/wayland/pipewire errors
    if (!pwfb->isValid()) {
        delete pwfb;
        return nullptr;
    }

    return pwfb;
}

#include "pw_framebufferplugin.moc"
