/*
 *  KUser - represent a user/account
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
#ifndef KUSER_H
#define KUSER_H

#include <qstring.h>

class KUserPrivate;
struct passwd;

/**
 * An user or account name.
 *
 * This class represents a user on your system. You can
 * between KInetAddress and KInetSocketAddress is that the socket address
 * consists of the address and the port, KInetAddress peresents only the 
 * address itself.
 *
 * @author Tim Jansen <tim@tjansen.de>
 * @short an Internet Address
 */
class KUser {

public:
  /**
   * Creates an object that contains information about the current user.
   * (As returned by getuid(2)).
   */
  KUser();

  /**
   * Creates an object for the user with the given user id.
   * If the user does not exist isValid() will return false.
   * @param uid       	the user id
   */
  KUser(long uid);

  /**
   * Creates an object that contains information about the user with the given
   * name. If the user does not exist isValid() will return false.
   *
   * @param name the name of the user
   */
  KUser(const QString& name);

  /**
   * Returns true if the user is valid. A KUser object can be invalid if 
   * you created it with an non-existing uid or name.
   * @return true if the user is valid
   */
  bool isValid() const;

  /**
   * Returns the user id of the user.
   * @return the id of the user or -1 if user is invalid
   */
  long uid() const;

  /**
   * The login name of the user.
   * @the login name of the user or QString::null if user is invalid 
  */
  QString loginName() const;

  /**
   * The full name of the user.
   * @the full name of the user or QString::null if user is invalid 
  */
  QString fullName() const;

  /**
   * Destructor
   */
  virtual ~KUser();

private:
  KUserPrivate* d;
  void fillPasswd(struct passwd* p);
};

#endif
