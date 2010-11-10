/*
    Copyright (C) 2009-2010 Collabora Ltd <info@collabora.co.uk>
      @author George Goldberg <george.goldberg@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
    Copyright (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "invitationsrfbclient.h"
#include "invitationmanager.h"
#include "krfbconfig.h"
#include "sockethelpers.h"
#include "connectiondialog.h"
#include <KNotification>
#include <KLocale>

bool InvitationsRfbClient::checkPassword(const QByteArray & encryptedPassword)
{
    bool allowUninvited = KrfbConfig::allowUninvitedConnections();
    QByteArray password =  KrfbConfig::uninvitedConnectionPassword().toLocal8Bit();

    bool authd = false;
    kDebug() << "about to start autentication";

    if (allowUninvited) {
        authd = vncAuthCheckPassword(password, encryptedPassword);
    }

    if (!authd) {
        QList<Invitation> invlist = InvitationManager::self()->invitations();

        foreach(const Invitation & it, invlist) {
            kDebug() << "checking password";

            if (vncAuthCheckPassword(it.password().toLocal8Bit(), encryptedPassword)
                && it.isValid())
            {
                authd = true;
                InvitationManager::self()->removeInvitation(it);
                break;
            }
        }
    }

    if (!authd) {
        if (InvitationManager::self()->invitations().size() > 0) {
            KNotification::event("InvalidPasswordInvitations",
                                 i18n("Failed login attempt from %1: wrong password", name()));
        } else {
            KNotification::event("InvalidPassword",
                                 i18n("Failed login attempt from %1: wrong password", name()));
        }

        return false;
    }

    return true;
}


void PendingInvitationsRfbClient::processNewClient()
{
    QString host = peerAddress(m_rfbClient->sock) + ":" + QString::number(peerPort(m_rfbClient->sock));
    bool allowUninvited = KrfbConfig::allowUninvitedConnections();

    if (!allowUninvited && InvitationManager::self()->activeInvitations() == 0) {
        KNotification::event("UnexpectedConnection",
                             i18n("Refused uninvited connection attempt from %1", host));
        reject();
    } else if (!KrfbConfig::askOnConnect()) {
        KNotification::event("NewConnectionAutoAccepted",
                             i18n("Accepted connection from %1", host));
        accept(new InvitationsRfbClient(m_rfbClient, parent()));
    } else {
        KNotification::event("NewConnectionOnHold",
                            i18n("Received connection from %1, on hold (waiting for confirmation)",
                                host));

        InvitationsConnectionDialog *dialog = new InvitationsConnectionDialog(0);
        dialog->setRemoteHost(host);
        dialog->setAllowRemoteControl(KrfbConfig::allowDesktopControl());

        connect(dialog, SIGNAL(okClicked()), SLOT(dialogAccepted()));
        connect(dialog, SIGNAL(cancelClicked()), SLOT(reject()));

        dialog->show();
    }
}

void PendingInvitationsRfbClient::dialogAccepted()
{
    InvitationsConnectionDialog *dialog = qobject_cast<InvitationsConnectionDialog *>(sender());
    Q_ASSERT(dialog);

    InvitationsRfbClient *client = new InvitationsRfbClient(m_rfbClient, parent());
    client->setControlEnabled(dialog->allowRemoteControl());
    accept(client);
}

#include "invitationsrfbclient.moc"
