
/***************************************************************************
                                  kinetd.h
                                ------------
    begin                : Mon Feb 11 2002
    copyright            : (C) 2002 by Tim Jansen
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

#ifndef _KINETD_H_
#define _KINETD_H_

#include <kservice.h>

class PortListener : public DCOPObject {
	K_DCOP
private:
	QString serviceName;
	int port;
	QCString execPath;
	
 public:
	PortListener(KService::Ptr s);

	
	
	

 k_dcop:
	
	
}


#endif
