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

#include "connectiondialog.h"
#include "connectionwidget.h"

#include <qcheckbox.h>
#include <qlabel.h>

#include <kiconloader.h>
#include <klocale.h>

ConnectionDialog::ConnectionDialog( QWidget *parent, const char *name )
    : KDialogBase( parent, name, true, i18n( "New Connection" ),
                   Ok|Cancel, Cancel, true )
{
  m_connectWidget = new ConnectionWidget( this, "ConnectWidget" );
  m_connectWidget->pixmapLabel->setPixmap(
      UserIcon( "connection-side-image.png" ) );

  KGuiItem accept = KStdGuiItem::ok();
  accept.setText( i18n( "Accept Connection" ) );
  setButtonOK( accept );

  KGuiItem refuse = KStdGuiItem::cancel();
  refuse.setText( i18n( "Refuse Connection" ) );
  setButtonCancel( refuse );

  setMainWidget( m_connectWidget );
}

void ConnectionDialog::setRemoteHost( const QString &host )
{
  m_connectWidget->remoteHost->setText( host );
}

void ConnectionDialog::setAllowRemoteControl( bool b )
{
  m_connectWidget->cbAllowRemoteControl->setChecked( b );
}

bool ConnectionDialog::allowRemoteControl()
{
  return m_connectWidget->cbAllowRemoteControl->isChecked();
}

#include "connectiondialog.moc"
