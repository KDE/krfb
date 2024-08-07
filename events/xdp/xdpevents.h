/*
   This file is part of the KDE project

   SPDX-FileCopyrightText: 2018-2019 Jan Grulich <jgrulich@redhat.com>

   SPDX-License-Identifier: GPL-2.0-or-later
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


