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

#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <kdialogbase.h>

class ConnectionWidget;

class ConnectionDialog : public KDialogBase
{
  Q_OBJECT

  public:
    ConnectionDialog( QWidget *parent, const char *name );
    ~ConnectionDialog() {};

    void setRemoteHost( const QString &host );
    void setAllowRemoteControl( bool b );
    bool allowRemoteControl();

  protected:
    ConnectionWidget *m_connectWidget;
};

#endif // CONNECTIONDIALOG_H

