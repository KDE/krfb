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
	configuration(c)
{
}

void KRfbIfaceImpl::exit()
{
	emit exitApp();
}
void KRfbIfaceImpl::setAllowDesktopControl(bool b)
{
	configuration->setAllowDesktopControl(b);
	configuration->save();
}

#include "krfbifaceimpl.moc"
