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

#include "invitedialog.h"
#include "invitewidget.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kstdguiitem.h>

#include <qlabel.h>
#include <qpushbutton.h>

InviteDialog::InviteDialog( QWidget *parent, const char *name )
    : KDialogBase( parent, name, true, i18n( "Invitation" ),
      User1|Close|Help, NoDefault, true )
{
  m_inviteWidget = new InviteWidget( this, "InviteWidget" );
  m_inviteWidget->pixmapLabel->setPixmap(
      UserIcon( "connection-side-image.png" ) );
  setMainWidget( m_inviteWidget );

  setButtonGuiItem( User1, KStdGuiItem::configure() );

  connect( m_inviteWidget->btnCreateInvite, SIGNAL( clicked() ),
           SIGNAL( createInviteClicked() ) );
  connect( m_inviteWidget->btnEmailInvite, SIGNAL( clicked() ),
           SIGNAL( emailInviteClicked() ) );
  connect( m_inviteWidget->btnManageInvite, SIGNAL( clicked() ),
           SIGNAL( manageInviteClicked() ) );
}

void InviteDialog::slotUser1()
{
  emit configureClicked();
}

void InviteDialog::enableInviteButton( bool enable )
{
  m_inviteWidget->btnCreateInvite->setEnabled( enable );
}

void InviteDialog::setInviteCount( int count )
{
  m_inviteWidget->btnManageInvite->setText(
      i18n( "&Manage Invitations (%1)..." ).arg( count ) );
}

#include "invitedialog.moc"
