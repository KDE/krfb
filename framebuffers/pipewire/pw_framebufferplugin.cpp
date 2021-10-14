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


K_PLUGIN_FACTORY_WITH_JSON(PWFrameBufferPluginFactory, "krfb_framebuffer_pw.json",
               registerPlugin<PWFrameBufferPlugin>();)

PWFrameBufferPlugin::PWFrameBufferPlugin(QObject *parent, const QVariantList &args)
    : FrameBufferPlugin(parent, args)
{
}


PWFrameBufferPlugin::~PWFrameBufferPlugin()
{
}

FrameBuffer *PWFrameBufferPlugin::frameBuffer(WId id, const QVariantMap &args)
{
    //NOTE WId is irrelevant in Wayland

    auto pwfb = new PWFrameBuffer(id);

    // sanity check for dbus/wayland/pipewire errors
    if (!pwfb->isValid()) {
        delete pwfb;
        return nullptr;
    }

    return pwfb;
}

#include "pw_framebufferplugin.moc"
