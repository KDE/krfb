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

#ifndef INVITEDIALOG_H
#define INVITEDIALOG_H

class InviteWidget;

#include <KDialog>

class InviteDialog : public KDialog
{
  Q_OBJECT

  public:
    InviteDialog( QWidget *parent );
    ~InviteDialog() {}

    void enableInviteButton( bool enable );

  public slots:
    void setInviteCount( int count );

  signals:
    void createInviteClicked();
    void emailInviteClicked();
    void manageInviteClicked();
    void configureClicked();

  protected slots:
    void slotUser1();

  protected:
    InviteWidget *m_inviteWidget;
};

#endif // INVITEDIALOG_H

