/***************************************************************************
                                trayicon.cpp
                             -------------------
    begin                : Tue Dec 11 2001
    copyright            : (C) 2001 by Tim Jansen
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

#include "trayicon.h"
#include <kstdaction.h>
#include <kapp.h>
#include <klocale.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpopupmenu.h>

TrayIcon::TrayIcon(KDialog *d, Configuration *c) : 
	KSystemTray(0, "krfb trayicon"),
	aboutDialog(d),
	actionCollection(this)
{
	KIconLoader *loader = KGlobal::iconLoader();
	trayIconOpen = loader->loadIcon("eyes-open24", KIcon::User);
	trayIconClosed = loader->loadIcon("eyes-closed24", KIcon::User);
	setPixmap(trayIconClosed);

	configureAction = KStdAction::preferences(this, SIGNAL(showConfigure()), &actionCollection);
	manageInvitationsAction = new KAction(i18n("Manage &invitations"), QString::null, 0, this, SIGNAL(showManageInvitations()), &actionCollection);
	if (c->mode() != KRFB_STAND_ALONE_CMDARG) {
		configureAction->plug(contextMenu());
		manageInvitationsAction->plug(contextMenu());
	}

	closeConnectionAction = new KAction(i18n("Cl&ose connection"), QString::null, 0, this, SLOT(connectionClose), &actionCollection);
	closeConnectionAction->plug(contextMenu());
	closeConnectionAction->setEnabled(false);

	contextMenu()->insertSeparator();
	aboutAction = KStdAction::aboutApp(this, SLOT(showAbout()), &actionCollection);
	aboutAction->plug(contextMenu());

	show();
}

TrayIcon::~TrayIcon(){
}

void TrayIcon::openConnection(){
	setPixmap(trayIconOpen);
	closeConnectionAction->setEnabled(true);
}

void TrayIcon::closeConnection(){
	setPixmap(trayIconClosed);
	closeConnectionAction->setEnabled(false);
}

void TrayIcon::showAbout() {
	aboutDialog->show();
}
#include "trayicon.moc"
