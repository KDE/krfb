/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krfbifaceimpl.h"

KRfbIfaceImpl::KRfbIfaceImpl(RFBController *c) :
	DCOPObject("krfbIface"),
	controller(c)
{
}

void KRfbIfaceImpl::exit()
{
	emit exitApp();
}
void KRfbIfaceImpl::setAllowDesktopControl(bool b)
{
	controller->enableDesktopControl(b);
}

#include "krfbifaceimpl.moc"
