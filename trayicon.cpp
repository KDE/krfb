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

#include <kstandardaction.h>
#include <klocale.h>
#include <kdialog.h>
#include <kmenu.h>
#include <kglobal.h>
#include <kactioncollection.h>
#include <kaboutapplicationdialog.h>

#include "manageinvitationsdialog.h"
#include "invitedialog.h"


TrayIcon::TrayIcon(KDialog *d) :
	KSystemTrayIcon(d),
	quitting(false)
{
	setIcon(KIcon("eyes-closed24"));

    setToolTip(i18n("Desktop Sharing - disconnected"));

// 	manageInvitationsAction = new KAction(i18n("Manage &Invitations"), &actionCollection);
// 	actionCollection.addAction("manage_invitations", manageInvitationsAction);
// 	connect(manageInvitationsAction, SIGNAL(triggered(bool)), SLOT(showManageInvitations()));
// 	contextMenu()->addAction(actionCollection.action("manage_invitations"));

// 	contextMenu()->addSeparator();

	enableControlAction = new KToggleAction(i18n("Enable Remote Control"), actionCollection());
	enableControlAction->setCheckedState(KGuiItem(i18n("Disable Remote Control")));
	enableControlAction->setEnabled(false);
	actionCollection()->addAction("enable_control", enableControlAction);
	connect(enableControlAction, SIGNAL(toggled(bool)), SIGNAL(enableDesktopControl(bool)));
	contextMenu()->addAction(actionCollection()->action("enable_control"));

    contextMenu()->addSeparator();
    contextMenu()->addAction(KStandardAction::aboutApp(this, SLOT(showAbout()), actionCollection()));

	show();
}

TrayIcon::~TrayIcon(){
}

void TrayIcon::showAbout() {
    KAboutApplicationDialog(KGlobal::mainComponent().aboutData()).exec();
}

void TrayIcon::prepareQuit() {
        quitting = true;
}

void TrayIcon::showConnectedMessage(const QString &host) {

        setIcon(KIcon("eyes-open24"));
        KPassivePopup::message(i18n("Desktop Sharing"),
				i18n("The remote user has been authenticated and is now connected."),
				KIcon("eyes-open24").pixmap(22),
				this);
	setToolTip(i18n("Desktop Sharing - connected with %1", host));
}

void TrayIcon::showDisconnectedMessage() {
        if (quitting)
                return;

	setToolTip(i18n("Desktop Sharing - disconnected"));
        setIcon(KIcon("eyes-closed24"));
        KPassivePopup *p = KPassivePopup::message(i18n("Desktop Sharing"),
						    i18n("The remote user has closed the connection."),
						    KIcon("eyes-closed24").pixmap(22),
						    this);
	connect(p, SIGNAL(hidden()), this, SIGNAL(diconnectedMessageDisplayed()));
}

void TrayIcon::setDesktopControlSetting(bool b) {
	enableControlAction->setEnabled(true);
	enableControlAction->setChecked(b);
}

void TrayIcon::showManageInvitations()
{
    ManageInvitationsDialog invMngDlg(0);
    invMngDlg.exec();
}

#include "trayicon.moc"