/***************************************************************************
                              configuration.cpp
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

#include "configuration.h"

#include <kglobal.h>
#include <kapplication.h>

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>

const int INVITATION_DURATION = 60*60;

void ConfigurationDialog2::closeEvent(QCloseEvent *)
{
	emit closed();
}

void ManageInvitationsDialog2::closeEvent(QCloseEvent *)
{
	emit closed();
}

// TODO:
// invitation manage dialog
// dialog to add/delete invitation

Invitation::Invitation() :
	m_viewItem(0) {
	m_password = KApplication::randomString(4)+
		"-"+
		KApplication::randomString(4);
	m_creationTime = QDateTime::currentDateTime();
	m_expirationTime = QDateTime::currentDateTime().addSecs(INVITATION_DURATION);
}

Invitation::Invitation(const QString &tmpPassword,
	const QDateTime &expirationTime,
	const QDateTime &creationTime) :
	m_password(tmpPassword),
	m_creationTime(creationTime),
	m_expirationTime(expirationTime),
	m_viewItem(0) {
}

Invitation::Invitation(const Invitation &x) :
	m_password(x.m_password),
	m_creationTime(x.m_creationTime),
	m_expirationTime(x.m_expirationTime),
	m_viewItem(0) {
}

Invitation &Invitation::operator= (const Invitation&x) {
	m_password = x.m_password;
	m_creationTime = x.m_creationTime;
	m_expirationTime = x.m_expirationTime;
	if (m_viewItem)
		delete m_viewItem;
	m_viewItem = 0;
	return *this;
}

Invitation::Invitation(KConfig* config, int num) {
	m_password = config->readEntry(QString("password%1").arg(num), "");
	m_creationTime = config->readDateTimeEntry(QString("creation%1").arg(num));
	m_expirationTime = config->readDateTimeEntry(QString("expiration%1").arg(num));
	m_viewItem = 0;
}

Invitation::~Invitation() {
	if (m_viewItem)
		delete m_viewItem;
}

void Invitation::save(KConfig *config, int num) const {
	config->writeEntry(QString("password%1").arg(num), m_password);
	config->writeEntry(QString("creation%1").arg(num), m_creationTime);
	config->writeEntry(QString("expiration%1").arg(num), m_expirationTime);
}

QString Invitation::password() const {
	return m_password;
}

QDateTime Invitation::expirationTime() const {
	return m_expirationTime;
}

QDateTime Invitation::creationTime() const {
	return m_creationTime;
}

void Invitation::setViewItem(KListViewItem *i) {
	if (m_viewItem)
		delete m_viewItem;
	m_viewItem = i;
}

KListViewItem *Invitation::getViewItem() const{
	return m_viewItem;
}

Configuration::Configuration(krfb_mode mode) :
	m_mode(mode),
	oneConnectionFlag(false)
{
	loadFromKConfig();
	saveToDialogs();

	connect(confDlg.okButton, SIGNAL(clicked()), SLOT(configOkPressed()));
	connect(confDlg.cancelButton, SIGNAL(clicked()), SLOT(configCancelPressed()));
	connect(confDlg.applyButton, SIGNAL(clicked()), SLOT(configApplyPressed()));
	connect(&confDlg, SIGNAL(closed()), SLOT(configCancelPressed()));

	connect(confDlg.passwordInput, SIGNAL(textChanged(const QString&)), SLOT(configChanged()) );
	connect(confDlg.allowUninvitedCB, SIGNAL(clicked()), SLOT(configChanged()) );
	connect(confDlg.askOnConnectCB, SIGNAL(clicked()), SLOT(configChanged()) );
	connect(confDlg.allowDesktopControlCB, SIGNAL(clicked()), SLOT(configChanged()) );

	connect(invDlg.closeButton, SIGNAL(clicked()), SLOT(invDlgClosed()));
	connect(&invDlg, SIGNAL(closed()), SLOT(invDlgClosed()));
	connect(invDlg.newButton, SIGNAL(clicked()), SIGNAL(createInvitation()));
	connect(invDlg.deleteOneButton, SIGNAL(clicked()), SLOT(invDlgDeleteOnePressed()));
	connect(invDlg.deleteAllButton, SIGNAL(clicked()), SLOT(invDlgDeleteAllPressed()));
	invDlg.listView->setSelectionMode(QListView::Extended);
	invDlg.listView->setMinimumSize(QSize(400, 100)); // QTs size is much to small
}

Configuration::Configuration(bool oneConnection, bool askOnConnect,
			     bool allowDesktopControl, QString password) :
	m_mode(KRFB_STAND_ALONE_CMDARG),
	askOnConnectFlag(askOnConnect),
	allowDesktopControlFlag(allowDesktopControl),
	oneConnectionFlag(oneConnection),
	passwordString(password)
{
}

Configuration::~Configuration() {
}

void Configuration::configChanged() {
	confDlg.applyButton->setEnabled(true);
}

void Configuration::loadFromKConfig() {
	if (KRFB_STAND_ALONE_CMDARG == mode())
		return;
	KConfig c("krfbrc");
	allowUninvitedFlag = c.readBoolEntry("allowUninvited", true);
	askOnConnectFlag = c.readBoolEntry("confirmUninvitedConnection", true);
	allowDesktopControlFlag = c.readBoolEntry("allowDesktopControl", false);
	passwordString = c.readEntry("uninvitedPassword", "");

	invitationList.clear();
	c.setGroup("invitations");
	int num = c.readNumEntry("invitation_num", 0);
	for (int i = 0; i < num; i++)
		invitationList.push_back(Invitation(&c, i));

	confDlg.applyButton->setEnabled(false);
}

void Configuration::loadFromDialogs() {
	allowUninvitedFlag = confDlg.allowUninvitedCB->isChecked();
	askOnConnectFlag = confDlg.askOnConnectCB->isChecked();
	allowDesktopControlFlag = confDlg.allowDesktopControlCB->isChecked();
	QString newPassword = confDlg.passwordInput->text();
	if (passwordString != newPassword) {
		passwordString = newPassword;
		emit passwordChanged();
	}
}

void Configuration::saveToKConfig() {
	if (KRFB_STAND_ALONE_CMDARG == mode())
		return;

	KConfig c("krfbrc");
	c.writeEntry("confirmUninvitedConnection", askOnConnectFlag);
	c.writeEntry("allowDesktopControl", allowDesktopControlFlag);
	c.writeEntry("allowUninvited", allowUninvitedFlag);
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

	QValueList<Invitation>::iterator it = invitationList.begin();
	while (it != invitationList.end()) {
		Invitation &inv = *(it++);
		if (!inv.getViewItem())
			inv.setViewItem(new KListViewItem(invDlg.listView,
				inv.creationTime().toString(Qt::LocalDate),
				inv.expirationTime().toString(Qt::LocalDate)));
	}
}

void Configuration::reload() {
	loadFromKConfig();
	saveToDialogs();
}

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

void Configuration::showConfigDialog() {
	confDlg.show();
}

void Configuration::showManageInvitationsDialog() {
	updateDialogs();
	invDlg.show();
}

void Configuration::updateDialogs() {
	saveToDialogs();
	invDlg.adjustSize();
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

void Configuration::invDlgClosed() {
	invDlg.hide();
}

void Configuration::invDlgDeleteOnePressed() {
}

void Configuration::invDlgDeleteAllPressed() {
}


#include "configuration.moc"
