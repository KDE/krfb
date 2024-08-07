/* This file is part of the KDE project
 SPDX-FileCopyrightText: 2017 Alexey Min <alexey.min@gmail.com>

 SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "xcb_framebufferplugin.h"
#include "xcb_framebuffer.h"
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(XCBFrameBufferPlugin, "xcb.json")

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

