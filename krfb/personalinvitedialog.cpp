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

#include "personalinvitedialog.h"

#include "krfbconfig.h"

#include <KIconLoader>
#include <KLocale>

#include <QtGui/QLabel>
#include <QtGui/QToolTip>

#include <QtNetwork/QNetworkInterface>

PersonalInviteDialog::PersonalInviteDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("Personal Invitation"));
    setButtons(Close);
    setDefaultButton(Close);
    setModal(true);

    setMinimumSize(500, 250);

    int port = KrfbConfig::port();

    m_inviteWidget = new QWidget(this);
    setupUi(m_inviteWidget);
    pixmapLabel->setPixmap(KIcon("krfb").pixmap(128));

    QList<QNetworkInterface> ifl = QNetworkInterface::allInterfaces();

    foreach(const QNetworkInterface & nif, ifl) {
        if (nif.flags() & QNetworkInterface::IsLoopBack) {
            continue;
        }

        if (nif.flags() & QNetworkInterface::IsRunning && !nif.addressEntries().isEmpty()) {
            hostLabel->setText(QString("%1:%2").arg(nif.addressEntries().first().ip().toString()).arg(port));
        }
    }

    connect(mainTextLabel, SIGNAL(linkActivated(QString)),
            SLOT(showWhatsthis(QString)));

    connect(hostHelpLabel, SIGNAL(linkActivated(QString)),
            SLOT(showWhatsthis(QString)));

    setMainWidget(m_inviteWidget);
}


void PersonalInviteDialog::setHost(const QString &host, uint port)
{
    hostLabel->setText(QString("%1:%2")
                       .arg(host).arg(port));
}

void PersonalInviteDialog::setPassword(const QString &passwd)
{
    passwordLabel->setText(passwd);
}

void PersonalInviteDialog::setExpiration(const QDateTime &expire)
{
    expirationLabel->setText(expire.toString(Qt::LocalDate));
}

void PersonalInviteDialog::showWhatsthis(const QString &link)
{
    if (link == "htc") {
        QToolTip::showText(QCursor::pos(),
                           i18n("Desktop Sharing uses the VNC protocol. You can use any VNC client to connect. \n"
                                "In KDE the client is called 'Remote Desktop Connection'. Enter the host information\n"
                                "into the client and it will connect.."));
    } else if (link == "help") {
        QToolTip::showText(QCursor::pos(),
                           i18n("This field contains the address of your computer and the display number, separated by a colon.\n"
                                "The address is just a hint - you can use any address that can reach your computer. \n"
                                "Desktop Sharing tries to guess your address from your network configuration, but does\n"
                                "not always succeed in doing so. If your computer is behind a firewall it may have a\n"
                                "different address or be unreachable for other computers."));
    }
}


#include "personalinvitedialog.moc"

