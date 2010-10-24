/* This file is part of the KDE project
   Copyright (C) 2009 Collabora Ltd <info@collabora.co.uk>
    @author George Goldberg <george.goldberg@collabora.co.uk>

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

#include "qtframebufferplugin.h"

#include "qtframebuffer.h"

#include <KGenericFactory>


QtFrameBufferPlugin::QtFrameBufferPlugin(QObject *parent, const QVariantList &args)
    : FrameBufferPlugin(parent, args)
{
}

QtFrameBufferPlugin::~QtFrameBufferPlugin()
{
}

FrameBuffer *QtFrameBufferPlugin::frameBuffer(WId id)
{
    return new QtFrameBuffer(id);
}

K_PLUGIN_FACTORY(factory, registerPlugin<QtFrameBufferPlugin>();) \
K_EXPORT_PLUGIN(factory("krfb_framebuffer_qt"))


#include "qtframebufferplugin.moc"

