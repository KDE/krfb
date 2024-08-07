/*
   This file is part of the KDE project

   SPDX-FileCopyrightText: 2016 Oleg Chernovskiy <kanedias@xaker.ru>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EVENTS_X11EVENTS_H
#define EVENTS_X11EVENTS_H

#include "../../krfb/events.h"

class X11EventHandler : public EventHandler
{
    Q_OBJECT
public:
    explicit X11EventHandler(QObject *parent = nullptr)
        : EventHandler(parent)
    {
    };

    void handleKeyboard(bool down, rfbKeySym key) override;
    void handlePointer(int buttonMask, int x, int y) override;
};

#endif

