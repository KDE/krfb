/*
 *  Represents an IP address.
 *  Copyright (C) 2002 Tim Jansen <tim@tjansen.de>
 *  based on code from KInetSocketAddress:
 *    Copyright (C) 2000,2001 Thiago Macieira <thiagom@mail.com>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <config.h>



#include <limits.h>
#include <string.h>

#include <kdebug.h>
#include <klocale.h>
#include "kinetaddr.h"
#include <netdb.h>

#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <arpa/nameser.h> 
#include <resolv.h>

#ifdef sun
#include <sys/socket.h>
#endif

#if defined(__osf__) && defined(AF_INET6)
#undef AF_INET6
#endif

#define V6_CAN_CONVERT_TO_V4(addr)	(KDE_IN6_IS_ADDR_V4MAPPED(addr) || KDE_IN6_IS_ADDR_V4COMPAT(addr))

// This is how it is
// 46 == strlen("1234:5678:9abc:def0:1234:5678:255.255.255.255")
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN		46
#endif

class KInetAddressPrivate
{
public:
	int sockfamily;
	struct in_addr in;
#ifdef AF_INET6
	struct in6_addr in6;
#endif

	KInetAddressPrivate() : sockfamily(AF_UNSPEC)
	{
		memset((void*)&in, 0, sizeof(in));
#ifdef AF_INET6
		memset((void*)&in6, 0, sizeof(in6));
#endif
	}

};

KInetAddress::KInetAddress(const KInetAddress &other) :
	d(new KInetAddressPrivate)
{
	d->sockfamily = other.d->sockfamily;
	memcpy(&d->in, &other.d->in, sizeof(d->in));
#ifdef AF_INET6
	memcpy(&d->in6, &other.d->in6, sizeof(d->in6));
#endif
}

KInetAddress::KInetAddress(const struct in_addr& in) :
	d(new KInetAddressPrivate)
{
	d->sockfamily = AF_INET;
	memcpy(&d->in, &in, sizeof(in));
}

KInetAddress::KInetAddress(const struct in6_addr& in6) :
	d(new KInetAddressPrivate)
{
#ifdef AF_INET6
	d->sockfamily = AF_INET6;
	memcpy(&d->in6, &in6, sizeof(in6));
#else
	d->sockfamily = AF_UNSPEC;
#endif
}

KInetAddress::KInetAddress(const QString &host) :
	d(new KInetAddressPrivate)
{
	struct hostent *h = gethostbyname(host.latin1());
	if ((!h) || (!h->h_addr_list) || (!h->h_addr_list[0])) {
		d->sockfamily = AF_UNSPEC;
		return;
	}
	d->sockfamily = h->h_addrtype;
#ifdef AF_INET6
	if (h->h_addrtype == AF_INET6)
		memcpy(&d->in6, h->h_addr_list[0], h->h_length);
	else
		memcpy(&d->in, h->h_addr_list[0], h->h_length);
#else
	memcpy(&d->in, h->h_addr_list[0], h->h_length);
#endif

	
}

KInetAddress::~KInetAddress()
{
	delete d;
}

const struct in_addr *KInetAddress::addressV4() const {
	if (d->sockfamily != AF_INET)
		return 0;
	return &d->in;
}

#ifdef AF_INET6
const struct in6_addr *KInetAddress::addressV6() const {
	if (d->sockfamily != AF_INET6)
		return 0;
	return &d->in6;
}
#endif

QString KInetAddress::nodeName() const
{
	char buf[INET6_ADDRSTRLEN+1];	// INET6_ADDRSTRLEN > INET_ADDRSTRLEN

#ifdef __osf__ || defined(sun)
	if (d->sockfamily == AF_INET) {
	    char *p = inet_ntoa(d->in);
	    strncpy(buf, p, sizeof(buf));
	}
#else
	if (d->sockfamily == AF_INET)
		inet_ntop(d->sockfamily, (void*)&d->in, buf, sizeof(buf));
#endif
#ifdef AF_INET6
	else if (d->sockfamily == AF_INET6)
		inet_ntop(d->sockfamily, (void*)&d->in6, buf, sizeof(buf));
#endif
	else {
		kdWarning() << "KInetAddress::nodeName() called on uninitialized class\n";
		return i18n("<empty>");
	}

	return QString::fromLatin1(buf); // FIXME! What's the encoding?
}

bool KInetAddress::areEqual(const KInetAddress &a1, const KInetAddress &a2)
{
	if (a1.d->sockfamily != a2.d->sockfamily)
		return false;

	if (a1.d->sockfamily == AF_INET) {
		return (memcmp(&a1.d->in.s_addr, &a2.d->in.s_addr, 
			sizeof(a1.d->in.s_addr))  == 0);
	}
#ifdef AF_INET6
	if (a1.d->sockfamily == AF_INET6) {
		return (memcmp(&a1.d->in6.s6_addr, &a2.d->in6.s6_addr, 
			sizeof(a1.d->in6.s6_addr))  == 0);
	}
#endif
	return true;
}


