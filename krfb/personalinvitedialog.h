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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef PERSONALINVITEDIALOG_H
#define PERSONALINVITEDIALOG_H

#include <qdatetime.h>
#include <KDialog>
#include "ui_rfbpersonalinvitewidget.h"

class QWidget;
class PersonalInviteDialog : public KDialog, public Ui::PersonalInviteWidget
{
  public:
    PersonalInviteDialog( QWidget *parent );
    virtual ~PersonalInviteDialog() {}

    void setHost( const QString &host, uint port );
    void setPassword( const QString &passwd );
    void setExpiration( const QDateTime &expire );

  protected:
      QWidget *m_inviteWidget;
};

#endif // PERSONALINVITEDIALOG_H

