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
 * This class represents a user on your system. You can either get
 * information about the current user, of fetch information about
 * a user on the system.
 *
 * @author Tim Jansen <tim@tjansen.de>
 * @short Represents a user on your system
 * @since 3.2
 */
class KUser {

public:

  enum UIDMode{ UseEffectiveUID, UseRealUserID };

  /**
   * Creates an object that contains information about the current user.
   * (as returned by getuid(2) or geteuid(2)).
   * @param If UseEffectiveUID it passed the effective user is returned. 
   *        Otherwise the real user will be returned. The difference is that
   *        that when the user uses a command like "su", this will change the
   *        effective user, but not the real user. Use the effective user when 
   *        checking permissions, and the real user for displaying
   *        information about the user
   */
  KUser(UIDMode mode = UseEffectiveUID);

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
   * Copy constructor.
   * Makes a deep copy.
   */
  KUser(const KUser &user);

  /**
   * Assignment operator.
   * Makes a deep copy.
   */
  KUser& operator =(const KUser& user);

  /**
   * Two KUser objects are equal if @ref isValid() and the uid() are
   * identical.
   */
  bool operator ==(const KUser& user);

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
   * Returns the group id of the user.
   * @return the id of the group or -1 if user is invalid
   */
  long gid() const;

  /**
   * Checks whether the user it the super user (root).
   * @return true if the user is root
   */
  bool isSuperUser() const;

  /**
   * The login name of the user.
   * @return the login name of the user or QString::null if user is invalid 
   */
  QString loginName() const;

  /**
   * The full name of the user.
   * @return the full name of the user or QString::null if user is invalid
   */
  QString fullName() const;

  /**
   * The user's room number.
   * @return the room number of the user or QString::null if not set or the
   *         user is invalid 
   */
  QString roomNumber() const;

  /**
   * The user's work phone.
   * @return the work phone of the user or QString::null if not set or the
   *         user is invalid 
   */
  QString workPhone() const;

  /**
   * The user's home phone.
   * @return the home phone of the user or QString::null if not set or the
   *         user is invalid 
   */
  QString homePhone() const;

  /**
   * The path to the user's home directory.
   * @return the home phone of the user or QString::null if the
   *         user is invalid 
   */
  QString homeDir() const;

  /**
   * The path to the user's login shell.
   * @return the login shell of the user or QString::null if the
   *         user is invalid 
   */
  QString shell() const;

  /**
   * Destructor
   */
  ~KUser();

private:
  KUserPrivate* d;
  void fillPasswd(struct passwd* p);
};

#endif
