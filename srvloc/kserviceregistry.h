/*
 *  Interface to register SLP services.
 *  Copyright (C) 2002 Tim Jansen <tim@tjansen.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

/*
 * TODO: put private variables and SLP-dependencies in private class
 */

#ifndef __KSERVICEREGISTRY_H
#define __KSERVICEREGISTRY_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

class KServiceRegistryPrivate;

/**
 * KServiceRegistry allows you to announce your service using SLP (RFC 2608).
 *
 * Use KServiceRegistry to announce a service. The service will be valid 
 * until its lifespan ends, you unregister it or delete the KServiceRegistry,
 * whatever comes first. 
 * A service will be described using a Service URL and an optional list of 
 * attributes. The syntax for a simple Service URL is 
 * <pre>
 *   service:%srvtype%://%addrspec%
 * </pre>
 * where "%srvtype%" specifies the protocol and "%addrspec5" the address. 
 * Examples of valid Service URLs are "service:http://www.kde.org", 
 * "service:lpr://printer.example.org/myqueure" and 
 * "service:http://your.server.org:8080".
 * To provide more information about your service than just the protocol, 
 * port and address you can use abstract service types.
 * A Service URL that uses an abstract service type has the form 
 * <pre>
 *   service:%abstract-type%:%concrete-type%
 * </pre>
 * where %abstract-type% describes the kind of service and %concrete-type% specifies
 * the protocol and address. Examples are 
 * "service:printer:lpr://printer.example.com/lp0" and 
 * "service:printer:ipp://printer.example.com/".
 * Note that you cannot invent you own types but only take those that are 
 * registered at IANA. To create your own service type you must become a naming
 * authority. Then you can use service types of the form 
 * "%srvtype%.%naming-authority%". Assuming that "kde" is the id of a 
 * IANA-registered naming authority, a valid Service URL would be
 * "service:weatherp.kde://weather.example.com". You can find more information 
 * about Service URLs in the IETF RFCs 2608 and 3224.
 * Additionally each service can have one or more attributes to carry 
 * additional information about the service. Attributes are a simple pair of 
 * strings, one for the attributes name and one for the value. They can be 
 * used, for example, to describe the type of a printer or the email address
 * of an administrator. 
 * 
 * Service templates can be used to describe an abstract type, including the
 * syntax of the concrete type and the attributes. The use of service 
 * templates is described in RFC 2609, you can find the existing service 
 * templates at http://www.iana.org/assignments/svrloc-templates.htm .
 * 
 * Example:
 * <pre>
 *   KServiceRegistry ksr;
 *   KInetAddress kia = KInetAddress->getLocalAddress();
 *   ksr.registerService(QString("service:remotedesktop.kde:vnc://%1:0").arg(kia->nodeName()),
 *                       "(type=shared)");
 *   delete kia;
 * </pre>
 * 
 * @version $Id$
 * @author Tim Jansen, tim@tjansen.de 
 */
class KServiceRegistry {
 public:
	/**
	 * Creates a new service registration instance for the given 
	 * language.
	 * @param lang the language as two letter code, or QString::null for the 
	 *             system default
	 */
	KServiceRegistry(const QString &lang = QString::null);
	virtual ~KServiceRegistry();

	/**
	 * Returns true if service registration is generally possible.
	 * Reasons for a failure could be that the SLP libraries are not
	 * installed or no SA daemon (slpd) is installed
	 * @return true if service registration seems to be possible
	 */
	bool available();

	/**
	 * Creates a comma-separated string of lists, as required by many functions.
	 * @param map the items of this list will be converted
	 * @return the comma-separated list
	 */
	static QString createCommaList(const QStringList &values);

	/**
	 * Encodes an QString for use as a attribute value. This will escape
	 * all characters that are not allowed. This method is only available
	 * when a SLP library is available, otherwise it will return the
	 * given value.
	 * @param value the value string to encode
	 * @return the encoded value string
	 */
	static QString encodeAttributeValue(const QString &value);

	/**
	 * Registers the given service. 
	 * @param serviceURL the service URL to register
	 * @param attributes a list of the attributes to register, encoded in 
	 *                   the format "(attr1=val1),(attr2=val2),(attr3=val3)"
	 *                   Use an empty string if you dont want to set attributes
	 * @param lifetime   the lifetime of the service in seconds, or 0 for infinite
	 * @return true if successful, false otherwise. False usually means that no 
	 *              SA daemon (slpd) is running.
	 */
	bool registerService(const QString &serviceURL, 
			     QString attributes = QString::null, 
			     unsigned short lifetime = 0);
	
	/**
	 * Registers the given service. 
	 * @param serviceURL the service URL to register
	 * @param attributes a map of all attributes
	 * @param lifetime   the lifetime of the service in seconds, or 0 for infinite
	 * @return true if successful, false otherwise. False usually means that no 
	 *              SA daemon is running (slpd).
	 */	
	bool registerService(const QString &serviceURL, 
			     QMap<QString,QString> attributes, 
			     unsigned short lifetime = 0);
	
	/**
	 * Unregisters the given service.
	 * @param serviceURL the service URL to unregister
	 */
	void unregisterService(const QString &serviceURL);

 private:
	KServiceRegistryPrivate *d;
};

#endif
