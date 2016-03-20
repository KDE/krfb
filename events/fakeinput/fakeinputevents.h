/*
   This file is part of the KDE project

   Copyright (C) 2016 by Oleg Chernovskiy <kanedias@xaker.ru>

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

#ifndef EVENTS_GBMEVENTS_H
#define EVENTS_GBMEVENTS_H

#include "../../krfb/events.h"

#include <QScopedPointer>

namespace KWayland
{
namespace Client
{
class FakeInput;
}
}

class EventData;

class FakeInputEventHandler : public EventHandler
{
public:
    FakeInputEventHandler();
    virtual void handleKeyboard(bool down, rfbKeySym key);
    virtual void handlePointer(int buttonMask, int x, int y);
private:
    void initWaylandConnection();
    void authIfNeeded();

    QScopedPointer<EventData> m_eventData;
    KWayland::Client::FakeInput *m_interface = nullptr;
    bool m_authRequested = false;
};

#endif
