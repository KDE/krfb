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

void ConfigurationDialog2::closeEvent(QCloseEvent *)
{ emit closed(); }
void ManageInvitationsDialog2::closeEvent(QCloseEvent *)
{ emit closed(); }
void InvitationDialog2::closeEvent(QCloseEvent *)
{ emit closed(); }
void PersonalInvitationDialog2::closeEvent(QCloseEvent *)
{ emit closed(); }

Configuration::Configuration(krfb_mode mode) :
	m_mode(mode),
	oneConnectionFlag(false),
	portNum(-1)
{
	loadFromKConfig();
	saveToDialogs();
	doKinetdConf();

	connect(confDlg.okButton, SIGNAL(clicked()), SLOT(configOkPressed()));
	connect(confDlg.cancelButton, SIGNAL(clicked()), SLOT(configCancelPressed()));
	connect(confDlg.applyButton, SIGNAL(clicked()), SLOT(configApplyPressed()));
	connect(&confDlg, SIGNAL(closed()), SLOT(configCancelPressed()));

	connect(confDlg.passwordInput, SIGNAL(textChanged(const QString&)), SLOT(configChanged()) );
	connect(confDlg.allowUninvitedCB, SIGNAL(clicked()), SLOT(configChanged()) );
	connect(confDlg.askOnConnectCB, SIGNAL(clicked()), SLOT(configChanged()) );
	connect(confDlg.allowDesktopControlCB, SIGNAL(clicked()), SLOT(configChanged()) );

	connect(invMngDlg.closeButton, SIGNAL(clicked()), SLOT(invMngDlgClosed()));
	connect(&invMngDlg, SIGNAL(closed()), SLOT(invMngDlgClosed()));
	connect(invMngDlg.newButton, SIGNAL(clicked()), SLOT(showInvitationDialog()));
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
	if (m_mode != KRFB_STAND_ALONE)
		invDlg.dontShowOnStartupButton->hide();

	connect(persInvDlg.closeButton, SIGNAL(clicked()), SLOT(persInvDlgClosed()));
	connect(&persInvDlg, SIGNAL(closed()), SLOT(persInvDlgClosed()));

	if ((m_mode == KRFB_STAND_ALONE) ||
	    (m_mode == KRFB_KINETD_MODE)) {
		connect(&expirationTimer, SIGNAL(timeout()), SLOT(invalidateOldInvitations()));
		expirationTimer.start(1000*60);
	}

}

Configuration::Configuration(bool oneConnection, bool askOnConnect,
			     bool allowDesktopControl, QString password) :
	m_mode(KRFB_STAND_ALONE_CMDARG),
	askOnConnectFlag(askOnConnect),
	allowDesktopControlFlag(allowDesktopControl),
	oneConnectionFlag(oneConnection),
	portNum(-1),
	passwordString(password)
{
}

Configuration::~Configuration() {
}

// special static method to determine daemon mode before constructor was invoked
bool Configuration::earlyDaemonMode() {
	KConfig c("krfbrc");
	return c.readBoolEntry("daemonMode", true);
}

void Configuration::setKInetd(bool enabled) {
	DCOPClient *d = KApplication::dcopClient();

	QByteArray sdata;
	QDataStream arg(sdata, IO_WriteOnly);
	arg << QString("krfb");
	arg << enabled;
	d->send ("kded", "kinetd", "setEnabled(QString,bool)", sdata);
}

void Configuration::setKInetd(const QDateTime &date) {
	DCOPClient *d = KApplication::dcopClient();

	QByteArray sdata;
	QDataStream arg(sdata, IO_WriteOnly);
	arg << QString("krfb");
	arg << date;
	d->send ("kded", "kinetd", "setEnabled(QString,QDateTime)", sdata);
}

