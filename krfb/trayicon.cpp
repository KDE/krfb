/***************************************************************************
                                trayicon.cpp
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

#include "trayicon.h"
#include <kstdaction.h>
#include <kapp.h>
#include <klocale.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpopupmenu.h>

KPassivePopup2::KPassivePopup2(QWidget *parent) :
   KPassivePopup(parent){
}

void KPassivePopup2::closeEvent( QCloseEvent *e )
{
    KPassivePopup::closeEvent(e);
    emit closed();
}

KPassivePopup2 *KPassivePopup2::message( const QString &caption, const QString &text,
				         const QPixmap &icon,
				         QWidget *parent)
{
    KPassivePopup2 *pop = new KPassivePopup2( parent);
    pop->setView( caption, text, icon );
    pop->show();

    return pop;
}


TrayIcon::TrayIcon(KDialog *d, Configuration *c) : 
	KSystemTray(0, "krfb trayicon"),
	aboutDialog(d),
	actionCollection(this),
	quitting(false)
{
	KIconLoader *loader = KGlobal::iconLoader();
	trayIconOpen = loader->loadIcon("eyes-open24", KIcon::User);
	trayIconClosed = loader->loadIcon("eyes-closed24", KIcon::User);
	setPixmap(trayIconClosed);

	manageInvitationsAction = new KAction(i18n("Manage &invitations"), QString::null, 
					      0, this, SIGNAL(showManageInvitations()), 
					      &actionCollection);
	manageInvitationsAction->plug(contextMenu());

	contextMenu()->insertSeparator();
	aboutAction = KStdAction::aboutApp(this, SLOT(showAbout()), &actionCollection);
	aboutAction->plug(contextMenu());

	show();
}

TrayIcon::~TrayIcon(){
}

void TrayIcon::showAbout() {
	aboutDialog->show();
}

void TrayIcon::prepareQuit() {
        quitting = true;
}

void TrayIcon::showConnectedMessage() {
        setPixmap(trayIconOpen);
        KPassivePopup2 *p = KPassivePopup2::message(i18n("Desktop Sharing"), 
						    i18n("The remote user has been authenticated and is now connected."), 
						    trayIconOpen,
						    this);
}

void TrayIcon::showDisconnectedMessage() {
        if (quitting)
                return;

        setPixmap(trayIconClosed);
        KPassivePopup2 *p = KPassivePopup2::message(i18n("Desktop Sharing"), 
						    i18n("The remote user has closed the connection."), 
						    trayIconClosed,
						    this);
	connect(p, SIGNAL(closed()), this, SIGNAL(diconnectedMessageDisplayed()));
}

#include "trayicon.moc"
