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
 */
#ifndef KINETADDR_H
#define KINETADDR_H

#include <sys/types.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#if defined(__FreeBSD__)
#include <sys/socket.h>
#endif

#include <qobject.h>
#include <qcstring.h>
#include <qstring.h>


class KInetAddressPrivate;


/**
 * An Inet (IPv4 or IPv6) address.
 *
 * This class represents an internet (IPv4 or IPv6) address. The difference
 * between KInetAddress and KInetSocketAddress is that the socket address
 * consists of the address and the port, KInetAddress peresents only the 
 * address itself.
 *
 * @author Tim Jansen <tim@tjansen.de>
 * @short an Internet Address
 */
class KInetAddress {
public:
  /**
   * Copy constructor
   */
  KInetAddress(const KInetAddress&);

  /**
   * Creates an IPv4 socket from in_addr
   * @param in		a in_addr structure to copy from
   * @param len		the socket address length
   */
  KInetAddress(const struct in_addr& in);

#ifdef AF_INET6
   /**
   * Creates an IPv6 socket from in6_addr
   * @param in6       	a in_addr6 structure to copy from
   * @param len		the socket address length
   */
  KInetAddress(const struct in6_addr& in6);
#endif

  /**
   * Creates a socket from text representation. Be careful with names
   * as they may require a name server lookup which can block for a 
   * long time.
   *
   * @param addr	a text representation of the address
   */
  KInetAddress(const QString& addr);

  /**
   * Destructor
   */
  virtual ~KInetAddress();

  /**
   * Returns a text representation of the host address
   */
  virtual QString nodeName() const;

  bool isEqual(const KInetAddress* other) const
  { return areEqual(*this, *other); }

  bool operator==(const KInetAddress& other) const
  { return areEqual(*this, other); }
    
  static bool areEqual(const KInetAddress &a1, const KInetAddress &a2);

  /**
   * Returns the in_addr structure. The pointer is valid as long as
   * the KInetAddress object lives.
   * This will be NULL if this is not a v4 address.
   * @see addressV6
   */
  const struct in_addr* addressV4() const;

  /**
   * Returns the in6_addr structure. The pointer is valid as long as
   * the KInetAddress object lives.
   * This will be NULL if this is not a v6 address.
   * @see addressV4
   */
#ifdef AF_INET6
  const struct in6_addr* addressV6() const;
#endif

  operator const struct in_addr*() const
  { return addressV4(); }
#ifdef AF_INET6
  operator const struct in6_addr*() const
  { return addressV6(); }
#endif
  /**
   * Returns an address that can be used for communication with
   * other computers on the internet.
   * Note that the returned address is not always a real 
   * internet address, because the computer couble be unable to connect 
   * to the internet or is behind a NAT gateway.
   * In the worst case you will get the address of the local loopback 
   * interface.
   * The user is responsible for freeing the object.
   * @return a new KInetAddress object that contains the address
   */
  static KInetAddress* getPrivateInetAddress();

  /**
   * Returns the address of the interface that should be used
   * to announce local services.
   * Note that the returned address is not always a real internet address, 
   * because the computer may be behind a NAT gateway, or 
   * it is no connected to the internet. In the worst case you 
   * will get the address of the local loopback interface.
   * The user is responsible for freeing the object.
   * @return a new KInetAddress object that contains the address
   */
  static KInetAddress* getLocalAddress();


private:
  KInetAddressPrivate* d;
};

#endif
