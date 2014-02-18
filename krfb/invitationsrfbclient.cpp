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
#include <KNotification>
#include <KLocale>
#include <QtCore/QSocketNotifier>
#include <poll.h>

struct PendingInvitationsRfbClient::Private
{
    Private(rfbClientPtr client) :
        client(client),
        askOnConnect(true)
    {}

    rfbClientPtr client;
    QSocketNotifier *notifier;
    bool askOnConnect;
};

static void clientGoneHookNoop(rfbClientPtr cl) { Q_UNUSED(cl); }

PendingInvitationsRfbClient::PendingInvitationsRfbClient(rfbClientPtr client, QObject *parent) :
    PendingRfbClient(client, parent),
    d(new Private(client))
{
    d->client->clientGoneHook = clientGoneHookNoop;
    d->notifier = new QSocketNotifier(client->sock, QSocketNotifier::Read, this);
    d->notifier->setEnabled(true);
    connect(d->notifier, SIGNAL(activated(int)),
            this, SLOT(onSocketActivated()));
}

PendingInvitationsRfbClient::~PendingInvitationsRfbClient()
{
    delete d;
}

void PendingInvitationsRfbClient::processNewClient()
{
    QString host = peerAddress(m_rfbClient->sock) + ":" + QString::number(peerPort(m_rfbClient->sock));

    if (d->askOnConnect == false) {

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

void PendingInvitationsRfbClient::onSocketActivated()
{
    //Process not only one, but all pending messages.
    //poll() idea/code copied from vino:
    // Copyright (C) 2003 Sun Microsystems, Inc.
    // License: GPL v2 or later
    struct pollfd pollfd = { d->client->sock, POLLIN|POLLPRI, 0 };

    while(poll(&pollfd, 1, 0) == 1) {

        if(d->client->state == rfbClientRec::RFB_INITIALISATION) {
            d->notifier->setEnabled(false);
            //Client is Authenticated
            processNewClient();
            break;
        }
        rfbProcessClientMessage(d->client);

        //This is how we handle disconnection.
        //if rfbProcessClientMessage() finds out that it can't read the socket,
        //it closes it and sets it to -1. So, we just have to check this here
        //and call rfbClientConnectionGone() if necessary. This will call
        //the clientGoneHook which in turn will remove this RfbClient instance
        //from the server manager and will call deleteLater() to delete it
        if (d->client->sock == -1) {
            kDebug() << "disconnected from socket signal";
            d->notifier->setEnabled(false);
            rfbClientConnectionGone(d->client);
            break;
        }
    }
}

bool PendingInvitationsRfbClient::checkPassword(const QByteArray & encryptedPassword)
{
    QByteArray password ;
    kDebug() << "about to start autentication";

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
    InvitationsConnectionDialog *dialog = qobject_cast<InvitationsConnectionDialog *>(sender());
    Q_ASSERT(dialog);

    InvitationsRfbClient *client = new InvitationsRfbClient(m_rfbClient, parent());
    client->setControlEnabled(dialog->allowRemoteControl());
    accept(client);
}

#include "invitationsrfbclient.moc"
