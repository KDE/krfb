
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
 * - get notified of changes in services
 * - replace debug messages with knotify
 */

#include "kinetd.h"
#include <kservicetype.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>

PortListener::PortListener(KService::Ptr s)
{
	loadConfig(s);

	process.setExecutable(execPath);

	port = portBase;
	socket = new KServerSocket(port, false);
	while (!socket->bindAndListen()) {
		port++;
		if (port >= (portBase+autoPortRange)) {
			kdDebug() << "Kinetd cannot load service "<<serviceName
				  <<": unable to get port" << endl;
			valid = false;
			return;
		}
		delete socket;
		socket = new KServerSocket(port, false);
	}

	connect(socket, SIGNAL(accepted(KSocket*)),
		SLOT(accepted(KSocket*)));
}

void PortListener::loadConfig(KService::Ptr s) {
	valid = true;
	autoPortRange = 0;
	enabled = true;
	argument = QString::null;
	multiInstance = false;

	QVariant vid, vport, vautoport, venabled, vargument, vmultiInstance;

	execPath = s->exec();
	vid = s->property("X-KDE-KINETD-id");
	vport = s->property("X-KDE-KINETD-port");
	vautoport = s->property("X-KDE-KINETD-autoPortRange");
	venabled = s->property("X-KDE-KINETD-enabled");
	vargument = s->property("X-KDE-KINETD-argument");
	vmultiInstance = s->property("X-KDE-KINETD-multiInstance");

	if (!vid.isValid()) {
		kdDebug() << "Kinetd cannot load service "<<serviceName
			  <<": no id set" << endl;
		valid = false;
		return;
	}

	if (!vport.isValid()) {
		kdDebug() << "Kinetd cannot load service "<<serviceName
			  <<": invalid port" << endl;
		valid = false;
		return;
	}

	serviceName = vid.toString();
	portBase = vport.toInt();
	if (vautoport.isValid())
		autoPortRange = vautoport.toInt();
	if (venabled.isValid())
		enabled = venabled.toBool();
	if (vargument.isValid())
		argument = vargument.toString();
	if (vmultiInstance.isValid())
		multiInstance = vmultiInstance.toBool();

	config = new KConfig("kinetdrc");
	config->setGroup("ListenerConfig");
	enabled = config->readBoolEntry("enabled_" + serviceName, enabled);
}

void PortListener::accepted(KSocket *sock) {
	kdDebug() << "got connection" << endl;

	if ((!enabled) ||
	   ((!multiInstance) && process.isRunning())) {
		delete sock;
		return;
	}

	process.clearArguments();
	process << argument << sock->socket();
	if (!process.start(KProcess::DontCare)) {
		kdDebug() << "kinetd: Calling process failed" << endl;
	}
	delete sock;
}

bool PortListener::isValid() {
	return valid;
}

bool PortListener::isEnabled() {
	return enabled;
}

void PortListener::setEnabled(bool e) {
	if (e == enabled)
		return;
	enabled = e;
	if (!config)
		return;
	config->setGroup("ListenerConfig");
	config->writeEntry("enabled_" + serviceName, enabled);
	config->sync();
}

QString PortListener::name() {
	return serviceName;
}

PortListener::~PortListener() {
	if (socket)
		delete socket;
	if (config)
		delete config;
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
		PortListener *pl = new PortListener(s);
		if (pl->isValid())
			portListeners.append(pl);
	}
}

PortListener *KInetD::getListenerByName(QString name)
{
	PortListener *pl = portListeners.first();
	while (pl) {
		if (pl->name() == name)
			return pl;
		pl = portListeners.next();
	}
	return pl;
}

QStringList KInetD::services()
{
	QStringList list;
	PortListener *pl = portListeners.first();
	while (pl) {
		list.append(pl->name());
		pl = portListeners.next();
	}
	return list;
}

bool KInetD::isEnabled(QString service)
{
	PortListener *pl = getListenerByName(service);
	if (!pl)
		return false;

	return pl->isEnabled();
}

void KInetD::setEnabled(QString service, bool enable)
{
	PortListener *pl = getListenerByName(service);
	if (!pl)
		return;

	pl->setEnabled(enable);
}


extern "C" {
	KDEDModule *create_kinetd(QCString &name)
	{
		return new KInetD(name);
	}
}

