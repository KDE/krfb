/* This file is part of the KDE project
   Copyright (C) 2016 Oleg Chernovskiy <adonai@xaker.ru>

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

#include "gbmframebufferplugin.h"

#include "gbmframebuffer.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(GbmFrameBufferPluginFactory, "krfb_framebuffer_gbm.json",
               registerPlugin<GbmFrameBufferPlugin>();)

GbmFrameBufferPlugin::GbmFrameBufferPlugin(QObject *parent, const QVariantList &args)
    : FrameBufferPlugin(parent, args)
{
}

GbmFrameBufferPlugin::~GbmFrameBufferPlugin()
{
}

FrameBuffer *GbmFrameBufferPlugin::frameBuffer(WId id)
{
    auto p = new GbmFrameBuffer(id);
    if (!p->isValid()) {
        delete p;
        return nullptr;
    }

    return p;
}

#include "gbmframebufferplugin.moc"

