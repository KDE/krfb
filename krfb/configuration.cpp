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

Configuration::Configuration(KConfig *config) :
	config(config)
{
	askOnConnect = config->readBoolEntry("askOnConnect", true);
	allowDesktopControl = config->readBoolEntry("allowDesktopControl", 
						    false);
	showMousePointer = config->readBoolEntry("showMousePointer", 
						 true);
	passwordString = config->readEntry("password", "");
	portNumber = config->readNumEntry("port", 0);
}

Configuration::~Configuration() {
}

bool Configuration::askOnConnect() {
	return askOnConnectFlag;
}

bool Configuration::allowDesktopControl() {
	return allowDesktopControlFlag;
}

bool Configuration::showMousePointer() {
	return showMousePointerFlag;
}

QString Configuration::password() {
	return passwordString;
}

int Configuration::port() {
	return portNumber;
}

void Configuration::showDialog() {
	confDlg.show();
}


#endif
