/* This file is part of the KDE project
   Copyright (C) 2004 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef PERSONALINVITEDIALOG_H
#define PERSONALINVITEDIALOG_H

class PersonalInviteWidget;

#include <qdatetime.h>

#include <kdialogbase.h>

class PersonalInviteDialog : public KDialogBase
{
  public:
    PersonalInviteDialog( QWidget *parent, const char *name );
    virtual ~PersonalInviteDialog() {}

    void setHost( const QString &host, uint port );
    void setPassword( const QString &passwd );
    void setExpiration( const QDateTime &expire );

  protected:
    PersonalInviteWidget *m_inviteWidget;
};

#endif // PERSONALINVITEDIALOG_H

