/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2010 Collabora Ltd <info@collabora.co.uk>
   SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
   SPDX-FileCopyrightText: 2004 Nadeem Hasan <nhasan@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include "ui_connectionwidget.h"
#include <QDialog>

template<typename UI>
class ConnectionDialog : public QDialog
{
public:
    explicit ConnectionDialog(QWidget *parent);
    ~ConnectionDialog() override {};

    void setAllowRemoteControl(bool b);
    bool allowRemoteControl();

protected:
    QWidget *m_connectWidget;
    UI m_ui;
};

template<typename UI>
void ConnectionDialog<UI>::setAllowRemoteControl(bool b)
{
    m_ui.cbAllowRemoteControl->setChecked(b);
    m_ui.cbAllowRemoteControl->setVisible(b);
}

template<typename UI>
bool ConnectionDialog<UI>::allowRemoteControl()
{
    return m_ui.cbAllowRemoteControl->isChecked();
}

//*********

class InvitationsConnectionDialog : public ConnectionDialog<Ui::ConnectionWidget>
{
    Q_OBJECT
public:
    explicit InvitationsConnectionDialog(QWidget *parent);
    void setRemoteHost(const QString &host);
};

//*********

#endif // CONNECTIONDIALOG_H
