/***************************************************************************
                              configuration.cpp
                             -------------------
    begin                : Tue Dec 11 2001
    copyright            : (C) 2001-2003 by Tim Jansen
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
#include "kinetinterface.h"

#include <kglobal.h>
#include <klocale.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <ksockaddr.h>
#include <kactivelabel.h>

#include <qdatastream.h>
#include <dcopclient.h>
#include <dcopref.h>

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <krun.h>

/**
 * Note that this class is used and provides GUI in every mode:
 * - for the invitation dialogs
 * - for the kcontrol module
 * - for the running krfb instance
 */
Configuration::Configuration(krfb_mode mode) :
	m_mode(mode),
	invMngDlg(0, 0, true),
	invDlg(0, "InviteDialog"),
	persInvDlg(0, "PersonalInviteDialog"),
	portNum(-1),
	kinetdRef("kded", "kinetd")
{
	kinetdRef.setDCOPClient(KApplication::dcopClient());
	loadFromKConfig();
	saveToDialogs();
	doKinetdConf();

    connectDCOPSignal( 0, "KRFB::ConfigChanged", "KRFB_ConfigChanged()",
						"updateKConfig()", false );
	connect(invMngDlg.newPersonalInvitationButton, SIGNAL(clicked()),
		SLOT(showPersonalInvitationDialog()));
	connect(invMngDlg.newEmailInvitationButton, SIGNAL(clicked()), SLOT(inviteEmail()));
	connect(invMngDlg.deleteOneButton, SIGNAL(clicked()), SLOT(invMngDlgDeleteOnePressed()));
	connect(invMngDlg.deleteAllButton, SIGNAL(clicked()), SLOT(invMngDlgDeleteAllPressed()));
	invMngDlg.listView->setSelectionMode(QListView::Extended);
	invMngDlg.listView->setMinimumSize(QSize(400, 100)); // QTs size is much to small

	connect(&invDlg, SIGNAL(createInviteClicked()),
		SLOT(showPersonalInvitationDialog()));
	connect(&invDlg, SIGNAL(emailInviteClicked()),
		SLOT(inviteEmail()));
	connect(&invDlg, SIGNAL(manageInviteClicked()),
		SLOT(showManageInvitationsDialog()));
	connect(&invDlg, SIGNAL(configureClicked()),
		SLOT(showConfigurationModule()));
	connect(this, SIGNAL(invitationNumChanged(int)),
		&invDlg, SLOT(setInviteCount(int)));
	connect(this, SIGNAL(invitationNumChanged(int)),
		&invMngDlg, SLOT(listSizeChanged(int)));
        emit invitationNumChanged(invitationList.size());

	connect(&refreshTimer, SIGNAL(timeout()), SLOT(refreshTimeout()));
	refreshTimer.start(1000*60);
}

Configuration::~Configuration() {
        save();
}

void Configuration::updateKConfig()
{
    loadFromKConfig();
}

void Configuration::setKInetdEnabled(bool enabled) {
	kinetdRef.send("setEnabled", QString("krfb"), enabled);
	kinetdRef.send("setEnabled", QString("krfb_httpd"), enabled);
}

void Configuration::setKInetdEnabled(const QDateTime &date) {
	kinetdRef.send("setEnabled", QString("krfb"), date);
	kinetdRef.send("setEnabled", QString("krfb_httpd"), date);
}

void Configuration::setKInetdServiceRegistrationEnabled(bool enabled) {
	kinetdRef.send("setServiceRegistrationEnabled",
		       QString("krfb"), enabled);
	kinetdRef.send("setServiceRegistrationEnabled",
		       QString("krfb_httpd"), enabled);
}

void Configuration::getPortFromKInetd() {
	DCOPReply r = kinetdRef.call("port", QString("krfb"));
	if (!r.isValid())
		return; // nice error msg here?
	r.get(portNum);
}

void Configuration::setKInetdPort(int p) {
	DCOPReply r = kinetdRef.call("setPort",
				     QString("krfb"), p, 1);
	// nice error msg here?
}


void Configuration::removeInvitation(QValueList<Invitation>::iterator it) {
	invitationList.remove(it);
	save();
}

void Configuration::doKinetdConf() {
	setKInetdPort(preferredPortNum);

	if (allowUninvitedFlag) {
		setKInetdEnabled(true);
		setKInetdServiceRegistrationEnabled(enableSLPFlag);
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
		setKInetdServiceRegistrationEnabled(false);
		setKInetdEnabled(lastExpiration);
		getPortFromKInetd();
	}
}

