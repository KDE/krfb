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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */
#ifndef KINETINTERFACE_H
#define KINETINTERFACE_H

#include <qvaluevector.h>
#include <qcstring.h>
#include <qstring.h>


class KInetInterfacePrivate;
class KInetSocketAddress;

/**
 * An Internet (IPv4 or IPv6) network interface. 
 *
 * Represents a snapshot of the network interface's state. It is
 * not possible to modify the interface using this class, only 
 * to read it. Note that the interfaces can change it any time 
 * (for example when the internet connection goes up or down), 
 * so when you use it in a server you may want to use it together
 * with a @ref KInetInterfaceWatcher.
 * Use @ref getAllInterfaces() to get all interfaces of a system.
 *
 * @author Tim Jansen <tim@tjansen.de>
 * @short Represents an IPv4 or IPv6 Network Interface 
 * @see KInetInterfaceWatcher
 * @since 3.2
 */
class KInetInterface {
private:
  KInetInterface(const QString &name, 
		 int flags, 
		 KInetSocketAddress *address,
		 KInetSocketAddress *netmask,
		 KInetSocketAddress *broadcast,
		 KInetSocketAddress *destination);
  
public:
  /**
   * Default constructor. Creates an uninitialized KInetInterface.
   * @see isValid()
   */
  KInetInterface();

  /**
   * Copy constructor. Makes a deep copy.
   * @param i the KInetInterface to copy
   */
  KInetInterface(const KInetInterface &i);

  /**
   * Destructor
   */
  virtual ~KInetInterface();

  /**
   * Assignment, makes a deep copy of @p i.
   * @param i the KInetInterface to copy
   * @return this KInetInterface
   */
  KInetInterface& operator =(const KInetInterface& i);

  /**
   * Checks whether the object is valid. Only interfaces that
   * have been created using the default constructor are invalid.
   * @return true if valid, false if invalid
   */
  bool isValid() const;

  /**
   * Returns a user-readable name of the interface, if available.
   * Otherwise it returns the same value as @ref name().
   * @return the display name of the interface
   * @see name()
   */
  QString displayName() const;

  /**
   * Returns the name of the interface, e.g. 'eth0'. 
   * @return the name of the interface
   * @see displayName()
   */
  QString name() const;

  /**
   * Flags describing the interface. These flags
   * can be ORed.
   */
  enum Flags {
    Up = 1, ///< Interface is up.
    Broadcast = 2, ///< Broadcast address (@ref broadcastAddress()) is valid..
    Loopback = 8,  ///< Interface is a loopback interface.
    PointToPoint = 16, ///< Interface is a point-to-point interface.
    Running = 128,     ///< Interface is running.
    Multicast = 65536  ///< Interface is multicast-capable.
  };

  /**
   * A set of ORed flags describing the interface. See
   * @ref Flags for description of the flags.
   * @return the ORed @ref Flags of the interface
   */
  int flags() const;

  /**
   * Returns the address of the interface.
   * The returned object is valid as long as this object
   * exists.
   * @return the address of this interface, can be 0 if the interface
   *         does not have an address
   */
  KInetSocketAddress *address() const;

  /**
   * Returns the netmask of the interface.
   * The returned object is valid as long as this object
   * exists.
   * @return the netmask of this interface, can be 0 if the interface
   *         does not have an address
   */
  KInetSocketAddress *netmask() const;

  /**
   * Returns the broadcast address of the interface.
   * The returned object is valid as long as this object
   * exists.
   * @return the broadcast address of this interface. Can be 0 if
   *         the interface is a peer-to-peer interface (like PPP)
   */
  KInetSocketAddress *broadcastAddress() const;

  /**
   * Returns the destination / peer address of the interface.
   * It is used for peer-to-peer interfaces like PPP.
   * The returned object is valid as long as this object
   * exists.
   * @return the destination address of this interface. Can be 0
   *         if the interface is not a peer-to-peer interface
   */
  KInetSocketAddress *destinationAddress() const;

  /**
   * Tries to guess the public internet address of this computer.
   * This is not always successful, especially when the computer
   * is behind a firewall or NAT gateway. In the worst case, it
   * returns localhost.
   * @return a KInetAddress object that contains the best match.
   *         The caller takes ownership of the object and is
   *         responsible for freeing it
   */
  static KInetSocketAddress *getPublicInetAddress();

  /**
   * Returns all active interfaces of the system.
   * 
   * @param includeLoopback if true, include the loopback interface's 
   *                        name
   * @return the list of IP addresses
   */
  static QValueVector<KInetInterface> getAllInterfaces(bool includeLoopback = false);

private:
  KInetInterfacePrivate* d;
};

#endif
