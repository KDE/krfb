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
#include <qtooltip.h>
//Added by qt3to4:
#include <QHideEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <kstandardaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmenu.h>

KPassivePopup2::KPassivePopup2(QWidget *parent) :
   KPassivePopup(parent){
}

void KPassivePopup2::hideEvent( QHideEvent *e )
{
    KPassivePopup::hideEvent(e);
    emit hidden();
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
	KSystemTrayIcon(),
	configuration(c),
	aboutDialog(d),
	actionCollection(this),
	quitting(false)
{
	KIconLoader *loader = KIconLoader::global();
	trayIconOpen = loader->loadIcon("eyes-open24", K3Icon::User);
	trayIconClosed = loader->loadIcon("eyes-closed24", K3Icon::User);
	setIcon(trayIconClosed);
	setToolTip(i18n("Desktop Sharing - connecting"));

	manageInvitationsAction = new KAction(i18n("Manage &Invitations"), &actionCollection);
	actionCollection.addAction("manage_invitations", manageInvitationsAction);
	connect(manageInvitationsAction, SIGNAL(toggled()), SIGNAL(showManageInvitations()));
	contextMenu()->addAction(actionCollection.action("manage_invitations"));

	contextMenu()->addSeparator();

	enableControlAction = new KToggleAction(i18n("Enable Remote Control"), &actionCollection);
	enableControlAction->setCheckedState(KGuiItem(i18n("Disable Remote Control")));
	enableControlAction->setEnabled(false);
	actionCollection.addAction("enable_control", enableControlAction);
	connect(enableControlAction, SIGNAL(toggled(bool)), SIGNAL(enableDesktopControl(bool)));
	contextMenu()->addAction("enable_control");

	contextMenu()->addSeparator();

	aboutAction = KStandardAction::aboutApp(this, SLOT(showAbout()), &actionCollection);
	actionCollection.addAction("about", aboutAction);
	contextMenu()->addAction("about");

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



void TrayIcon::showConnectedMessage(QString host) {

        setIcon(trayIconOpen);
        KPassivePopup2::message(i18n("Desktop Sharing"),
				i18n("The remote user has been authenticated and is now connected."),
				trayIconOpen,
				(QWidget*)this);
	setToolTip(i18n("Desktop Sharing - connected with %1", host));
}

void TrayIcon::showDisconnectedMessage() {
        if (quitting)
                return;

	setToolTip(i18n("Desktop Sharing - disconnected"));
        setIcon(trayIconClosed);
        KPassivePopup2 *p = KPassivePopup2::message(i18n("Desktop Sharing"),
						    i18n("The remote user has closed the connection."),
						    trayIconClosed,
						    (QWidget*)this);
	connect(p, SIGNAL(hidden()), this, SIGNAL(diconnectedMessageDisplayed()));
}

void TrayIcon::setDesktopControlSetting(bool b) {
	enableControlAction->setEnabled(true);
	enableControlAction->setChecked(b);
}

void TrayIcon::activated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        contextMenu()->popup(QCursor::pos());
    }
    else
        KSystemTrayIcon::activated(reason);
}

#include "trayicon.moc"
