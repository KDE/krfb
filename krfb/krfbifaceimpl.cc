/*
     Copyright 2002 Tim Jansen <tim@tjansen.de>
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krfbifaceimpl.h"
#include <krfbadaptor.h>

KRfbIfaceImpl::KRfbIfaceImpl(RFBController *c) :
	controller(c)
{
    (void)new KrfbAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/Krfb",this);
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
