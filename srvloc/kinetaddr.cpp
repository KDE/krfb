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

class KInetAddressPrivate
{
public:
	int sockfamily;
	struct in_addr in;

	KInetAddressPrivate() : sockfamily(AF_UNSPEC)
	{
		memset((void*)&in, 0, sizeof(in));
	}

};

KInetAddress::KInetAddress(const KInetAddress &other) :
	d(new KInetAddressPrivate)
{
	d->sockfamily = other.d->sockfamily;
	memcpy(&d->in, &other.d->in, sizeof(d->in));
}

KInetAddress::KInetAddress(const struct in_addr& in) :
	d(new KInetAddressPrivate)
{
	d->sockfamily = AF_INET;
	memcpy(&d->in, &in, sizeof(in));
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
	memcpy(&d->in, h->h_addr_list[0], h->h_length);
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

QString KInetAddress::nodeName() const
{
	char buf[INET_ADDRSTRLEN+1];

#ifdef __osf__ || defined(sun)
	if (d->sockfamily == AF_INET) {
	    char *p = inet_ntoa(d->in);
	    strncpy(buf, p, sizeof(buf));
	}
#else
	if (d->sockfamily == AF_INET)
		inet_ntop(d->sockfamily, (void*)&d->in, buf, sizeof(buf));
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
	return true;
}


