/***************************************************************************
                              configuration.cpp
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

#include "configuration.h"

#include <kglobal.h>
#include <klocale.h>
#include <kapplication.h>
#include <kprocess.h>

#include <qdatastream.h>
#include <dcopclient.h>

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>

ManageInvitationsDialog2::ManageInvitationsDialog2() :
  ManageInvitationsDialog(0, 0, false, WShowModal)
{ }
void ManageInvitationsDialog2::closeEvent(QCloseEvent *)
{ emit closed(); }

InvitationDialog2::InvitationDialog2() :
  InvitationDialog(0, 0, false, WShowModal)
{ }
void InvitationDialog2::closeEvent(QCloseEvent *)
{ emit closed(); }

PersonalInvitationDialog2::PersonalInvitationDialog2() :
  PersonalInvitationDialog(0, 0, false, WShowModal)
{ }
void PersonalInvitationDialog2::closeEvent(QCloseEvent *)
{ emit closed(); }


Configuration::Configuration(krfb_mode mode) :
	m_mode(mode),
	portNum(-1)
{
	loadFromKConfig();
	saveToDialogs();
	doKinetdConf();

	connect(invMngDlg.closeButton, SIGNAL(clicked()), SLOT(invMngDlgClosed()));
	connect(&invMngDlg, SIGNAL(closed()), SLOT(invMngDlgClosed()));
	connect(invMngDlg.newPersonalInvitationButton, SIGNAL(clicked()), 
		SLOT(showPersonalInvitationDialog()));
	connect(invMngDlg.newEmailInvitationButton, SIGNAL(clicked()), SLOT(inviteEmail()));
	connect(invMngDlg.deleteOneButton, SIGNAL(clicked()), SLOT(invMngDlgDeleteOnePressed()));
	connect(invMngDlg.deleteAllButton, SIGNAL(clicked()), SLOT(invMngDlgDeleteAllPressed()));
	invMngDlg.listView->setSelectionMode(QListView::Extended);
	invMngDlg.listView->setMinimumSize(QSize(400, 100)); // QTs size is much to small

	connect(invDlg.closeButton, SIGNAL(clicked()), SLOT(invDlgClosed()));
	connect(&invDlg, SIGNAL(closed()), SLOT(invDlgClosed()));
	connect(invDlg.createInvitationButton, SIGNAL(clicked()),
		SLOT(showPersonalInvitationDialog()));
	connect(invDlg.createInvitationEMailButton, SIGNAL(clicked()),
		SLOT(inviteEmail()));
	connect(invDlg.manageInvitationButton, SIGNAL(clicked()),
		SLOT(showManageInvitationsDialog()));
	connect(this, SIGNAL(invitationNumChanged(int)), this, SLOT(changeInvDlgNum(int)));
	changeInvDlgNum(invitationList.size());

	connect(persInvDlg.closeButton, SIGNAL(clicked()), SLOT(persInvDlgClosed()));
	connect(&persInvDlg, SIGNAL(closed()), SLOT(persInvDlgClosed()));

	connect(&refreshTimer, SIGNAL(timeout()), SLOT(refreshTimeout()));
	refreshTimer.start(1000*60);
}

Configuration::~Configuration() {
        save();
}

void Configuration::setKInetdEnabled(bool enabled) {
	DCOPClient *d = KApplication::dcopClient();

	QByteArray sdata;
	QDataStream arg(sdata, IO_WriteOnly);
	arg << QString("krfb");
	arg << enabled;
	d->send ("kded", "kinetd", "setEnabled(QString,bool)", sdata);
}

void Configuration::setKInetdEnabled(const QDateTime &date) {
	DCOPClient *d = KApplication::dcopClient();

	QByteArray sdata;
	QDataStream arg(sdata, IO_WriteOnly);
	arg << QString("krfb");
	arg << date;
	d->send ("kded", "kinetd", "setEnabled(QString,QDateTime)", sdata);
}

void Configuration::getPortFromKInetd() {
	DCOPClient *d = KApplication::dcopClient();

	QByteArray sdata, rdata;
	QCString replyType;
	QDataStream arg(sdata, IO_WriteOnly);
	arg << QString("krfb");
	if (!d->call ("kded", "kinetd", "port(QString)", sdata, replyType, rdata))
		return; // nicer error here

	if (replyType != "int")
		return; // nicer error here

	QDataStream answer(rdata, IO_ReadOnly);
	answer >> portNum;
}

void Configuration::setKInetdPort(int p) {
	DCOPClient *d = KApplication::dcopClient();

	QByteArray sdata, rdata;
	QCString replyType;
	QDataStream arg(sdata, IO_WriteOnly);
	arg << QString("krfb") << p << 1;
	if (!d->call ("kded", "kinetd", "setPort(QString,int,int)", sdata, replyType, rdata))
		return; // nicer error here
}

void Configuration::removeInvitation(QValueList<Invitation>::iterator it) {
	invitationList.remove(it);
	save();
}

void Configuration::doKinetdConf() {
	setKInetdPort(preferredPortNum);

	if (allowUninvitedFlag) {
		setKInetdEnabled(true);
		getPortFromKInetd();
		return;
	}

	QDateTime lastExpiration;
	QValueList<Invitation>::iterator it = invitationList.begin();
	while (it != invitationList.end()) {
		Invitation &ix = (*it);
		QDateTime t = ix.expirationTime();
		if (t > lastExpiration)
			lastExpiration = t;
		it++;
	}
	if (lastExpiration.isNull() || (lastExpiration < QDateTime::currentDateTime())) {
		setKInetdEnabled(false);
		portNum = -1;
	}
	else {
		setKInetdEnabled(lastExpiration);
		getPortFromKInetd();
	}
}

void Configuration::loadFromKConfig() {

	KConfig c("krfbrc");
	allowUninvitedFlag = c.readBoolEntry("allowUninvited", false);
	askOnConnectFlag = c.readBoolEntry("confirmUninvitedConnection", true);
	allowDesktopControlFlag = c.readBoolEntry("allowDesktopControl", false);
	passwordString = c.readEntry("uninvitedPassword", "");
	preferredPortNum = c.readNumEntry("preferredPort", -1);

	unsigned int invNum = invitationList.size();
	invitationList.clear();
	c.setGroup("invitations");
	int num = c.readNumEntry("invitation_num", 0);
	for (int i = 0; i < num; i++)
		invitationList.push_back(Invitation(&c, i));

	invalidateOldInvitations();
	if (invNum != invitationList.size())
		emit invitationNumChanged(invitationList.size());
}

void Configuration::saveToKConfig() {

	KConfig c("krfbrc");
	c.writeEntry("confirmUninvitedConnection", askOnConnectFlag);
	c.writeEntry("allowDesktopControl", allowDesktopControlFlag);
	c.writeEntry("allowUninvited", allowUninvitedFlag);
	c.writeEntry("uninvitedPassword", passwordString);
	c.writeEntry("preferredPort", preferredPortNum);

	c.setGroup("invitations");
	int num = invitationList.count();
	c.writeEntry("invitation_num", num);
	int i = 0;
	while (i < num)
		invitationList[i++].save(&c, i);

}

void Configuration::saveToDialogs() {
	invalidateOldInvitations();
	QValueList<Invitation>::iterator it = invitationList.begin();
	while (it != invitationList.end()) {
		Invitation &inv = *(it++);
		if (!inv.getViewItem())
			inv.setViewItem(new KListViewItem(invMngDlg.listView,
				inv.creationTime().toString(Qt::LocalDate),
				inv.expirationTime().toString(Qt::LocalDate)));
	}

	invMngDlg.adjustSize();
}

void Configuration::save() {
	saveToKConfig();
	saveToDialogs();
	doKinetdConf();
}

void Configuration::update() {
	loadFromKConfig();
	saveToDialogs();
}

Invitation Configuration::createInvitation() {
	Invitation inv;
	invitationList.push_back(inv);
	return inv;
}

void Configuration::invalidateOldInvitations() {
	QValueList<Invitation>::iterator it = invitationList.begin();
	while (it != invitationList.end()) {
		if (!(*it).isValid())
			it = invitationList.remove(it);
		else
			it++;
	}
}

void Configuration::refreshTimeout() {
	unsigned int invNum = invitationList.size();
	loadFromKConfig();
	if (invNum != invitationList.size()) {
		saveToDialogs();
		emit invitationNumChanged(invitationList.size());
	}
}

///////// properties ///////////////////////////

krfb_mode Configuration::mode() const {
	return m_mode;
}

bool Configuration::askOnConnect() const {
	return askOnConnectFlag;
}

bool Configuration::allowDesktopControl() const {
	return allowDesktopControlFlag;
}

bool Configuration::allowUninvitedConnections() const {
	return allowUninvitedFlag;
}

QString Configuration::password() const {
	return passwordString;
}

QValueList<Invitation> &Configuration::invitations() {
	return invitationList;
}

void Configuration::setAllowUninvited(bool allowUninvited) {
	allowUninvitedFlag = allowUninvited;
}

void Configuration::setAskOnConnect(bool askOnConnect)
{
	askOnConnectFlag = askOnConnect;
}

void Configuration::setAllowDesktopControl(bool allowDesktopControl)
{
	allowDesktopControlFlag = allowDesktopControl;
}

void Configuration::setPassword(QString password)
{
	passwordString = password;
}

int Configuration::port() const
{
	if ((portNum < 5900) || (portNum >= 6000))
		return portNum;
	else
		return portNum - 5900;
}

// use p=-1 for defaults
void Configuration::setPreferredPort(int p)
{
	preferredPortNum = p;		
}

int Configuration::preferredPort() const
{
	return preferredPortNum;
}


// hostname is implemented in configuration_hostname.cpp
// QString Configuration::hostname()

////////////// invitation manage dialog //////////////////////////

void Configuration::showManageInvitationsDialog() {
        loadFromKConfig();
	saveToDialogs();
	invMngDlg.show();
}

void Configuration::invMngDlgClosed() {
	invMngDlg.hide();
}

void Configuration::invMngDlgDeleteOnePressed() {
	QValueList<Invitation>::iterator it = invitationList.begin();
	while (it != invitationList.end()) {
		Invitation &ix = (*it);
		KListViewItem *iv = ix.getViewItem();
		if (iv && iv->isSelected())
			it = invitationList.remove(it);
		else
			it++;
	}
	saveToKConfig();
	doKinetdConf();
	emit invitationNumChanged(invitationList.size());
}

void Configuration::invMngDlgDeleteAllPressed() {
	invitationList.clear();
	saveToKConfig();
	doKinetdConf();
	emit invitationNumChanged(invitationList.size());
}

////////////// invitation dialog //////////////////////////

void Configuration::showInvitationDialog() {
	invDlg.show();
}

void Configuration::invDlgClosed() {
	closeInvDlg();
	emit invitationFinished();
}

void Configuration::closeInvDlg() {
	saveToKConfig();
	invDlg.hide();
}

void Configuration::changeInvDlgNum(int newNum) {
	invDlg.manageInvitationButton->setText( i18n("Manage Invitations %1...").arg(newNum) );
}

////////////// personal invitation dialog //////////////////////////

void Configuration::showPersonalInvitationDialog() {
	loadFromKConfig();
	Invitation inv = createInvitation();
	save();
	emit invitationNumChanged(invitationList.size());
	
	invDlg.createInvitationButton->setEnabled(false);
	invMngDlg.newPersonalInvitationButton->setEnabled(false);

	persInvDlg.hostLabel->setText(QString("%1:%2").arg(hostname()).arg(port()));
	persInvDlg.passwordLabel->setText(inv.password());
	persInvDlg.expirationLabel->setText(
		inv.expirationTime().toString(Qt::LocalDate));
	persInvDlg.show();
}

void Configuration::persInvDlgClosed() {
	persInvDlg.hide();
	invDlg.createInvitationButton->setEnabled(true);
	invMngDlg.newPersonalInvitationButton->setEnabled(true);
}

////////////// invite email //////////////////////////

void Configuration::inviteEmail() {
	loadFromKConfig();
	Invitation inv = createInvitation();
	save();
	emit invitationNumChanged(invitationList.size());

	KApplication *app = KApplication::kApplication();
	app->invokeMailer(QString::null, QString::null, QString::null,
		i18n("Desktop Sharing (VNC) invitation"),
		i18n("You have been invited to a VNC session. To connect, start "
		     "a VNC client with the following parameters:\n\n"
		     "Host: %1:%2\n"
		     "Password: %3\n\n"
		     "For security reasons this invitation will expire at %4.")
			.arg(hostname())
			.arg(port())
			.arg(inv.password())
			.arg(KGlobal::locale()->formatDateTime(inv.expirationTime())));

}

#include "configuration.moc"
