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

#include "personalinvitedialog.h"
#include "personalinvitewidget.h"

#include <qlabel.h>

#include <kactivelabel.h>
#include <kiconloader.h>
#include <klocale.h>

PersonalInviteDialog::PersonalInviteDialog( QWidget *parent, const char *name )
    : KDialogBase( parent, name, true, i18n( "Personal Invitation" ),
                   Close, Close, true )
{
  m_inviteWidget = new PersonalInviteWidget( this, "PersonalInviteWidget" );
  m_inviteWidget->pixmapLabel->setPixmap( 
      UserIcon( "connection-side-image.png" ) );

  setMainWidget( m_inviteWidget );
}

void PersonalInviteDialog::setHost( const QString &host, uint port )
{
  m_inviteWidget->hostLabel->setText( QString( "%1:%2" )
      .arg( host ).arg( port ) );
}

void PersonalInviteDialog::setPassword( const QString &passwd )
{
  m_inviteWidget->passwordLabel->setText( passwd );
}

void PersonalInviteDialog::setExpiration( const QDateTime &expire )
{
  m_inviteWidget->expirationLabel->setText( expire.toString( Qt::LocalDate ) );
}
