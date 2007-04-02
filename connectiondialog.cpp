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

#include "connectiondialog.h"

#include <QCheckBox>
#include <QLabel>

#include <KIconLoader>
#include <KLocale>
#include <KStandardGuiItem>

ConnectionDialog::ConnectionDialog( QWidget *parent )
    : KDialog( parent )
{
  setCaption(i18n("New Connection"));
  setButtons(Ok|Cancel);
  setDefaultButton(Cancel);
  setModal(true);

  m_connectWidget = new QWidget( this );
  setupUi(m_connectWidget);

  pixmapLabel->setPixmap(
      UserIcon( "connection-side-image.png" ) );

  KGuiItem accept = KStandardGuiItem::ok();
  accept.setText( i18n( "Accept Connection" ) );
  setButtonGuiItem(Ok, accept);

  KGuiItem refuse = KStandardGuiItem::cancel();
  refuse.setText( i18n( "Refuse Connection" ) );
  setButtonGuiItem(Cancel, refuse);

  setMainWidget( m_connectWidget );
}

void ConnectionDialog::setRemoteHost( const QString &host )
{
  remoteHost->setText( host );
}

void ConnectionDialog::setAllowRemoteControl( bool b )
{
  cbAllowRemoteControl->setChecked( b );
}

bool ConnectionDialog::allowRemoteControl()
{
  return cbAllowRemoteControl->isChecked();
}

#include "connectiondialog.moc"
