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

#include <kconfig.h>
#include <kglobal.h>

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>

void ConfigurationDialog2::closeEvent(QCloseEvent *)
{
	emit closed();
}

Configuration::Configuration() :
	preconfiguredFlag(false),
	oneConnectionFlag(false)
{
	loadFromKConfig();
	saveToDialog();

	connect(confDlg.okButton, SIGNAL(clicked()),
		SLOT(okPressed()));
	connect(confDlg.cancelButton, SIGNAL(clicked()),
		SLOT(cancelPressed()));
	connect(confDlg.applyButton, SIGNAL(clicked()),
		SLOT(applyPressed()));
	connect(&confDlg, SIGNAL(closed()), SLOT(cancelPressed()));
}

Configuration::Configuration(bool oneConnection, bool askOnConnect,
			     bool allowDesktopControl, QString password) :
	preconfiguredFlag(true),
	askOnConnectFlag(askOnConnect),
	allowDesktopControlFlag(allowDesktopControl),
	oneConnectionFlag(oneConnection),
	passwordString(password)
{
}


Configuration::~Configuration() {
}

void Configuration::loadFromKConfig() {
	if (preconfigured())
		return;
	KConfig *config = KGlobal::config();
	askOnConnectFlag = config->readBoolEntry("askOnConnect", true);
	allowDesktopControlFlag = config->readBoolEntry("allowDesktopControl",
							false);
	passwordString = config->readEntry("password", "");
}

void Configuration::loadFromDialog() {
	askOnConnectFlag = confDlg.askOnConnectCB->isChecked();
	allowDesktopControlFlag = confDlg.allowDesktopControlCB->isChecked();
	QString newPassword = confDlg.passwordInput->text();
	if (passwordString != newPassword) {
		passwordString = newPassword;
		emit passwordChanged();
	}
}

void Configuration::saveToKConfig() {
	if (preconfigured())
		return;
	KConfig *config = KGlobal::config();
	config->writeEntry("askOnConnect", askOnConnectFlag);
	config->writeEntry("allowDesktopControl", allowDesktopControlFlag);
	config->writeEntry("password", passwordString);
}

void Configuration::saveToDialog() {
	confDlg.askOnConnectCB->setChecked(askOnConnectFlag);
	confDlg.allowDesktopControlCB->setChecked(allowDesktopControlFlag);
	confDlg.passwordInput->setText(passwordString);
}

void Configuration::reload() {
	loadFromKConfig();
	saveToDialog();
}

bool Configuration::preconfigured() const {
	return preconfiguredFlag;
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

void Configuration::setOnceConnection(bool oneConnection)
{
        oneConnectionFlag = oneConnection;
        saveToKConfig();
        saveToDialog();
}

void Configuration::setAskOnConnect(bool askOnConnect)
{
        askOnConnectFlag = askOnConnect;
        saveToKConfig();
        saveToDialog();
}

void Configuration::setAllowDesktopControl(bool allowDesktopControl)
{
        allowDesktopControlFlag = allowDesktopControl;
        saveToKConfig();
        saveToDialog();
}

void Configuration::setPassword(QString password)
{
        passwordString = password;
  	emit passwordChanged();
        saveToKConfig();
        saveToDialog();
}

void Configuration::showDialog() {
	confDlg.show();
}

void Configuration::okPressed() {
	loadFromDialog();
	saveToKConfig();
	confDlg.hide();
}

void Configuration::cancelPressed() {
	saveToDialog();
	confDlg.hide();
}

void Configuration::applyPressed() {
	loadFromDialog();
	saveToKConfig();
}

#include "configuration.moc"
