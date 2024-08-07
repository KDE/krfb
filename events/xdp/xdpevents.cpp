/*
   This file is part of the KDE project

   SPDX-FileCopyrightText: 2018-2019 Jan Grulich <jgrulich@redhat.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xdpevents.h"

#include "rfbservermanager.h"
#include "xdp_dbus_remotedesktop_interface.h"

#include <linux/input.h>

#include <QApplication>
#include <QGlobalStatic>

class EventData
{
public:
    EventData();

    //mouse
    int buttonMask = 0;
    int x = 0;
    int y = 0;

    QScopedPointer<OrgFreedesktopPortalRemoteDesktopInterface> dbusXdpRemoteDesktopService;

private:
    void init();
};

Q_GLOBAL_STATIC(EventData, data)

EventData::EventData()
{
    init();
}

void EventData::init()
{
    dbusXdpRemoteDesktopService.reset(new OrgFreedesktopPortalRemoteDesktopInterface(QStringLiteral("org.freedesktop.portal.Desktop"),
                                      QStringLiteral("/org/freedesktop/portal/desktop"), QDBusConnection::sessionBus()));
}

void XdpEventHandler::handleKeyboard(bool down, rfbKeySym keySym)
{
    const QDBusObjectPath sessionHandle = frameBuffer()->customProperty(QStringLiteral("session_handle")).value<QDBusObjectPath>();
    data->dbusXdpRemoteDesktopService->NotifyKeyboardKeysym(sessionHandle, {}, keySym, down);
}

void XdpEventHandler::handlePointer(int buttonMask, int x, int y)
{
    const uint streamNodeId = frameBuffer()->customProperty(QStringLiteral("stream_node_id")).toUInt();
    const QDBusObjectPath sessionHandle = frameBuffer()->customProperty(QStringLiteral("session_handle")).value<QDBusObjectPath>();

    if (streamNodeId == 0 || sessionHandle.path().isEmpty()) {
        return;
    }

    if (x != data->x || y != data->y) {
        data->dbusXdpRemoteDesktopService->NotifyPointerMotionAbsolute(sessionHandle, QVariantMap(), streamNodeId, x, y);
        data->x = x;
        data->y = y;
    }

    if (buttonMask != data->buttonMask) {
        int i = 0;
        QList<int> buttons = { BTN_LEFT, BTN_MIDDLE, BTN_RIGHT, 0, 0, 0, 0, BTN_SIDE, BTN_EXTRA };
        for (auto it = buttons.constBegin(); it != buttons.constEnd(); ++it) {
            int prevButtonState = (data->buttonMask >> i) & 0x01;
            int currentButtonState = (buttonMask >> i) & 0x01;

            if (prevButtonState != currentButtonState) {
                if (*it) {
                    data->dbusXdpRemoteDesktopService->NotifyPointerButton(sessionHandle, QVariantMap(), *it, buttonMask);
                } else {
                    int axis = 0;
                    int steps = 0;
                    switch (i) {
                    case 3:
                        axis = 0;   // Vertical
                        steps = -1;
                        break;
                    case 4:
                        axis = 0;   // Vertical
                        steps = 1;
                        break;
                    case 5:
                        axis = 1;   // Horizontal
                        steps = -1;
                        break;
                    case 6:
                        axis = 1;   // Horizontal
                        steps = 1;
                        break;
                        }

                    data->dbusXdpRemoteDesktopService->NotifyPointerAxisDiscrete(sessionHandle, QVariantMap(), axis, steps);
                }
            }
            ++i;
        }
        data->buttonMask = buttonMask;
    }
}
