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
#ifndef __KRFB_IFACE_H
#define __KRFB_IFACE_H

#include <dcopobject.h>

class krfbIface : virtual public DCOPObject
{
        K_DCOP
k_dcop:

	/**
	 * Quits krfb, connected clients will be disconnected.
	 */
	virtual void exit() = 0;

	/**
	 * If this feature is activated krfb allows the connecting client to
	 * control the desktop (pointer & keyboard).
	 * @return a true to activate desktop control
	 */
	virtual void setAllowDesktopControl(bool a) = 0;
	
};
#endif