void Configuration::loadFromKConfig() {

	KConfig c("krfbrc");
	allowUninvitedFlag = c.readBoolEntry("allowUninvited", false);
	enableSLPFlag = c.readBoolEntry("enableSLP", true);
	askOnConnectFlag = c.readBoolEntry("confirmUninvitedConnection", true);
	allowDesktopControlFlag = c.readBoolEntry("allowDesktopControl", false);
	preferredPortNum = c.readNumEntry("preferredPort", -1);
	disableBackgroundFlag = c.readBoolEntry("disableBackground", false);
	disableXShmFlag = c.readBoolEntry("disableXShm", false);
	if (c.hasKey("uninvitedPasswordCrypted"))
		passwordString = cryptStr(c.readEntry("uninvitedPasswordCrypted", ""));
	else
		passwordString = c.readEntry("uninvitedPassword", "");

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
	c.writeEntry("enableSLP", enableSLPFlag);
	c.writeEntry("preferredPort", preferredPortNum);
	c.writeEntry("disableBackground", disableBackgroundFlag);
	c.writeEntry("disableXShm", disableXShmFlag);
	c.writeEntry("uninvitedPasswordCrypted", cryptStr(passwordString));
	c.deleteEntry("uninvitedPassword");

	c.setGroup("invitations");
	int num = invitationList.count();
	c.writeEntry("invitation_num", num);
	int i = 0;
	while (i < num) {
		invitationList[i].save(&c, i);
		i++;
	}

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
	saveToDialogs();
	if (invNum != invitationList.size())
		emit invitationNumChanged(invitationList.size());
}

QString Configuration::hostname() const
{
  	KInetSocketAddress *a = KInetInterface::getPublicInetAddress();
	QString hostName;
	if (a) {
		hostName = a->nodeName();
		delete a;
	}
	else
		hostName = "localhost";
	return hostName;
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

bool Configuration::enableSLP() const {
	return enableSLPFlag;
}

QString Configuration::password() const {
	return passwordString;
}

QValueList<Invitation> &Configuration::invitations() {
	return invitationList;
}

bool Configuration::disableBackground() const {
	return disableBackgroundFlag;
}

bool Configuration::disableXShm() const {
	return disableXShmFlag;
}

void Configuration::setAllowUninvited(bool allowUninvited) {
	allowUninvitedFlag = allowUninvited;
}

void Configuration::setEnableSLP(bool e) {
	enableSLPFlag = e;
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

void Configuration::setDisableBackground(bool disable) {
	disableBackgroundFlag = disable;
}

void Configuration::setDisableXShm(bool disable) {
	disableXShmFlag = disable;
}

////////////// invitation manage dialog //////////////////////////

void Configuration::showManageInvitationsDialog() {
        loadFromKConfig();
	saveToDialogs();
	invMngDlg.exec();
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
	invDlg.exec();
	emit invitationFinished();
	saveToKConfig();
}

////////////// personal invitation dialog //////////////////////////

void Configuration::showPersonalInvitationDialog() {
	loadFromKConfig();
	Invitation inv = createInvitation();
	save();
	emit invitationNumChanged(invitationList.size());

	invDlg.enableInviteButton(false);
	invMngDlg.newPersonalInvitationButton->setEnabled(false);

	persInvDlg.setHost(hostname(), port());
	persInvDlg.setPassword(inv.password());
	persInvDlg.setExpiration(inv.expirationTime());

	persInvDlg.exec();
	invDlg.enableInviteButton(true);
	invMngDlg.newPersonalInvitationButton->setEnabled(true);
}

////////////// invite email //////////////////////////

void Configuration::inviteEmail() {
	int r = KMessageBox::warningContinueCancel(0,
	   i18n("When sending an invitation by email, note that everybody who reads this email "
		"will be able to connect to your computer for one hour, or until the first "
		"successful connection took place, whichever comes first. \n"
		"You should either encrypt the email or at least send it only in a "
		"secure network, but not over the Internet."),
						   i18n("Send Invitation via Email"),
						   KStdGuiItem::cont(),
						   "showEmailInvitationWarning");
	if (r == KMessageBox::Cancel)
		return;

	loadFromKConfig();
	Invitation inv = createInvitation();
	save();
	emit invitationNumChanged(invitationList.size());

	KApplication *app = KApplication::kApplication();
	app->invokeMailer(QString::null, QString::null, QString::null,
		i18n("Desktop Sharing (VNC) invitation"),
		i18n("You have been invited to a VNC session. If you have the KDE Remote "
                     "Desktop Connection installed, just click on the link below.\n\n"
		     "vnc://invitation:%1@%2:%3\n\n"
                     "Otherwise you can use any VNC client with the following parameters:\n\n"
		     "Host: %4:%5\n"
		     "Password: %6\n\n"
		     "Alternatively you can click on the link below to start the VNC session\n"
		     "within your web browser.\n"
		     "\n"
		     "	http://%7:%8/\n"
		     "\n"
		     "For security reasons this invitation will expire at %9.")
			.arg(inv.password())
			.arg(hostname())
			.arg(port())
			.arg(hostname())
			.arg(port())
			.arg(inv.password())
			.arg(hostname())
			.arg(5800) // determine with dcop ... later ...
			.arg(KGlobal::locale()->formatDateTime(inv.expirationTime())));
}

////////////// invoke kcontrol module //////////////////////////

void Configuration::showConfigurationModule() {
		KRun::run( "kcmshell kcmkrfb", KURL::List() );
}


#include "configuration.moc"
