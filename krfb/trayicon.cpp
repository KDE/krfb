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

#include "invitedialog.h"
#include "manageinvitationsdialog.h"

#include <KAboutApplicationDialog>
#include <KActionCollection>
#include <KDialog>
#include <KGlobal>
#include <KLocale>
#include <KMenu>
#include <KNotification>
#include <KStandardAction>

TrayIcon::TrayIcon(KDialog *d)
    : KStatusNotifierItem(d),
      quitting(false)
{
    setIconByPixmap(KIcon("krfb").pixmap(22, 22, KIcon::Disabled));

    setToolTipTitle(i18n("Desktop Sharing - disconnected"));
    setCategory(KStatusNotifierItem::ApplicationStatus);
//  manageInvitationsAction = new KAction(i18n("Manage &Invitations"), &actionCollection);
//  actionCollection.addAction("manage_invitations", manageInvitationsAction);
//  connect(manageInvitationsAction, SIGNAL(triggered(bool)), SLOT(showManageInvitations()));
//  contextMenu()->addAction(actionCollection.action("manage_invitations"));

//  contextMenu()->addSeparator();

    enableControlAction = new KToggleAction(i18n("Enable Remote Control"), actionCollection());
    enableControlAction->setCheckedState(KGuiItem(i18n("Disable Remote Control")));
    enableControlAction->setEnabled(false);
    actionCollection()->addAction("enable_control", enableControlAction);
    connect(enableControlAction, SIGNAL(toggled(bool)), SIGNAL(enableDesktopControl(bool)));
    contextMenu()->addAction(actionCollection()->action("enable_control"));

    contextMenu()->addSeparator();
    contextMenu()->addAction(KStandardAction::aboutApp(this, SLOT(showAbout()), actionCollection()));

}

TrayIcon::~TrayIcon()
{
}

void TrayIcon::showAbout()
{
    KAboutApplicationDialog(KGlobal::mainComponent().aboutData()).exec();
}

void TrayIcon::prepareQuit()
{
    quitting = true;
}

void TrayIcon::showConnectedMessage(const QString &host)
{

    setIconByPixmap(KIcon("krfb"));
    KNotification::event("UserAcceptsConnection",
                         i18n("The remote user %1 is now connected.",
                              host));
    setToolTipTitle(i18n("Desktop Sharing - connected with %1", host));

    setStatus(KStatusNotifierItem::Active);
}

void TrayIcon::showDisconnectedMessage()
{
    if (quitting) {
        return;
    }

    setToolTipTitle(i18n("Desktop Sharing - disconnected"));
    setIconByPixmap(KIcon("krfb").pixmap(22, 22, KIcon::Disabled));
    KNotification::event("ConnectionClosed", i18n("The remote user has closed the connection."));

    setStatus(KStatusNotifierItem::Passive);

    Q_EMIT disconnectedMessageDisplayed();
}

void TrayIcon::setDesktopControlSetting(bool b)
{
    enableControlAction->setEnabled(true);
    enableControlAction->setChecked(b);
}

void TrayIcon::showManageInvitations()
{
    ManageInvitationsDialog invMngDlg(0);
    invMngDlg.exec();
}

#include "trayicon.moc"
