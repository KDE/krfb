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

#include <kuser.h>

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <qstringlist.h>

class KUserPrivate
{
public:
	bool valid;
	long uid, gid;
	QString loginName, fullName;
	QString roomNumber, workPhone, homePhone;
	QString homeDir, shell;

	KUserPrivate() {
		valid = false;
	}

	KUserPrivate(long _uid,
		     long _gid,
		     const QString &_loginname, 
		     const QString &_fullname,
		     const QString &_room,
		     const QString &_workPhone,
		     const QString &_homePhone,
		     const QString &_homedir,
		     const QString &_shell) :
		valid(true),
		uid(_uid),
		gid(_gid),
		loginName(_loginname),
		fullName(_fullname),
		roomNumber(_room),
		workPhone(_workPhone),
		homePhone(_homePhone),
		homeDir(_homedir),
		shell(_shell) {
	}
};


KUser::KUser(UIDMode mode) {
	fillPasswd(getpwuid((mode == UseEffectiveUID) ? geteuid() : getuid()));
}

KUser::KUser(long uid) {
	fillPasswd(getpwuid(uid));
}

KUser::KUser(const QString& name) {
	fillPasswd(getpwnam((const char*)name.utf8()));
}

KUser::KUser(const KUser &user) {
	d = new KUserPrivate(user.uid(),
			     user.gid(),
			     user.loginName(),
			     user.fullName(), 
			     user.roomNumber(),
			     user.workPhone(),
			     user.homePhone(),
			     user.homeDir(),
			     user.shell());
}

KUser& KUser::operator =(const KUser& user) {
	if ( this == &user ) 
		return *this;
	
	delete d;
	d = new KUserPrivate(user.uid(),
			     user.gid(),
			     user.loginName(),
			     user.fullName(), 
			     user.roomNumber(),
			     user.workPhone(),
			     user.homePhone(),
			     user.homeDir(),
			     user.shell());
	return *this;
}

bool KUser::operator ==(const KUser& user) {
    if (isValid() != user.isValid())
	return false;
    if (isValid())
	return uid() == user.uid();
    else
	return true;
}

void KUser::fillPasswd(struct passwd *p) {
	if (p) {
		QString gecos = QString::fromLocal8Bit(p->pw_gecos);
		QStringList gecosList = QStringList::split(',', gecos, true);

		d = new KUserPrivate(p->pw_uid,
				     p->pw_gid,
				     QString::fromLocal8Bit(p->pw_name),
				     (gecosList.size() > 0) ? gecosList[0] : QString::null,
				     (gecosList.size() > 1) ? gecosList[1] : QString::null,
				     (gecosList.size() > 2) ? gecosList[2] : QString::null,
				     (gecosList.size() > 3) ? gecosList[3] : QString::null,
				     QString::fromLocal8Bit(p->pw_dir),
				     QString::fromLocal8Bit(p->pw_shell));
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

long KUser::gid() const {
	if (d->valid)
		return d->gid;
	else
		return -1;
}

bool KUser::isSuperUser() const {
	return uid() == 0;
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

QString KUser::roomNumber() const {
	if (d->valid)
		return d->roomNumber;
	else
		return QString::null;
}

QString KUser::workPhone() const {
	if (d->valid)
		return d->workPhone;
	else
		return QString::null;
}

QString KUser::homePhone() const {
	if (d->valid)
		return d->homePhone;
	else
		return QString::null;
}

QString KUser::homeDir() const {
	if (d->valid)
		return d->homeDir;
	else
		return QString::null;
}

QString KUser::shell() const {
	if (d->valid)
		return d->shell;
	else
		return QString::null;
}

KUser::~KUser() {
	delete d;
}
