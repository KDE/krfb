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

#ifndef KRFB_FRAMEBUFFERMANAGER_H
#define KRFB_FRAMEBUFFERMANAGER_H

#include "framebuffer.h"

#include <kdemacros.h>

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QWeakPointer>

#include <QtGui/QWidget>

class FrameBufferPlugin;
class KPluginFactory;

class KDE_EXPORT FrameBufferManager : public QObject
{
    Q_OBJECT
    friend class FrameBufferManagerStatic;

public:
    static FrameBufferManager *instance();

    virtual ~FrameBufferManager();

    QSharedPointer<FrameBuffer> frameBuffer(WId id);

private:
    Q_DISABLE_COPY(FrameBufferManager)

    FrameBufferManager();

    void loadPlugins();

    QMap<QString, FrameBufferPlugin *> m_plugins;
    QMap<WId, QWeakPointer<FrameBuffer> > m_frameBuffers;
};


#endif  // Header guard

