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

Configuration::Configuration()
{
	loadFromKConfig();
	saveToDialog();

	
	portValidator = new QIntValidator(0, 65535, 
					  confDlg.displayNumberInput);
	confDlg.displayNumberInput->setValidator(portValidator);

	connect(confDlg.okButton, SIGNAL(clicked()), 
		SLOT(okPressed()));
	connect(confDlg.cancelButton, SIGNAL(clicked()), 
		SLOT(cancelPressed()));
	connect(confDlg.applyButton, SIGNAL(clicked()), 
		SLOT(applyPressed()));
}

Configuration::~Configuration() {
}

void Configuration::loadFromKConfig() {
	KConfig *config = KGlobal::config();
	askOnConnectFlag = config->readBoolEntry("askOnConnect", true);
	allowDesktopControlFlag = config->readBoolEntry("allowDesktopControl", 
							false);
	showMousePointerFlag = config->readBoolEntry("showMousePointer", 
						     true);
	passwordString = config->readEntry("password", "");
	portNumber = config->readNumEntry("port", 0);
}

void Configuration::loadFromDialog() {
	askOnConnectFlag = confDlg.askOnConnectCB->isChecked();
	allowDesktopControlFlag = confDlg.allowDesktopControlCB->isChecked();
	showMousePointerFlag = confDlg.showMousePointerCB->isChecked(); 
	passwordString = confDlg.passwordInput->text();
	int p = confDlg.displayNumberInput->text().toInt();
	if (p != portNumber) {
		portNumber = p;
		emit portChanged();
	}
}

void Configuration::saveToKConfig() {
	KConfig *config = KGlobal::config();
	config->writeEntry("askOnConnect", askOnConnectFlag);
	config->writeEntry("allowDesktopControl", allowDesktopControlFlag);
	config->writeEntry("showMousePointer", showMousePointerFlag);
	config->writeEntry("password", passwordString);
	config->writeEntry("port", portNumber);
}

void Configuration::saveToDialog() {
	confDlg.askOnConnectCB->setChecked(askOnConnectFlag);
	confDlg.allowDesktopControlCB->setChecked(allowDesktopControlFlag);
	confDlg.showMousePointerCB->setChecked(showMousePointerFlag); 
	confDlg.passwordInput->setText(passwordString);
	confDlg.displayNumberInput->setText(QString().setNum(portNumber));
}

bool Configuration::askOnConnect() const {
	return askOnConnectFlag;
}

bool Configuration::allowDesktopControl() const {
	return allowDesktopControlFlag;
}

bool Configuration::showMousePointer() const {
	return showMousePointerFlag;
}

QString Configuration::password() const {
	return passwordString;
}

int Configuration::port() const {
	return (portNumber < 100) ? (portNumber + 5900) : portNumber;
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
