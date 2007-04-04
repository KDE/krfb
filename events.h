/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
             (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef EVENTS_H
#define EVENTS_H

#include <X11/Xlib.h>

class CurrentController;

class VNCEvent {
public:
    virtual void exec() = 0;
    virtual ~VNCEvent();
};

class KeyboardEvent : public VNCEvent {
    bool down;
    KeySym keySym;

    static Display *dpy;
    static signed char modifiers[0x100];
    static KeyCode keycodes[0x100], leftShiftCode, rightShiftCode, altGrCode;
    static const int LEFTSHIFT;
    static const int RIGHTSHIFT;
    static const int ALTGR;
    static char ModifierState;

    static void tweakModifiers(signed char mod, bool down);
public:
    static void initKeycodes();

    KeyboardEvent(bool d, KeySym k);
    virtual void exec();
};

class PointerEvent : public VNCEvent {
    int button_mask, x, y;

    static bool initialized;
    static Display *dpy;
    static int buttonMask;
public:
    PointerEvent(int b, int _x, int _y);
    virtual void exec();
};

class ClipboardEvent : public VNCEvent {
    CurrentController *controller;
    QString text;
public:
    ClipboardEvent(CurrentController *c, const QString &text);
    virtual void exec();
};

#endif
