/*
   This file is part of the KDE project

   Copyright (C) 2018-2019 Jan Grulich <jgrulich@redhat.com>

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

#ifndef EVENTS_XDPEVENTS_H
#define EVENTS_XDPEVENTS_H

#include "../../krfb/events.h"

class XdpEventHandler : public EventHandler
{
    Q_OBJECT
public:
    void handleKeyboard(bool down, rfbKeySym key) override;
    void handlePointer(int buttonMask, int x, int y) override;
};

#endif


