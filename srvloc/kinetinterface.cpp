/*
 *  Represents an Inet interface
 *  Copyright (C) 2002 Tim Jansen <tim@tjansen.de>
 *
 *  $Id$
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

#include "kinetinterface.h"

#include "getifaddrs.h"

#include <netinet/in.h>
#include <ksocketaddress.h>


class KInetInterfacePrivate {
public:
	QString name;
	int flags;
	KInetSocketAddress *address;
	KInetSocketAddress *netmask;
	KInetSocketAddress *broadcast;
	KInetSocketAddress *destination;

	KInetInterfacePrivate() :
		flags(0),
		address(0),
		netmask(0),
		broadcast(0),
		destination(0) {
	}

	KInetInterfacePrivate(const QString _name, 
			      int _flags, 
			      KInetSocketAddress *_address,
			      KInetSocketAddress *_netmask,
			      KInetSocketAddress *_broadcast,
			      KInetSocketAddress *_destination) :
		name(_name),
		flags(_flags),
		address(_address),
		netmask(_netmask),
		broadcast(_broadcast),
		destination(_destination) {
	}

	~KInetInterfacePrivate() {
		if (address)
			delete address;
		if (netmask)
			delete netmask;
		if (broadcast)
			delete broadcast;
		if (destination)
			delete destination;
	}
	
	KInetInterfacePrivate& operator =(const KInetInterfacePrivate& i) {
		name = i.name;
		flags = i.flags;
		if (i.address)
			address = new KInetSocketAddress(*i.address);
		else
			address = 0;
		if (i.netmask)
			netmask = new KInetSocketAddress(*i.netmask);
		else
			netmask = 0;
		if (i.broadcast)
			broadcast = new KInetSocketAddress(*i.broadcast);
		else
			broadcast = 0;
		if (i.destination)
			destination = new KInetSocketAddress(*i.destination);
		else
			destination = 0;
		return *this;
	}

};


KInetInterface::KInetInterface() :
	d(0) {
}

KInetInterface::KInetInterface(const QString &_name, 
			       int _flags, 
			       KInetSocketAddress *_address,
			       KInetSocketAddress *_netmask,
			       KInetSocketAddress *_broadcast,
			       KInetSocketAddress *_destination) {
	d = new KInetInterfacePrivate(_name, _flags,
				      _address, _netmask, 
				      _broadcast, _destination);
}

KInetInterface::KInetInterface(const KInetInterface &i) :
	d(0) {
	if (!i.d)
		return;

	d = new KInetInterfacePrivate();
	*d = *i.d;
}

KInetInterface::~KInetInterface() {
	if (d)
		delete d;
}

KInetInterface& KInetInterface::operator =(const KInetInterface& i) {
	if (this == &i)
		return *this;

	if (d)
		delete d;
	d = 0;
	if (!i.d)
		return *this;

	d = new KInetInterfacePrivate();
	*d = *i.d;
	return *this;
}

bool KInetInterface::isValid() const {
	return d == 0;
}

QString KInetInterface::displayName() const {
	return d->name;
}

QString KInetInterface::name() const {
	return d->name;
}

int KInetInterface::flags() const {
	return d->flags;
}

KInetSocketAddress *KInetInterface::address() const {
	return d->address;
}

KInetSocketAddress *KInetInterface::netmask() const {
	return d->netmask;
}

KInetSocketAddress *KInetInterface::broadcastAddress() const {
	return d->broadcast;
}

KInetSocketAddress *KInetInterface::destinationAddress() const {
	return d->destination;
}

KInetSocketAddress *KInetInterface::getPublicInetAddress() {
	Q3ValueVector<KInetInterface> v = getAllInterfaces(true);		

	// TODO: first step: take the default route interface

	// try to find point-2-point interface, because it may be
	// a dial-up connection. Take it.
	Q3ValueVector<KInetInterface>::const_iterator it = v.begin();
	while (it != v.end()) {
		if (((*it).flags() & (PointToPoint | Up | Running)) &&
		    (!((*it).flags() & Loopback)) &&
		    (*it).address() &&
            ((*it).address()->family() == AF_INET))
			return new KInetSocketAddress(*(*it).address());
		it++;
	}

	// otherwise, just take the first non-loopback interface
	it = v.begin();
	while (it != v.end()) {
		if (((*it).flags() & (Up | Running)) &&
		    (!((*it).flags() & Loopback)) &&
		    (*it).address() &&
            ((*it).address()->family() == AF_INET))
			return new KInetSocketAddress(*(*it).address());
		it++;
	}

	// ok, giving up, try to take loopback
	it = v.begin();
	while (it != v.end()) {
		if (((*it).flags() & (Up | Running)) &&
		    (*it).address())
			return new KInetSocketAddress(*(*it).address());
		it++;
	}

        // not even loopback..
	return 0;
}

namespace {
	KInetSocketAddress *createAddress(struct sockaddr *a) {
		if (!a)
			return 0;
		else if (a->sa_family == AF_INET)
			return new KInetSocketAddress((struct sockaddr_in*) a,
						      sizeof(struct sockaddr_in));
#ifdef AF_INET6
		else if (a->sa_family == AF_INET6)
			return new KInetSocketAddress((struct sockaddr_in6*) a,
						      sizeof(struct sockaddr_in6));
#endif
		else
			return 0;
	}

	int convertFlags(int flags) {
		int r = 0;
		if (flags & IFF_UP)
			r |= KInetInterface::Up;
		if (flags & IFF_BROADCAST)
			r |= KInetInterface::Broadcast;
		if (flags & IFF_LOOPBACK)
			r |= KInetInterface::Loopback;
		if (flags & IFF_POINTOPOINT)
			r |= KInetInterface::PointToPoint;
		if (flags & IFF_RUNNING)
			r |= KInetInterface::Running;
		if (flags & IFF_MULTICAST)
			r |= KInetInterface::Multicast;

		return r;
	}
}

Q3ValueVector<KInetInterface> KInetInterface::getAllInterfaces(bool includeLoopback) {
	struct kde_ifaddrs *ads;
	struct kde_ifaddrs *a;
	Q3ValueVector<KInetInterface> r;
	if (kde_getifaddrs(&ads))
		return r;

	a = ads;
	while (a) {
		if ((a->ifa_flags & IFF_LOOPBACK) && !includeLoopback) {
			a = a->ifa_next;
			continue;
		}
		r.push_back(KInetInterface(QString::fromUtf8(a->ifa_name),
					   convertFlags(a->ifa_flags),
					   createAddress(a->ifa_addr),
					   createAddress(a->ifa_netmask),
					   (a->ifa_flags & IFF_BROADCAST) ?
					   createAddress(a->ifa_broadaddr) : 0,
					   (a->ifa_flags & IFF_POINTOPOINT) ? 
					   createAddress(a->ifa_dstaddr) : 0));
		a = a->ifa_next;
	}
	
	kde_freeifaddrs(ads);
	return r;
}

