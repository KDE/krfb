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
#include "rfb.h"
#include "invitationsrfbclient.h"
#include "invitationsrfbserver.h"
#include "krfbconfig.h"
#include "sockethelpers.h"
#include "connectiondialog.h"
#include "krfbdebug.h"

#include <KNotification>
#include <KLocalizedString>

#include <QSocketNotifier>
#include <poll.h>
#include <KConfigGroup>

struct PendingInvitationsRfbClient::Private
{
    Private(rfbClientPtr client) :
        client(client),
        askOnConnect(true)
    {}

    rfbClientPtr client;
    QSocketNotifier *notifier = nullptr;
    bool askOnConnect;
};

static void clientGoneHookNoop(rfbClientPtr cl) { Q_UNUSED(cl); }

PendingInvitationsRfbClient::PendingInvitationsRfbClient(rfbClientPtr client, QObject *parent) :
    PendingRfbClient(client, parent),
    d(new Private(client))
{
    d->client->clientGoneHook = clientGoneHookNoop;
}

PendingInvitationsRfbClient::~PendingInvitationsRfbClient()
{
    delete d;
}

void PendingInvitationsRfbClient::processNewClient()
{
    QString host = peerAddress(m_rfbClient->sock) + QLatin1Char(':') + QString::number(peerPort(m_rfbClient->sock));

    if (d->askOnConnect == false) {

        KNotification::event(QStringLiteral("NewConnectionAutoAccepted"),
                             i18n("Accepted connection from %1", host));
        accept(new InvitationsRfbClient(m_rfbClient, parent()));

    } else {

        KNotification::event(QStringLiteral("NewConnectionOnHold"),
                            i18n("Received connection from %1, on hold (waiting for confirmation)",
                                host));

        auto dialog = new InvitationsConnectionDialog(nullptr);
        dialog->setRemoteHost(host);
        dialog->setAllowRemoteControl(KrfbConfig::allowDesktopControl());

        connect(dialog, &InvitationsConnectionDialog::accepted, this, &PendingInvitationsRfbClient::dialogAccepted);
        connect(dialog, &InvitationsConnectionDialog::rejected, this, &PendingInvitationsRfbClient::reject);

        dialog->show();
    }
}

bool PendingInvitationsRfbClient::checkPassword(const QByteArray & encryptedPassword)
{
    qCDebug(KRFB) << "about to start authentication";

    if(InvitationsRfbServer::instance->allowUnattendedAccess() && vncAuthCheckPassword(
            InvitationsRfbServer::instance->unattendedPassword().toLocal8Bit(),
            encryptedPassword) ) {
        d->askOnConnect = false;
        return true;
    }

    return vncAuthCheckPassword(
            InvitationsRfbServer::instance->desktopPassword().toLocal8Bit(),
            encryptedPassword);
}

void PendingInvitationsRfbClient::dialogAccepted()
{
    auto dialog = qobject_cast<InvitationsConnectionDialog *>(sender());
    Q_ASSERT(dialog);

    auto client = new InvitationsRfbClient(m_rfbClient, parent());
    client->setControlEnabled(dialog->allowRemoteControl());
    accept(client);
}
