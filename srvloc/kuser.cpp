/*
 *  This file is part of the KDE libraries
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

#include <kuser.h>

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>


class KUserPrivate
{
public:
	bool valid;
	long uid;
	QString loginName, fullName;

	KUserPrivate() {
		valid = false;
	}

	KUserPrivate(long _uid,
		     const QString &_loginname, 
		     const QString &_fullname) {
		valid = true;
		uid = _uid;
		loginName = _loginname;
		fullName = _fullname;
	}
};


KUser::KUser() {
	fillPasswd(getpwuid(getuid()));
}

KUser::KUser(long uid) {
	fillPasswd(getpwuid(uid));
}

KUser::KUser(const QString& name) {
	fillPasswd(getpwnam(name.latin1()));
}

void KUser::fillPasswd(struct passwd *p) {
	if (p) {
		QString fn(p->pw_gecos);
		int pos = fn.find(',');
		if (pos >= 0)
			fn = fn.left(pos);

		d = new KUserPrivate(p->pw_uid, 
				     QString(p->pw_name),
				     fn.stripWhiteSpace());
	}
	else
		d = new KUserPrivate();
}

bool KUser::isValid() const {
	return d->valid;
}

long KUser::uid() const {
	if (d->valid)
		return d->uid;
	else
		return -1;
}

QString KUser::loginName() const {
	if (d->valid)
		return d->loginName;
	else
		return QString::null;
}

QString KUser::fullName() const {
	if (d->valid)
		return d->fullName;
	else
		return QString::null;
}

KUser::~KUser() {
	delete d;
}