void Configuration::setPortKInetd() {
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

void Configuration::removeInvitation(QValueList<Invitation>::iterator it) {
	invitationList.remove(it);
	doKinetdConf();
}

void Configuration::doKinetdConf() {
	if (!daemonFlag)
		return;

	if (allowUninvitedFlag) {
		setKInetd(true);
		setPortKInetd();
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
		setKInetd(false);
		portNum = -1;
	}
	else {
		setKInetd(lastExpiration);
		setPortKInetd();
	}
}

void Configuration::loadFromKConfig() {
	if (KRFB_STAND_ALONE_CMDARG == mode())
		return;
	KConfig c("krfbrc");
	daemonFlag = c.readBoolEntry("daemonMode", true);
	allowUninvitedFlag = c.readBoolEntry("allowUninvited", false);
	askOnConnectFlag = c.readBoolEntry("confirmUninvitedConnection", true);
	allowDesktopControlFlag = c.readBoolEntry("allowDesktopControl", false);
	showInvDlgOnStartupFlag = c.readBoolEntry("shovInvDlgOnStartup", false);
	passwordString = c.readEntry("uninvitedPassword", "");

	invitationList.clear();
	c.setGroup("invitations");
	int num = c.readNumEntry("invitation_num", 0);
	for (int i = 0; i < num; i++)
		invitationList.push_back(Invitation(&c, i));

	confDlg.applyButton->setEnabled(false);
	invalidateOldInvitations();

}

void Configuration::loadFromDialogs() {
	bool oldAllowUninvitedFlag = allowUninvitedFlag;
	allowUninvitedFlag = confDlg.allowUninvitedCB->isChecked();
	askOnConnectFlag = confDlg.askOnConnectCB->isChecked();
	allowDesktopControlFlag = confDlg.allowDesktopControlCB->isChecked();

	showInvDlgOnStartupFlag = !invDlg.dontShowOnStartupButton->isChecked();

	QString newPassword = confDlg.passwordInput->text();
	if (passwordString != newPassword) {
		passwordString = newPassword;
		emit passwordChanged();
	}

	if (oldAllowUninvitedFlag != allowUninvitedFlag)
		doKinetdConf();
}

void Configuration::saveToKConfig() {
	if (KRFB_STAND_ALONE_CMDARG == mode())
		return;

	KConfig c("krfbrc");
	c.writeEntry("daemonMode", daemonFlag);
	c.writeEntry("confirmUninvitedConnection", askOnConnectFlag);
	c.writeEntry("allowDesktopControl", allowDesktopControlFlag);
	c.writeEntry("allowUninvited", allowUninvitedFlag);
	c.writeEntry("shovInvDlgOnStartup", showInvDlgOnStartupFlag);
	c.writeEntry("uninvitedPassword", passwordString);

	c.setGroup("invitations");
	int num = invitationList.count();
	c.writeEntry("invitation_num", num);
	int i = 0;
	while (i < num)
		invitationList[i++].save(&c, i);

	confDlg.applyButton->setEnabled(false);
}

void Configuration::saveToDialogs() {
	confDlg.allowUninvitedCB->setChecked(allowUninvitedFlag);
	confDlg.askOnConnectCB->setChecked(askOnConnectFlag);
	confDlg.allowDesktopControlCB->setChecked(allowDesktopControlFlag);
	confDlg.passwordInput->setText(passwordString);

	invDlg.dontShowOnStartupButton->setChecked(!showInvDlgOnStartupFlag);

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

void Configuration::reload() {
	loadFromKConfig();
	saveToDialogs();
}

Invitation Configuration::createInvitation() {
	Invitation inv;
	invitationList.push_back(inv);
	emit passwordChanged();
	return inv;
}

void Configuration::invalidateOldInvitations() {
	bool changed = false;
	QValueList<Invitation>::iterator it = invitationList.begin();
	while (it != invitationList.end()) {
		if (!(*it).isValid()) {
			it = invitationList.remove(it);
			changed = true;
		}
		else
			it++;
	}
	if (changed)
		emit passwordChanged();
}

///////// properties ///////////////////////////

krfb_mode Configuration::mode() const {
	return m_mode;
}

bool Configuration::oneConnection() const {
	return oneConnectionFlag;
}

bool Configuration::askOnConnect() const {
	return askOnConnectFlag;
}

bool Configuration::allowDesktopControl() const {
	return allowDesktopControlFlag;
}

bool Configuration::allowUninvitedConnects() const {
	return allowUninvitedFlag;
}

bool Configuration::showInvitationDialogOnStartup() const {
	return showInvDlgOnStartupFlag;
}

bool Configuration::daemonMode() const {
	return daemonFlag;
}

QString Configuration::password() const {
	return passwordString;
}

QValueList<Invitation> &Configuration::invitations() {
	return invitationList;
}

void Configuration::setOnceConnection(bool oneConnection)
{
	oneConnectionFlag = oneConnection;
	saveToKConfig();
	saveToDialogs();
}

void Configuration::setAskOnConnect(bool askOnConnect)
{
	askOnConnectFlag = askOnConnect;
	saveToKConfig();
	saveToDialogs();
}

void Configuration::setAllowDesktopControl(bool allowDesktopControl)
{
	allowDesktopControlFlag = allowDesktopControl;
	saveToKConfig();
	saveToDialogs();
}

void Configuration::setPassword(QString password)
{
	passwordString = password;
	emit passwordChanged();
	saveToKConfig();
	saveToDialogs();
}

int Configuration::port() const
{
	if ((portNum < 5900) || (portNum >= 6000))
		return portNum;
	else
		return portNum - 5900;
}

void Configuration::setPort(int p) {
	portNum = p;
}

// hostname is implemented in configuration_hostname.cpp
// QString Configuration::hostname()

////////////// config dialog //////////////////////////

void Configuration::showConfigDialog() {
	if (m_mode == KRFB_KINETD_MODE) {
		KProcess p;
		p << "kcmshell" << "kcmkrfb";
		p.start(KProcess::DontCare);
	}
	else {
		loadFromKConfig();
		saveToDialogs();
		confDlg.show();
	}
}

void Configuration::configOkPressed() {
	loadFromDialogs();
	saveToKConfig();
	confDlg.hide();
}

void Configuration::configCancelPressed() {
	saveToDialogs();
	confDlg.hide();
}

void Configuration::configApplyPressed() {
	loadFromDialogs();
	saveToKConfig();
}

void Configuration::configChanged() {
	confDlg.applyButton->setEnabled(true);
}

////////////// invitation manage dialog //////////////////////////

void Configuration::showManageInvitationsDialog() {
	saveToDialogs();
	invMngDlg.show();
}

void Configuration::invMngDlgClosed() {
	invMngDlg.hide();
	saveToKConfig();
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
	emit passwordChanged();
	doKinetdConf();
}

void Configuration::invMngDlgDeleteAllPressed() {
	invitationList.clear();
	saveToKConfig();
	emit passwordChanged();
	doKinetdConf();
}

////////////// invitation dialog //////////////////////////

void Configuration::showInvitationDialog() {
	invMngDlg.newButton->setEnabled(false);
	invDlg.show();
}

void Configuration::invDlgClosed() {
	closeInvDlg();
	emit invitationFinished();
}

void Configuration::closeInvDlg() {
	loadFromDialogs();
	saveToKConfig();
	invDlg.hide();
	invMngDlg.newButton->setEnabled(true);
}


////////////// personal invitation dialog //////////////////////////

void Configuration::showPersonalInvitationDialog() {
	closeInvDlg();

	loadFromKConfig();
	Invitation inv = createInvitation();
	saveToDialogs();
	saveToKConfig();
	doKinetdConf();
	
	invDlg.createInvitationButton->setEnabled(false);
	persInvDlg.hostLabel->setText(QString("%1:%2").arg(hostname()).arg(port()));
	persInvDlg.passwordLabel->setText(inv.password());
	persInvDlg.expirationLabel->setText(
		inv.expirationTime().toString(Qt::LocalDate));
	persInvDlg.show();
}

void Configuration::persInvDlgClosed() {
	persInvDlg.hide();
	invDlg.createInvitationButton->setEnabled(true);
	emit invitationFinished();
}

////////////// invite email //////////////////////////

void Configuration::inviteEmail() {
	closeInvDlg();
	loadFromKConfig();
	Invitation inv = createInvitation();
	saveToDialogs();
	saveToKConfig();
	doKinetdConf();

	KApplication *app = KApplication::kApplication();
	app->invokeMailer(QString::null, QString::null, QString::null,
		i18n("Desktop Sharing (VNC) invitation"),
		i18n("You have been invited to a VNC session. To connect, start "
		     "a VNC client with the following parameters:\n\n"
		     "Host: %1:%2\n"
		     "Password: %3\n\n"
		     "For security reasons this invitation will expire on %4.")
			.arg(hostname())
			.arg(port())
			.arg(inv.password())
			.arg(inv.expirationTime().toString(Qt::LocalDate)));

	emit invitationFinished();
}

#include "configuration.moc"
