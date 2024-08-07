/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Collabora Ltd <info@collabora.co.uk>
   SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LIB_KRFB_FRAMEBUFFERPLUGIN_H
#define LIB_KRFB_FRAMEBUFFERPLUGIN_H

#include "krfbprivate_export.h"

#include <QVariantList>
#include <QWidget>


class FrameBuffer;

class KRFBPRIVATE_EXPORT FrameBufferPlugin : public QObject
{
    Q_OBJECT

public:
    explicit FrameBufferPlugin(QObject *parent, const QVariantList &args);
    ~FrameBufferPlugin() override;

    virtual FrameBuffer *frameBuffer(const QVariantMap &args) = 0;
};

#endif  // Header guard

