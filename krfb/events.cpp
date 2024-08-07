/*
   This file is part of the KDE project

   SPDX-FileCopyrightText: 2010 Collabora Ltd.
   SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
   SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>
   SPDX-FileCopyrightText: 2001-2003 Tim Jansen <tim@tjansen.de>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "events.h"

EventHandler::EventHandler(QObject *parent)
    : QObject(parent)
{
}

void EventHandler::setFrameBufferPlugin(const QSharedPointer<FrameBuffer> &frameBuffer)
{
    fb = frameBuffer;
}

QSharedPointer<FrameBuffer> EventHandler::frameBuffer()
{
    return fb;
}
