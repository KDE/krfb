
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

/*
 * TODOs: 
 * - setup servicetype
 * - override configuration in KDEHOME with a KConfig
 * - set listening ip address
 * - implement autoPortRange
 */

#include "kinetd.h"
#include <kservicetype.h>
#include <kservice.h>
#include <kdebug.h>

PortListener::PortListener(KService::Ptr s) :
	valid(true),
	autoPortRange(0),
	multiInstance(false),
	enabled(true)
{

	QVariant vport, vautoport, venabled, vargument, vmultiInstance;
	serviceName = s->name();
	execPath = s->exec();
	vport = s->property("X-KDE-KINETD-port");
	vautoport = s->property("X-KDE-KINETD-autoPortRange");
	venabled = s->property("X-KDE-KINETD-enabled");
        vargument = s->property("X-KDE-KINETD-argument");
        vmultiInstance = s->property("X-KDE-KINETD-multiInstance");
	
	if (!vport.isValid())
		valid = false;
	else
		port = vport.toInt();
	if (vautoport.isValid())
		autoPortRange = vautoport.toInt();
	if (venabled.isValid())
		enabled = venabled.toBool();
	if (vargument.isValid())
		argument = vargument.toString();
	if (vmultiInstance.isValid())
		multiInstance = vmultiInstance.toBool();

	socket = new KServerSocket(port, false);
	connect(socket, SIGNAL(accepted(KSocket*)), 
		SLOT(accepted(KSocket*)));

	process.setExecutable(execPath);

	if (!socket->bindAndListen()) {
		// TODO: do something, implement autoport
		kdDebug() << "bind failed" <<endl;
	}
}

void PortListener::accepted(KSocket *sock) {
	kdDebug() << "got connection" << endl;

	if ((!enabled) || 
	    ((!multiInstance) && process.isRunning())) {
		delete sock;
		return;
	}

	process.clearArguments();
	// TODO: call now:
	// process << argument << sock->socket();
	// process.start(DontCare);
	delete sock;
}

bool PortListener::isValid() {
	return valid;
}

PortListener::~PortListener() {
	if (socket)
		delete socket;
}


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


extern "C" {
	KDEDModule *create_kinetd(QCString &name)
	{
		return new KInetD(name);
	}
}

