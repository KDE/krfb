/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2016 Oleg Chernovskiy <kanedias@xaker.ru>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRFB_EVENTS_X11_X11EVENTSPLUGIN_H
#define KRFB_EVENTS_X11_X11EVENTSPLUGIN_H

#include "eventsplugin.h"

#include <QWidget>

class EventHandler;

class X11EventsPlugin : public EventsPlugin
{
    Q_OBJECT
public:
    X11EventsPlugin(QObject *parent, const QVariantList &args);

    EventHandler *eventHandler() override;

private:
    Q_DISABLE_COPY(X11EventsPlugin)
};


#endif  // Header guard
