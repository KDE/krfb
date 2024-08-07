/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2018 Oleg Chernovskiy <kanedias@xaker.ru>

   SPDX-License-Identifier: GPL-3.0-or-later
*/


#include "pw_framebufferplugin.h"
#include "pw_framebuffer.h"
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(PWFrameBufferPlugin, "pipewire.json")

PWFrameBufferPlugin::PWFrameBufferPlugin(QObject *parent, const QVariantList &args)
    : FrameBufferPlugin(parent, args)
{
}


FrameBuffer *PWFrameBufferPlugin::frameBuffer(const QVariantMap &args)
{
    auto pwfb = new PWFrameBuffer;
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
