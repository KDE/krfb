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
#include <qmap.h>
#include <qvaluevector.h>
#include <qcstring.h>
#include <qstring.h>


class KInetAddressPrivate;


/**
 * An Inet (IPv4 or IPv6) address.
 *
 * This class represents an internet (IPv4 or IPv6) address. The difference
 * between @ref KInetAddress and @ref KInetSocketAddress is that the socket 
 * address consists of the address and the port, KInetAddress represents 
 * only the address itself.
 *
 * @author Tim Jansen <tim@tjansen.de>
 * @short Represents an Internet Address
 * @since 3.2
 */
class KInetAddress {
public:
  /**
   * Default constructor. Creates an uninitialized KInetAddress.
   */
  KInetAddress();

  /**
   * Copy constructor
   * @param a the KInetAddress to copy
   */
  KInetAddress(const KInetAddress &a);

  /**
   * Creates an IPv4 socket from in_addr
   * @param in		a in_addr structure to copy from
   * @param len		the socket address length
   */
  KInetAddress(const struct in_addr& in);

  /**
   * Creates an IPv6 socket from in6_addr
   * @param in6       	a in_addr6 structure to copy from
   * @param len		the socket address length
   */
  KInetAddress(const struct in6_addr& in6);

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
   * Assignment, makes a deep copy of @p a.
   * @param a the KInetAddress to copy
   * @return this KInetAddress
   */
  KInetAddress& operator =(const KInetAddress& a);

  /**
   * Returns a text representation of the host address.
   * @return a text representation of the host address, or 
   *         QString::null if created using the default
   *         constructor
   */
  virtual QString nodeName() const;

  /**
   * Checks whether this KInetAddress equals the @p other.
   * @param other the other KInetAddress to compare
   * @return true if both addresses equal
   * @see areEqual()
   */
  bool isEqual(const KInetAddress* other) const
  { return areEqual(*this, *other); }

  /**
   * Checks whether this KInetAddress equals the @p other.
   * @param other the other KInetAddress to compare
   * @return true if both addresses equal
   * @see areEqual()
   */
  bool operator==(const KInetAddress& other) const
  { return areEqual(*this, other); }
    
  /**
   * Checks whether the given KInetAddresses are equal.
   * @param a1 the first KInetAddress to compare
   * @param a2 the second KInetAddress to compare
   * @return true if both addresses equal
   * @see isEqual()
   */
  static bool areEqual(const KInetAddress &a1, const KInetAddress &a2);

  /**
   * Returns the in_addr structure.
   * @return the in_addr structure, or 0 if this is not a v4 address.
   *         The pointer is valid as long as the KInetAddress object lives.
   * @see addressV6
   */
  const struct in_addr* addressV4() const;

  /**
   * Returns the in6_addr structure. 
   * @return the in_addr structure, or 0 if this is not a v6 address.
   *         The pointer is valid as long as the KInetAddress object lives.
   * @see addressV4
   */
  const struct in6_addr* addressV6() const;

  operator const struct in_addr*() const
  { return addressV4(); }

  operator const struct in6_addr*() const
  { return addressV6(); }

  /**
   * Tries to guess the public internet address of this computer.
   * This is not always successful, especially when the computer
   * is behind a firewall or NAT gateway. In the worst case, it
   * returns localhost.
   * @return a KInetAddress object that contains the best match
   */
  static KInetAddress getPublicInetAddress();

  /**
   * Returns all active IP addresses that can be used to reach this
   * system.
   * @param includeLoopback if true, include the loopback interface's 
   *                        name
   * @return the list of IP addresses
   */
  static QValueVector<KInetAddress> getAllAddresses(bool includeLoopback = false);

  /**
   * Returns all active interfaces as name/IP address pairs.
   * @param includeLoopback if true, include the loopback interface's 
   *                        name
   * @return the map of interfaces. The QString contains the name (like 'eth0'
   *         or 'ppp1'), KInetAddress the address
   */
  static QMap<QString,KInetAddress> getAllInterfaces(bool includeLoopback = false);


private:
  KInetAddressPrivate* d;
};

#endif
