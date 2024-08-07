/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Collabora Ltd <info@collabora.co.uk>
   SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "framebufferplugin.h"

#include "framebuffer.h"

FrameBufferPlugin::FrameBufferPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
}

FrameBufferPlugin::~FrameBufferPlugin()
{
}
