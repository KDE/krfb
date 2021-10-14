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

#ifndef KRFB_FRAMEBUFFER_PW_PWFRAMEBUFFERPLUGIN_H
#define KRFB_FRAMEBUFFER_PW_PWFRAMEBUFFERPLUGIN_H


#include "framebufferplugin.h"
#include <QWidget>


class FrameBuffer;

class PWFrameBufferPlugin: public FrameBufferPlugin
{
   Q_OBJECT

public:
   PWFrameBufferPlugin(QObject *parent, const QVariantList &args);
   virtual ~PWFrameBufferPlugin() override;

   FrameBuffer *frameBuffer(WId id, const QVariantMap &args) override;

private:
   Q_DISABLE_COPY(PWFrameBufferPlugin)
};


#endif  // Header guard
