 /* This file is part of the KDE project
    SPDX-FileCopyrightText: 2018 Oleg Chernovskiy <kanedias@xaker.ru>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef KRFB_FRAMEBUFFER_PW_PWFRAMEBUFFERPLUGIN_H
#define KRFB_FRAMEBUFFER_PW_PWFRAMEBUFFERPLUGIN_H


#include "framebufferplugin.h"

class FrameBuffer;

class PWFrameBufferPlugin: public FrameBufferPlugin
{
   Q_OBJECT

public:
   PWFrameBufferPlugin(QObject *parent, const QVariantList &args);

   FrameBuffer *frameBuffer(const QVariantMap &args) override;

private:
   Q_DISABLE_COPY(PWFrameBufferPlugin)
};


#endif  // Header guard
