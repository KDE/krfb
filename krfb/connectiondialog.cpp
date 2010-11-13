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

#include "connectiondialog.h"

#include <KIconLoader>
#include <KLocale>
#include <KStandardGuiItem>

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>

template <typename UI>
ConnectionDialog<UI>::ConnectionDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("New Connection"));
    setButtons(Ok | Cancel);
    setDefaultButton(Cancel);
    setModal(true);

    setMinimumSize(500, 200);

    m_connectWidget = new QWidget(this);
    m_ui.setupUi(m_connectWidget);

    m_ui.pixmapLabel->setPixmap(KIcon("krfb").pixmap(128));

    KGuiItem accept = KStandardGuiItem::ok();
    accept.setText(i18n("Accept Connection"));
    setButtonGuiItem(Ok, accept);

    KGuiItem refuse = KStandardGuiItem::cancel();
    refuse.setText(i18n("Refuse Connection"));
    setButtonGuiItem(Cancel, refuse);

    setMainWidget(m_connectWidget);
}

//**********

InvitationsConnectionDialog::InvitationsConnectionDialog(QWidget *parent)
    : ConnectionDialog<Ui::ConnectionWidget>(parent)
{
}

void InvitationsConnectionDialog::setRemoteHost(const QString &host)
{
    m_ui.remoteHost->setText(host);
}

//**********

#ifdef KRFB_WITH_TELEPATHY_TUBES

TubesConnectionDialog::TubesConnectionDialog(QWidget *parent)
    : ConnectionDialog<Ui::TubesConnectionWidget>(parent)
{
}

void TubesConnectionDialog::setContactName(const QString & name)
{
    QString txt = i18n("You have requested to share your desktop with %1. If you proceed, "
                       "you will allow the remote user to watch your desktop.", name);
    m_ui.mainTextLabel->setText(txt);
}

#endif // KRFB_WITH_TELEPATHY_TUBES

#include "connectiondialog.moc"
