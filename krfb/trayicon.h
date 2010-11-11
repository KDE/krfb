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
class RfbClient;
class ClientActions;

/**
  * Implements the trayicon.
  * @author Tim Jansen
  */

class TrayIcon : public KStatusNotifierItem
{
    Q_OBJECT
public:
    TrayIcon(QWidget *mainWindow);

public Q_SLOTS:
    void onClientConnected(RfbClient *client);
    void onClientDisconnected(RfbClient *client);
    void showAbout();

private:
    KAction *m_aboutAction;
    QHash<RfbClient*, ClientActions*> m_clientActions;
};

#endif
