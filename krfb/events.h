/*
   This file is part of the KDE project

   Copyright (C) 2010 Collabora Ltd.
     @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
   Copyright (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

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

#ifndef EVENTS_H
#define EVENTS_H

#include "rfb.h"

class EventHandler
{
public:
    static void handleKeyboard(bool down, rfbKeySym key);
    static void handlePointer(int buttonMask, int x, int y);
};

#endif
