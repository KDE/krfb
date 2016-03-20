/*
   This file is part of the KDE project

   Copyright (C) 2016 Oleg Chernovskiy <kanedias@xaker.ru>

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

#include "fakeinputevents.h"
// Qt
#include <QApplication>
#include <QDesktopWidget>
#include <QGlobalStatic>
// KDE
#include <KLocalizedString>
// KWayland
#include <KWayland/Client/registry.h>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/fakeinput.h>

class EventData
{
public:
    EventData() = default;

    //mouse
    int buttonMask = 0;
    int oldX = 0;
    int oldY = 0;
};

FakeInputEventHandler::FakeInputEventHandler()
    : EventHandler(), m_eventData(new EventData())
{
    initWaylandConnection();
}

void FakeInputEventHandler::initWaylandConnection()
{
    using namespace KWayland::Client;
    ConnectionThread *conn = ConnectionThread::fromApplication(this);
    Registry *registry = new Registry(this);
    registry->create(conn);
    connect(registry, &Registry::fakeInputAnnounced, this,
        [this, registry] (qint32 name, qint32 version) {
            m_interface = registry->createFakeInput(name, version, this);
        }
    );
    registry->setup();
}

void FakeInputEventHandler::authIfNeeded()
{
    if (!m_authRequested) {
        m_interface->authenticate(i18n("Krfb"), i18n("Remote VNC input"));
        m_authRequested = true;
    }
}

void FakeInputEventHandler::handleKeyboard(bool down, rfbKeySym keySym)
{
    if (!m_interface)
        return;

    authIfNeeded();

    // TODO: implement button handling
    // both in FakeInput interface and here
    Q_UNUSED(down)
    Q_UNUSED(keySym)
}

void FakeInputEventHandler::handlePointer(int buttonMask, int x, int y)
{
    if (!m_interface)
        return;

    authIfNeeded();

    int oldMask = m_eventData->buttonMask;
    int oldX = m_eventData->oldX;
    int oldY = m_eventData->oldY;

    if (!(oldMask & 1) && buttonMask & 1) { // single left click
        m_interface->requestPointerButtonClick(Qt::LeftButton);
    }
    if (!(oldMask & 2) && buttonMask & 2) { // single right click
        m_interface->requestPointerButtonClick(Qt::RightButton);
    }
    if (!(oldMask & 4) && buttonMask & 4) { // single middle click
        m_interface->requestPointerButtonClick(Qt::MiddleButton);
    }
    if (!(oldMask & 8) && buttonMask & 8) { // wheel up
        m_interface->requestPointerAxis(Qt::Vertical, +10);
    }
    if (!(oldMask & 16) && buttonMask & 16) { // wheel down
        m_interface->requestPointerAxis(Qt::Vertical, -10);
    }

    if (oldX != x || oldY != y) {
        m_interface->requestPointerMove(QSizeF(x - oldX, y - oldY));
    }

    m_eventData->buttonMask = buttonMask;
}
