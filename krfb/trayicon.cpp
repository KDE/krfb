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
#include <kstdaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpopupmenu.h>

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
	KSystemTray(0, "krfb trayicon"),
	configuration(c),
	aboutDialog(d),
	actionCollection(this),
	quitting(false)
{
	KIconLoader *loader = KGlobal::iconLoader();
	trayIconOpen = loader->loadIcon("eyes-open24", KIcon::User);
	trayIconClosed = loader->loadIcon("eyes-closed24", KIcon::User);
	setPixmap(trayIconClosed);
	QToolTip::add(this, i18n("Desktop Sharing - connecting"));

	manageInvitationsAction = new KAction(i18n("Manage &Invitations"), QString::null,
					      0, this, SIGNAL(showManageInvitations()),
					      &actionCollection);
	manageInvitationsAction->plug(contextMenu());

	contextMenu()->insertSeparator();

	enableControlAction = new KToggleAction(i18n("Enable Remote Control"));
	enableControlAction->setCheckedState(i18n("Disable Remote Control"));
	enableControlAction->plug(contextMenu());
	enableControlAction->setEnabled(false);
	connect(enableControlAction, SIGNAL(toggled(bool)), SIGNAL(enableDesktopControl(bool)));

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



void TrayIcon::showConnectedMessage(QString host) {

        setPixmap(trayIconOpen);
        KPassivePopup2::message(i18n("Desktop Sharing"),
				i18n("The remote user has been authenticated and is now connected."),
				trayIconOpen,
				this);
	QToolTip::add(this, i18n("Desktop Sharing - connected with %1").arg(host));
}

void TrayIcon::showDisconnectedMessage() {
        if (quitting)
                return;

	QToolTip::add(this, i18n("Desktop Sharing - disconnected"));
        setPixmap(trayIconClosed);
        KPassivePopup2 *p = KPassivePopup2::message(i18n("Desktop Sharing"),
						    i18n("The remote user has closed the connection."),
						    trayIconClosed,
						    this);
	connect(p, SIGNAL(hidden()), this, SIGNAL(diconnectedMessageDisplayed()));
}

void TrayIcon::setDesktopControlSetting(bool b) {
	enableControlAction->setEnabled(true);
	enableControlAction->setChecked(b);
}

void TrayIcon::mousePressEvent(QMouseEvent *e)
{
        if (!rect().contains(e->pos()))
                return;

	if (e->button() == LeftButton) {
	        contextMenuAboutToShow(contextMenu());
		contextMenu()->popup(e->globalPos());
	}
	else
	        KSystemTray::mousePressEvent(e);
}

#include "trayicon.moc"
