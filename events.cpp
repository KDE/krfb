/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
             (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include <QApplication>
#include <QX11Info>
#include <QString>
#include <QDesktopWidget>
#include <QClipboard>

#include <KNotification>

#include "events.h"
#include "connectioncontroller.h"

#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>


Display *KeyboardEvent::dpy;
signed char KeyboardEvent::modifiers[0x100];
KeyCode KeyboardEvent::keycodes[0x100];
KeyCode KeyboardEvent::leftShiftCode;
KeyCode KeyboardEvent::rightShiftCode;
KeyCode KeyboardEvent::altGrCode;
const int KeyboardEvent::LEFTSHIFT = 1;
const int KeyboardEvent::RIGHTSHIFT = 2;
const int KeyboardEvent::ALTGR = 4;
char KeyboardEvent::ModifierState;
bool KeyboardEvent::initDone = false;


KeyboardEvent::KeyboardEvent(bool d, KeySym k)
    : down(d), keySym(k)
{
    initKeycodes();
}

void KeyboardEvent::initKeycodes()
{
    if (initDone) return;
    initDone = true;
    KeySym key,*keymap;
    int i,j,minkey,maxkey,syms_per_keycode;

    dpy = QX11Info::display();

    memset(modifiers,-1,sizeof(modifiers));

    XDisplayKeycodes(dpy,&minkey,&maxkey);
    Q_ASSERT(minkey >= 8);
    Q_ASSERT(maxkey < 256);
    keymap = (KeySym*) XGetKeyboardMapping(dpy, minkey,
            (maxkey - minkey + 1),
            &syms_per_keycode);
    Q_ASSERT(keymap);

    for (i = minkey; i <= maxkey; i++) {
        for (j=0; j<syms_per_keycode; j++) {
            key = keymap[(i-minkey)*syms_per_keycode+j];
            if (key>=' ' && key<0x100 && i==XKeysymToKeycode(dpy,key)) {
                keycodes[key]=i;
                modifiers[key]=j;
            }
        }
    }

    leftShiftCode = XKeysymToKeycode(dpy, XK_Shift_L);
    rightShiftCode = XKeysymToKeycode(dpy, XK_Shift_R);
    altGrCode = XKeysymToKeycode(dpy, XK_Mode_switch);

    XFree ((char *)keymap);
}

/* this function adjusts the modifiers according to mod (as from modifiers) and ModifierState */
void KeyboardEvent::tweakModifiers(signed char mod, bool down)
{

    bool isShift = ModifierState & (LEFTSHIFT|RIGHTSHIFT);
    if(mod < 0)
        return;

    if(isShift && mod != 1) {
        if(ModifierState & LEFTSHIFT) {
            XTestFakeKeyEvent(dpy, leftShiftCode,
                            !down, CurrentTime);
        }
        if(ModifierState & RIGHTSHIFT) {
            XTestFakeKeyEvent(dpy, rightShiftCode,
                            !down, CurrentTime);
        }
    }

    if(!isShift && mod==1) {
        XTestFakeKeyEvent(dpy, leftShiftCode,
                        down, CurrentTime);
    }

    if((ModifierState&ALTGR) && mod != 2) {
        XTestFakeKeyEvent(dpy, altGrCode,
                        !down, CurrentTime);
    }

    if(!(ModifierState&ALTGR) && mod==2) {
        XTestFakeKeyEvent(dpy, altGrCode,
                        down, CurrentTime);
    }
}

void KeyboardEvent::exec() {
#define ADJUSTMOD(sym,state) \
    if(keySym==sym) { if(down) ModifierState|=state; else ModifierState&=~state; }

    ADJUSTMOD(XK_Shift_L,LEFTSHIFT);
    ADJUSTMOD(XK_Shift_R,RIGHTSHIFT);
    ADJUSTMOD(XK_Mode_switch,ALTGR);

    if(keySym>=' ' && keySym<0x100) {
        KeyCode k;
        if (down) {
            tweakModifiers(modifiers[keySym],True);
        }
        k = keycodes[keySym];
        if (k != NoSymbol) {
            XTestFakeKeyEvent(dpy, k, down, CurrentTime);
        }
        if (down) {
            tweakModifiers(modifiers[keySym],False);
        }
    } else {
        KeyCode k = XKeysymToKeycode(dpy, keySym );
        if (k != NoSymbol) {
            XTestFakeKeyEvent(dpy, k, down, CurrentTime);
        }
    }
}

bool PointerEvent::initialized = false;
Display *PointerEvent::dpy;
int PointerEvent::buttonMask = 0;

PointerEvent::PointerEvent(int b, int _x, int _y)
    : button_mask(b),x(_x),y(_y)
{
    if (!initialized) {
        initialized = true;
        dpy = QX11Info::display();
        buttonMask = 0;
    }
}

void PointerEvent::exec() {
    QDesktopWidget *desktopWidget = QApplication::desktop();

    int screen = desktopWidget->screenNumber();
    if (screen < 0)
        screen = 0;
    XTestFakeMotionEvent(dpy, screen, x, y, CurrentTime);

    for(int i = 0; i < 5; i++) {
        if ((buttonMask&(1<<i))!=(button_mask&(1<<i))) {
            XTestFakeButtonEvent(dpy,
                i+1,
                (button_mask&(1<<i))?True:False,
                CurrentTime);
        }
    }

    buttonMask = button_mask;
}


ClipboardEvent::ClipboardEvent(ConnectionController *c, const QString &ctext)
    :controller(c),text(ctext)
{
}

void ClipboardEvent::exec()
{
#if 0
    if ((controller->lastClipboardDirection == ConnectionController::LAST_SYNC_TO_CLIENT) &&
        (controller->lastClipboardText == text)) {
        return;
    }
    controller->lastClipboardDirection = ConnectionController::LAST_SYNC_TO_SERVER;
    controller->lastClipboardText = text;

    controller->clipboard->setText(text, QClipboard::Clipboard);
    controller->clipboard->setText(text, QClipboard::Selection);
#endif
}


VNCEvent::~ VNCEvent()
{
}
