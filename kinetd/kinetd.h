
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
#include <qdatetime.h>
#include <qtimer.h>
#include <dnssd/publicservice.h>

#include "kserviceregistry.h"

class PortListener : public QObject {
	Q_OBJECT
private:
	bool m_valid;
	QString m_serviceName;
	QString m_serviceURL, m_serviceAttributes;
	QStringList m_registeredServiceURLs;
	QString m_dnssdName, m_dnssdType;
	QMap<QString,QString> m_dnssdData;
	int m_serviceLifetime;
	int m_port;
	int m_portBase, m_autoPortRange;
	int m_defaultPortBase, m_defaultAutoPortRange;
	bool m_multiInstance;
	QString m_execPath;
	QString m_argument;
	bool m_enabled;
	bool m_serviceRegistered, m_registerService;
	bool m_dnssdRegister, m_dnssdRegistered;
	QDateTime m_expirationTime;
	QDateTime m_slpLifetimeEnd;
	QString m_uuid;

	KServerSocket *m_socket;
	KProcess m_process;

	KConfig *m_config;
	KServiceRegistry *m_srvreg;
	DNSSD::PublicService *m_dnssdreg;

	void freePort();
	void loadConfig(KService::Ptr s);
	void setEnabledInternal(bool e, const QDateTime &ex);
	void dnssdRegister(bool enabled);
	void setServiceRegistrationEnabledInternal(bool enabled);

public:
	PortListener(KService::Ptr s, KConfig *c, KServiceRegistry *srvreg);
	~PortListener();

	bool acquirePort();
	bool isValid();
	QString name();
	void setEnabled(bool enabled);
	void setEnabled(const QDateTime &expiration);
	void setServiceRegistrationEnabled(bool enabled);
	bool isServiceRegistrationEnabled();
	QDateTime expiration();
	QDateTime serviceLifetimeEnd();
	bool isEnabled();
	int port();
	QStringList processServiceTemplate(const QString &a);
	bool setPort(int port = -1, int autoProbeRange = 1);
	void refreshRegistration();

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

	/**
	 * Enables the given service until the given time. Ignored if the given
	 * service does not exist.
	 * @param service name of a service as specified in its .desktop file
	 * @param expiration the time the service will be disabled at
	 */
	void setEnabled(QString service, QDateTime expiration);

	/**
	 * Returns the port of the service, or -1 if not listening.
	 * @param service name of a service as specified in its .desktop file
	 * @return the port or -1 if no port used or service does not exist
	 */
	int port(QString service);

	/**
	 * Sets the port of the service, and possibly a range of ports to try.
	 * It will return true if a port could be found. If it didnt find one but is
	 * enabled it will start a timer that probes that port every 30s.
	 * @param service name of a service as specified in its .desktop file
	 * @param port the first port number to try or -1 to restore defaults
	 * @param autoPortRange the number of ports to try
	 * @return true if a port could be found or service is disabled, false 
	 *          otherwise. 
	 */
	bool setPort(QString service, int port = -1, int autoPortRange = 1);

	/**
	 * Tests whether the given service is installed..
	 * @param service name of a service as specified in its .desktop file
	 * @return true if installed, false otherwise
	 */
	bool isInstalled(QString service);

	/**
	 * Enables or disables the SLP registration. Ignored if the service does
	 * not have a service URL. If the service is disabled the service will
	 * registered as soon as it is enabled.
	 * @param service name of a service as specified in its .desktop file
	 * @param enable true to enable, false to disable.
	 */
	void setServiceRegistrationEnabled(QString service, bool enabled);

	/**
	 * Returns true if service registration for the given service is enabled.
	 * Note that this does not mean that the service is currently registered,
	 * because the service may be disabled.
	 * @param service name of a service as specified in its .desktop file
	 * @return true if service registration is enabled
	 */
	bool isServiceRegistrationEnabled(QString service);

 private:
	QDateTime getNextExpirationTime();
	void setPortRetryTimer(bool retry);
	void setReregistrationTimer();

	KConfig *m_config;
	KServiceRegistry *m_srvreg;
	QPtrList<PortListener> m_portListeners;
	QTimer m_expirationTimer;
	QTimer m_portRetryTimer;
	QTimer m_reregistrationTimer;

 private slots:
	void setExpirationTimer();
	void expirationTimer();
	void portRetryTimer();
	void reregistrationTimer();

 public:
	KInetD(QCString &n);
	virtual ~KInetD();
	void loadServiceList();
	PortListener *getListenerByName(QString name);
};


#endif
