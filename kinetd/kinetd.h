
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

#include <kdedmodule.h>
#include <kservice.h>
#include <ksock.h>
#include <kprocess.h>
#include <qstringlist.h>
#include <qstring.h>

class PortListener : public QObject {
	Q_OBJECT
private:
	bool valid;
	QString serviceName;
	int port, portBase, autoPortRange;
	bool multiInstance;
	QCString execPath;
	QString argument;
	bool enabled;

	KServerSocket *socket;
	KProcess process;

	KConfig *config;

	void loadConfig(KService::Ptr s);
public:
	PortListener(KService::Ptr s);
	~PortListener();

	bool isValid();
	QString name();
	void setEnabled(bool enabled);
	bool isEnabled();

private slots:
	void accepted(KSocket*);
};

class KInetD : public KDEDModule {
	Q_OBJECT
	K_DCOP

k_dcop:
	/**
	 * Returns a list of all registered services in KInetd.
	 * To add a service you need to add a .desktop file with
	 * the servicetype "KInetDModule" into the services director
	 * (see kinetdmodule.desktop in servicetypes dir).
	 * @return a list with the names of all services
	 */
	QStringList services();

	/**
	 * Returns true if the service exists and is available.
	 * @param service name of a service as specified in its .desktop file
	 * @return true if a service with the given name exists and is enabled
	 */
	bool isEnabled(QString service);

	/**
	 * Enables or disabled the given service. Ignored if the given service
	 * does not exist.
	 * @param service name of a service as specified in its .desktop file
	 * @param enable true to enable, false to disable.
	 */
	void setEnabled(QString service, bool enable);

 private:
	QPtrList<PortListener> portListeners;

 public:
	KInetD(QCString &n);
	void loadServiceList();
	PortListener *getListenerByName(QString name);
};


#endif
