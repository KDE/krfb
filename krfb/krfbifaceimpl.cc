/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krfbifaceimpl.h"

KRfbIfaceImpl::KRfbIfaceImpl(Configuration *c) :
	DCOPObject("krfbIface"),
	configuration(c),
	portNum(0)
{	
}

void KRfbIfaceImpl::disconnect()
{
	emit connectionClosed();
}

/*
void KRfbIfaceImpl::setWindowID(int)
{
}
*/

void KRfbIfaceImpl::exit()
{
	emit exitApp();
}
bool KRfbIfaceImpl::oneConnection()
{
	return configuration->oneConnection();
}
void KRfbIfaceImpl::setOneConnection(bool b)
{
	configuration->setOnceConnection(b);
}
bool KRfbIfaceImpl::askOnConnect()
{
	return configuration->askOnConnect();
}
void KRfbIfaceImpl::setAskOnConnect(bool b)
{
	configuration->setAskOnConnect(b);
}
bool KRfbIfaceImpl::allowDesktopControl()
{
	return configuration->allowDesktopControl();
}
void KRfbIfaceImpl::setAllowDesktopControl(bool b)
{
	configuration->setAllowDesktopControl(b);
}
void KRfbIfaceImpl::setPassword(QString password)
{
	configuration->setPassword(password);
}
int KRfbIfaceImpl::port()
{
	return portNum;
}

/*
 * Note that setPort() is not a DCOP function, but a slot that's
 * connected to RFBController to get the autoprobed port. 
 */
void KRfbIfaceImpl::setPort(int p)
{
	portNum = p;
}


