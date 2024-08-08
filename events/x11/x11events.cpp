/*
   This file is part of the KDE project

   SPDX-FileCopyrightText: 2016 Oleg Chernovskiy <kanedias@xaker.ru>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "x11events.h"

#include <QApplication>
#include <QGlobalStatic>
#include <QtGui/private/qtx11extras_p.h>

#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>

enum {
    LEFTSHIFT = 1,
    RIGHTSHIFT = 2,
    ALTGR = 4,
};

class EventData
{
public:
    EventData();

    // keyboard
    Display *dpy = nullptr;
    signed char modifiers[0x100] = {};
    KeyCode keycodes[0x100] = {};
    KeyCode leftShiftCode = 0;
    KeyCode rightShiftCode = 0;
    KeyCode altGrCode = 0;
    char modifierState = 0;

    // mouse
    int buttonMask = 0;
    int x = 0;
    int y = 0;

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
    buttonMask = 0;

    dpy = QX11Info::display();
    // initialize keycodes
    KeySym key, *keymap;
    int i, j, minkey, maxkey, syms_per_keycode;

    memset(modifiers, -1, sizeof(modifiers));

    XDisplayKeycodes(dpy, &minkey, &maxkey);
    Q_ASSERT(minkey >= 8);
    Q_ASSERT(maxkey < 256);
    keymap = (KeySym *)XGetKeyboardMapping(dpy, minkey, (maxkey - minkey + 1), &syms_per_keycode);
    Q_ASSERT(keymap);

    for (i = minkey; i <= maxkey; i++) {
        for (j = 0; j < syms_per_keycode; j++) {
            key = keymap[(i - minkey) * syms_per_keycode + j];

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
            XTestFakeKeyEvent(data->dpy, data->leftShiftCode, down, CurrentTime);
        }

        if (data->modifierState & RIGHTSHIFT) {
            XTestFakeKeyEvent(data->dpy, data->rightShiftCode, down, CurrentTime);
        }
    }

    if (!isShift && mod == 1) {
        XTestFakeKeyEvent(data->dpy, data->leftShiftCode, down, CurrentTime);
    }

    if ((data->modifierState & ALTGR) && mod != 2) {
        XTestFakeKeyEvent(data->dpy, data->altGrCode, !down, CurrentTime);
    }

    if (!(data->modifierState & ALTGR) && mod == 2) {
        XTestFakeKeyEvent(data->dpy, data->altGrCode, down, CurrentTime);
    }
}

void X11EventHandler::handleKeyboard(bool down, rfbKeySym keySym)
{
#define ADJUSTMOD(sym, state)                                                                                                                                  \
    if (keySym == sym) {                                                                                                                                       \
        if (down)                                                                                                                                              \
            data->modifierState |= state;                                                                                                                      \
        else                                                                                                                                                   \
            data->modifierState &= ~state;                                                                                                                     \
    }

    if (QX11Info::isPlatformX11()) {
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
    /*
        // Wayland platform and pipweire plugin in use
        if (KrfbConfig::preferredFrameBufferPlugin() == QStringLiteral("pw")) {

        }*/
}

void X11EventHandler::handlePointer(int buttonMask, int x, int y)
{
    if (QX11Info::isPlatformX11()) {
        XTestFakeMotionEvent(data->dpy, 0, x, y, CurrentTime);

        for (int i = 0; i < 5; i++) {
            if ((data->buttonMask & (1 << i)) != (buttonMask & (1 << i))) {
                XTestFakeButtonEvent(data->dpy, i + 1, (buttonMask & (1 << i)) ? True : False, CurrentTime);
            }
        }

        data->buttonMask = buttonMask;
    }
}
