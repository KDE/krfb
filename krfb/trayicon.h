/*
    SPDX-FileCopyrightText: 2001-2002 Tim Jansen <tim@tjansen.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRAYICON_H
#define TRAYICON_H

#include <QHash>

#include <KStatusNotifierItem>

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
    explicit TrayIcon(QWidget *mainWindow);

public Q_SLOTS:
    void onClientConnected(RfbClient *client);
    void onClientDisconnected(RfbClient *client);
    void showAbout();

private:
    QAction *m_aboutAction;
    QHash<RfbClient*, ClientActions*> m_clientActions;
};

#endif
