
/***************************************************************************
                                kinetd.cpp
                              --------------
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

#include "kinetd.h"
#include <kservicetype.h>
#include <kservice.h>

PortListener::PortListener(KService::Ptr s) {

}


class KInetD : public KDEDModule {
	KInetD::KInetD(QCString &n) :
		KDEDModule(n)
	{
		portListeners.setAutoDelete(true);
		loadServiceList();
	}
	
	void KInetD::loadServiceList() 
	{
		portListeners.clear();

		KService::List kinetdModules = 
			KServiceType::offers("KInetDModule");
		for(KService::List::ConstIterator it = kinetdModules.begin(); 
		    it != kinetdModules.end(); 
		    it++) {
			KService::Ptr s = *it;
			portListeners.append(new PortListener(s));
		}
		
	}
}




extern "C" {
	KDEDModule *create_kinetd(QCString *name)
	{
		return new KInetD(name);
	}
}

