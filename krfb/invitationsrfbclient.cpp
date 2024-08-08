/*
    SPDX-FileCopyrightText: 2009-2010 Collabora Ltd <info@collabora.co.uk>
    SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>
    SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>
    SPDX-FileCopyrightText: 2001-2003 Tim Jansen <tim@tjansen.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "invitationsrfbclient.h"
#include "connectiondialog.h"
#include "invitationsrfbserver.h"
#include "krfbconfig.h"
#include "krfbdebug.h"
#include "rfb.h"
#include "sockethelpers.h"

#include <KLocalizedString>
#include <KNotification>

#include <KConfigGroup>
#include <QSocketNotifier>
#include <poll.h>

struct PendingInvitationsRfbClient::Private {
    Private(rfbClientPtr client)
        : client(client)
        , askOnConnect(true)
    {
    }

    rfbClientPtr client;
    QSocketNotifier *notifier = nullptr;
    bool askOnConnect;
};

static void clientGoneHookNoop(rfbClientPtr cl)
{
    Q_UNUSED(cl);
}

PendingInvitationsRfbClient::PendingInvitationsRfbClient(rfbClientPtr client, QObject *parent)
    : PendingRfbClient(client, parent)
    , d(new Private(client))
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
        KNotification::event(QStringLiteral("NewConnectionAutoAccepted"), i18n("Accepted connection from %1", host));
        accept(new InvitationsRfbClient(m_rfbClient, parent()));

    } else {
        KNotification::event(QStringLiteral("NewConnectionOnHold"), i18n("Received connection from %1, on hold (waiting for confirmation)", host));

        auto dialog = new InvitationsConnectionDialog(nullptr);
        dialog->setRemoteHost(host);
        dialog->setAllowRemoteControl(KrfbConfig::allowDesktopControl());

        connect(dialog, &InvitationsConnectionDialog::accepted, this, &PendingInvitationsRfbClient::dialogAccepted);
        connect(dialog, &InvitationsConnectionDialog::rejected, this, &PendingInvitationsRfbClient::reject);

        dialog->show();
    }
}

bool PendingInvitationsRfbClient::checkPassword(const QByteArray &encryptedPassword)
{
    qCDebug(KRFB) << "about to start authentication";

    if (InvitationsRfbServer::instance->allowUnattendedAccess()
        && vncAuthCheckPassword(InvitationsRfbServer::instance->unattendedPassword().toLocal8Bit(), encryptedPassword)) {
        d->askOnConnect = false;
        return true;
    }

    return vncAuthCheckPassword(InvitationsRfbServer::instance->desktopPassword().toLocal8Bit(), encryptedPassword);
}

void PendingInvitationsRfbClient::dialogAccepted()
{
    auto dialog = qobject_cast<InvitationsConnectionDialog *>(sender());
    Q_ASSERT(dialog);

    auto client = new InvitationsRfbClient(m_rfbClient, parent());
    client->setControlEnabled(dialog->allowRemoteControl());
    accept(client);
}
