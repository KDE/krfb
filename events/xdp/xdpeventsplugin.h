/*
   This file is part of the KDE project

   SPDX-FileCopyrightText: 2018-2019 Jan Grulich <jgrulich@redhat.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRFB_EVENTS_XDP_XDPEVENTSPLUGIN_H
#define KRFB_EVENTS_XDP_XDPEVENTSPLUGIN_H

#include "eventsplugin.h"

#include <QWidget>

class EventHandler;

class XdpEventsPlugin : public EventsPlugin
{
    Q_OBJECT
public:
    XdpEventsPlugin(QObject *parent, const QVariantList &args);

    EventHandler *eventHandler() override;

private:
    Q_DISABLE_COPY(XdpEventsPlugin)
};

#endif // Header guard
