
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
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <knotifyclient.h>
#include <ksockaddr.h>
#include <kextsock.h>
#include <klocale.h>

PortListener::PortListener(KService::Ptr s)
{
	loadConfig(s);

	if (enabled)
		acquirePort();
	else
		portNum = -1;
}

void PortListener::acquirePort() {
	portNum = portBase;
	socket = new KServerSocket(portNum, false);
	while (!socket->bindAndListen()) {
		portNum++;
		if (portNum >= (portBase+autoPortRange)) {
			kdDebug() << "Kinetd cannot load service "<<serviceName
				  <<": unable to get port" << endl;
			portNum = -1;
			enabled = false;
			delete socket;
			socket = 0;
			return;
		}
		delete socket;
		socket = new KServerSocket(portNum, false);
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

	execPath = s->exec().utf8();
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
	QDateTime nullTime;
	expirationTime = config->readDateTimeEntry("enabled_expiration_"+serviceName, &nullTime);
	if ((!expirationTime.isNull()) && (expirationTime < QDateTime::currentDateTime()))
		enabled = false;
}

void PortListener::accepted(KSocket *sock) {
	QString host, port;
	KSocketAddress *ksa = KExtendedSocket::peerAddress(sock->socket());
	KExtendedSocket::resolve(ksa, host, port);
	KNotifyClient::event("IncomingConnection",
		i18n("connection from %1").arg(host));
	delete ksa;

	if ((!enabled) ||
	   ((!multiInstance) && process.isRunning())) {
		delete sock;
		return;
	}

	process.clearArguments();
	process << execPath << argument << QString::number(sock->socket());
	if (!process.start(KProcess::DontCare)) {
		KNotifyClient::event("ProcessFailed",
			i18n("Call \"%1 %2 %3\" failed").arg(execPath)
				.arg(argument)
				.arg(sock->socket()));
	}

	delete sock;
}

bool PortListener::isValid() {
	return valid;
}

bool PortListener::isEnabled() {
	return enabled;
}

int PortListener::port() {
	return portNum;
}

void PortListener::setEnabled(bool e) {
	setEnabledInternal(e, QDateTime());
}

void PortListener::setEnabledInternal(bool e, const QDateTime &ex) {
	expirationTime = ex;
	if (e) {
		if (portNum < 0)
			acquirePort();
		if (portNum < 0) {
			enabled = false;
			return;
		}
	}
	else {
		portNum = -1;
		if (socket)
			delete socket;
		socket = 0;
	}

	enabled = e;

	if (!config)
		return;
	config->setGroup("ListenerConfig");
	config->writeEntry("enabled_" + serviceName, enabled);
	config->writeEntry("enabled_expiration_"+serviceName, ex);
	config->sync();
}

void PortListener::setEnabled(const QDateTime &ex) {
	setEnabledInternal(true, ex);
}

QDateTime PortListener::expiration() {
	return expirationTime;
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
	connect(&expirationTimer, SIGNAL(timeout()), SLOT(setTimer()));
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

	setTimer();
}

void KInetD::setTimer() {
	QDateTime nextEx = getNextExpirationTime(); // disables expired portlistener!
	if (!nextEx.isNull())
		expirationTimer.start(QDateTime::currentDateTime().secsTo(nextEx)*1000 + 30000,
			false);
	else
		expirationTimer.stop();
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

// gets next expiration timer, SIDEEFFECT: disables expired portlisteners while doing this
QDateTime KInetD::getNextExpirationTime()
{
	PortListener *pl = portListeners.first();
	QDateTime d;
	while (pl) {
		QDateTime d2 = pl->expiration();
		if (!d2.isNull()) {
			if (d2 < QDateTime::currentDateTime())
				pl->setEnabled(false);
			else if (d.isNull() || (d2 < d))
				d = d2;
		}
		pl = portListeners.next();
	}
	return d;
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

int KInetD::port(QString service)
{
	PortListener *pl = getListenerByName(service);
	if (!pl)
		return -1;

	return pl->port();
}

bool KInetD::isInstalled(QString service)
{
	PortListener *pl = getListenerByName(service);
	return (pl != 0);
}

void KInetD::setEnabled(QString service, bool enable)
{
	PortListener *pl = getListenerByName(service);
	if (!pl)
		return;

	pl->setEnabled(enable);
	setTimer();
}

void KInetD::setEnabled(QString service, QDateTime expiration)
{
	PortListener *pl = getListenerByName(service);
	if (!pl)
		return;

	pl->setEnabled(expiration);
	setTimer();
}


extern "C" {
	KDEDModule *create_kinetd(QCString &name)
	{
		return new KInetD(name);
	}
}

