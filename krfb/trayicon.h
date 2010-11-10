/***************************************************************************
                          trayicon.h  -  description
                             -------------------
    begin                : Tue Dec 11 2001
    copyright            : (C) 2001-2002 by Tim Jansen
    email                : tim@tjansen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRAYICON_H
#define TRAYICON_H

#include <KStatusNotifierItem>
#include <KToggleAction>

class KDialog;

/**
  * Implements the trayicon.
  * @author Tim Jansen
  */

class TrayIcon : public KStatusNotifierItem
{
    Q_OBJECT
public:
    TrayIcon(KDialog *);

signals:
    void disconnectedMessageDisplayed();
    void enableDesktopControl(bool);

public Q_SLOTS:
    void showConnectedMessage(const QString &host);
    void showDisconnectedMessage();
    void setDesktopControlSetting(bool);
    void showAbout();

private:
    KAction *aboutAction;
    KToggleAction *enableControlAction;
};

#endif
