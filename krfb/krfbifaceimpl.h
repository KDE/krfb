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
 
#ifndef __KRFB_IFACE_IMPL_H
#define __KRFB_IFACE_IMPL_H

#include <qobject.h>
#include "rfbcontroller.h"
#include "krfbiface.h"

class KRfbIfaceImpl : public QObject, public virtual krfbIface
{
	Q_OBJECT
private:
	RFBController *controller;
public:
	KRfbIfaceImpl(RFBController *c);
signals:
	void exitApp();

public:
	void exit();
	void setAllowDesktopControl(bool);
};
#endif
