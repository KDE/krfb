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
#include <kdebug.h>
#include <klocale.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpopupmenu.h>

TrayIcon::TrayIcon(KDialog *d, Configuration *c) : 
	KSystemTray(),
	aboutDialog(d) 
{
	KIconLoader *loader = KGlobal::iconLoader();
	trayIconOpen = loader->loadIcon("eyes-open24", KIcon::User);
	trayIconClosed = loader->loadIcon("eyes-closed24", KIcon::User);
	setPixmap(trayIconClosed);

	configureAction = new KAction(i18n("&Configure KRfb")); 	
	if (!c->preconfigured()) 
		configureAction->plug(contextMenu());

	closeConnectionAction = new KAction(i18n("Close connection"));
	closeConnectionAction->plug(contextMenu());
	closeConnectionAction->setEnabled(false);

	contextMenu()->insertSeparator();
	aboutAction = new KAction(i18n("&About KRfb")); 	
	aboutAction->plug(contextMenu());
	connect(configureAction, SIGNAL(activated()), SIGNAL(showConfigure()));
	connect(aboutAction, SIGNAL(activated()), SLOT(showAbout()));
	connect(closeConnectionAction, SIGNAL(activated()), 
		SIGNAL(connectionClosed()));
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
