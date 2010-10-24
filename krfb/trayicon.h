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

#include <KActionCollection>
#include <KPassivePopup>
#include <KStatusNotifierItem>
#include <KToggleAction>

class KDialog;

/**
  * Implements the trayicon.
  * @author Tim Jansen
  */

class TrayIcon : public KStatusNotifierItem {
   	Q_OBJECT
public:
	TrayIcon(KDialog*);
	~TrayIcon();

signals:

        void disconnectedMessageDisplayed();
	void enableDesktopControl(bool);
    void quitApp();

public Q_SLOTS:
    void prepareQuit();
    void showConnectedMessage(const QString &host);
    void showDisconnectedMessage();
    void setDesktopControlSetting(bool);
    void showManageInvitations();
    void showAbout();

private:
  	KAction* manageInvitationsAction;
  	KAction* aboutAction;
	KToggleAction* enableControlAction;
	bool quitting;

};

#endif
