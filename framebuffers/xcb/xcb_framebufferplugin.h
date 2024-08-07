 /* This file is part of the KDE project
  SPDX-FileCopyrightText: Alexey Min <alexey.min@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later
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
