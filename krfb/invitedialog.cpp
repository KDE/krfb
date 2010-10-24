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

#include "invitedialog.h"

#include <KIconLoader>
#include <KLocale>
#include <KStandardGuiItem>

#include <QtGui/QCursor>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QToolTip>

InviteDialog::InviteDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("Invitation"));
    setButtons(User1 | Close | Help);
    setHelp(QString(), "krfb");
    setDefaultButton(NoDefault);

    setMinimumSize(500, 300);

    m_inviteWidget = new QWidget(this);
    setupUi(m_inviteWidget);

    pixmapLabel->setPixmap(KIcon("krfb").pixmap(128));
    setMainWidget(m_inviteWidget);

    setButtonGuiItem(User1, KStandardGuiItem::configure());

    connect(btnCreateInvite, SIGNAL(clicked()),
            SIGNAL(createInviteClicked()));
    connect(btnEmailInvite, SIGNAL(clicked()),
            SIGNAL(emailInviteClicked()));
    connect(btnManageInvite, SIGNAL(clicked()),
            SIGNAL(manageInviteClicked()));
    connect(helpLabel, SIGNAL(linkActivated(QString)),
            SLOT(showWhatsthis()));
}

void InviteDialog::slotUser1()
{
    emit configureClicked();
}

void InviteDialog::enableInviteButton(bool enable)
{
    btnCreateInvite->setEnabled(enable);
}

void InviteDialog::setInviteCount(int count)
{
    btnManageInvite->setText(
        i18n("&Manage Invitations (%1)...", count));
}

void InviteDialog::showWhatsthis()
{
    QToolTip::showText(QCursor::pos(),
                       i18n("An invitation creates a one-time password that allows the receiver to connect to your desktop.\n"
                            "It is valid for only one successful connection and will expire after an hour if it has not been used. \n"
                            "When somebody connects to your computer a dialog will appear and ask you for permission.\n "
                            "The connection will not be established before you accept it. In this dialog you can also\n restrict "
                            "the other person to view your desktop only, without the ability to move your\n mouse pointer or press "
                            "keys.\nIf you want to create a permanent password for Desktop Sharing, allow 'Uninvited Connections' \n"
                            "in the configuration."));
}

#include "invitedialog.moc"
