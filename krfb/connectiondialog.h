/* This file is part of the KDE project
   Copyright (C) 2010 Collabora Ltd <info@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
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

#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include "ui_connectionwidget.h"
#include <KDialog>

template <typename UI>
class ConnectionDialog : public KDialog
{
public:
    ConnectionDialog(QWidget *parent);
    ~ConnectionDialog() {};

    void setAllowRemoteControl(bool b);
    bool allowRemoteControl();

protected:
    QWidget *m_connectWidget;
    UI m_ui;
};

template <typename UI>
void ConnectionDialog<UI>::setAllowRemoteControl(bool b)
{
    m_ui.cbAllowRemoteControl->setChecked(b);
    m_ui.cbAllowRemoteControl->setVisible(b);
}

template <typename UI>
bool ConnectionDialog<UI>::allowRemoteControl()
{
    return m_ui.cbAllowRemoteControl->isChecked();
}

//*********

class InvitationsConnectionDialog : public ConnectionDialog<Ui::ConnectionWidget>
{
    Q_OBJECT
public:
    InvitationsConnectionDialog(QWidget *parent);
    void setRemoteHost(const QString & host);
};

//*********

#ifdef KRFB_WITH_TELEPATHY_TUBES
# include "ui_tubesconnectionwidget.h"

class TubesConnectionDialog : public ConnectionDialog<Ui::TubesConnectionWidget>
{
    Q_OBJECT
public:
    TubesConnectionDialog(QWidget *parent);
    void setContactName(const QString & name);
};

#endif // KRFB_WITH_TELEPATHY_TUBES

#endif // CONNECTIONDIALOG_H

