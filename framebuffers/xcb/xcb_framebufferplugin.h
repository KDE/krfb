 /* This file is part of the KDE project
  @author Alexey Min <alexey.min@gmail.com>

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

#ifndef KRFB_FRAMEBUFFER_XCB_XCBFRAMEBUFFERPLUGIN_H
#define KRFB_FRAMEBUFFER_XCB_XCBFRAMEBUFFERPLUGIN_H


#include "framebufferplugin.h"
#include <QWidget>


class FrameBuffer;

class XCBFrameBufferPlugin: public FrameBufferPlugin
{
   Q_OBJECT

public:
   XCBFrameBufferPlugin(QObject *parent, const QVariantList &args);

   FrameBuffer *frameBuffer(const QVariantMap &args) override;

private:
   Q_DISABLE_COPY(XCBFrameBufferPlugin)
};


#endif  // Header guard
