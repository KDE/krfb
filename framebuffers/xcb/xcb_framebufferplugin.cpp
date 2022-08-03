/* This file is part of the KDE project
 Copyright (C) 2017 Alexey Min <alexey.min@gmail.com>

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


#include "xcb_framebufferplugin.h"
#include "xcb_framebuffer.h"
#include <KPluginFactory>

K_PLUGIN_CLASS(XCBFrameBufferPlugin)

XCBFrameBufferPlugin::XCBFrameBufferPlugin(QObject *parent, const QVariantList &args)
    : FrameBufferPlugin(parent, args)
{
}

FrameBuffer *XCBFrameBufferPlugin::frameBuffer(const QVariantMap &args)
{
    Q_UNUSED(args);
    return new XCBFrameBuffer;
}

#include "xcb_framebufferplugin.moc"

