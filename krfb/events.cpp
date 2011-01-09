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

#include "events.h"

#include <QtGui/QApplication>
#include <QtGui/QX11Info>
#include <QtGui/QDesktopWidget>
#include <KGlobal>

#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

enum {
    LEFTSHIFT = 1,
    RIGHTSHIFT = 2,
    ALTGR = 4
};

class EventData
{
public:
    EventData();

    //keyboard
    Display *dpy;
    signed char modifiers[0x100];
    KeyCode keycodes[0x100];
    KeyCode leftShiftCode;
    KeyCode rightShiftCode;
    KeyCode altGrCode;
    char modifierState;

    //mouse
    int buttonMask;

private:
    void init();
};

K_GLOBAL_STATIC(EventData, data)

EventData::EventData()
{
    init();
}

void EventData::init()
{
    dpy = QX11Info::display();
    buttonMask = 0;

    //initialize keycodes
    KeySym key, *keymap;
    int i, j, minkey, maxkey, syms_per_keycode;

    memset(modifiers, -1, sizeof(modifiers));

    XDisplayKeycodes(dpy, &minkey, &maxkey);
    Q_ASSERT(minkey >= 8);
    Q_ASSERT(maxkey < 256);
    keymap = (KeySym *) XGetKeyboardMapping(dpy, minkey,
                                            (maxkey - minkey + 1),
                                            &syms_per_keycode);
    Q_ASSERT(keymap);

    for (i = minkey; i <= maxkey; i++) {
        for (j = 0; j < syms_per_keycode; j++) {
            key = keymap[(i-minkey)*syms_per_keycode+j];

            if (key >= ' ' && key < 0x100 && i == XKeysymToKeycode(dpy, key)) {
                keycodes[key] = i;
                modifiers[key] = j;
            }
        }
    }

    leftShiftCode = XKeysymToKeycode(dpy, XK_Shift_L);
    rightShiftCode = XKeysymToKeycode(dpy, XK_Shift_R);
    altGrCode = XKeysymToKeycode(dpy, XK_Mode_switch);

    XFree((char *)keymap);
}

/* this function adjusts the modifiers according to mod (as from modifiers) and data->modifierState */
static void tweakModifiers(signed char mod, bool down)
{
    bool isShift = data->modifierState & (LEFTSHIFT | RIGHTSHIFT);

    if (mod < 0) {
        return;
    }

    if (isShift && mod != 1) {
        if (data->modifierState & LEFTSHIFT) {
            XTestFakeKeyEvent(data->dpy, data->leftShiftCode,
                              down, CurrentTime);
        }

        if (data->modifierState & RIGHTSHIFT) {
            XTestFakeKeyEvent(data->dpy, data->rightShiftCode,
                              down, CurrentTime);
        }
    }

    if (!isShift && mod == 1) {
        XTestFakeKeyEvent(data->dpy, data->leftShiftCode,
                          down, CurrentTime);
    }

    if ((data->modifierState & ALTGR) && mod != 2) {
        XTestFakeKeyEvent(data->dpy, data->altGrCode,
                          !down, CurrentTime);
    }

    if (!(data->modifierState & ALTGR) && mod == 2) {
        XTestFakeKeyEvent(data->dpy, data->altGrCode,
                          down, CurrentTime);
    }
}

void EventHandler::handleKeyboard(bool down, rfbKeySym keySym)
{
#define ADJUSTMOD(sym,state) \
    if(keySym==sym) { if(down) data->modifierState|=state; else data->modifierState&=~state; }

    ADJUSTMOD(XK_Shift_L, LEFTSHIFT);
    ADJUSTMOD(XK_Shift_R, RIGHTSHIFT);
    ADJUSTMOD(XK_Mode_switch, ALTGR);

    if (keySym >= ' ' && keySym < 0x100) {
        KeyCode k;

        if (down) {
            tweakModifiers(data->modifiers[keySym], True);
        }

        k = data->keycodes[keySym];

        if (k != NoSymbol) {
            XTestFakeKeyEvent(data->dpy, k, down, CurrentTime);
        }

        if (down) {
            tweakModifiers(data->modifiers[keySym], False);
        }
    } else {
        KeyCode k = XKeysymToKeycode(data->dpy, keySym);

        if (k != NoSymbol) {
            XTestFakeKeyEvent(data->dpy, k, down, CurrentTime);
        }
    }
}

void EventHandler::handlePointer(int buttonMask, int x, int y)
{
    QDesktopWidget *desktopWidget = QApplication::desktop();

    int screen = desktopWidget->screenNumber();

    if (screen < 0) {
        screen = 0;
    }

    XTestFakeMotionEvent(data->dpy, screen, x, y, CurrentTime);

    for (int i = 0; i < 5; i++) {
        if ((data->buttonMask&(1 << i)) != (buttonMask&(1 << i))) {
            XTestFakeButtonEvent(data->dpy,
                                 i + 1,
                                 (buttonMask&(1 << i)) ? True : False,
                                 CurrentTime);
        }
    }

    data->buttonMask = buttonMask;
}
