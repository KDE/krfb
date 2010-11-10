/*
    Copyright (C) 2009-2010 Collabora Ltd <info@collabora.co.uk>
      @author George Goldberg <george.goldberg@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

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
#include "rfbclient.h"
#include "connectiondialog.h"
#include "krfbconfig.h"
#include "sockethelpers.h"
#include "events.h"
#include <QtCore/QSocketNotifier>
#include <KDebug>
#include <KNotification>
#include <poll.h>

struct RfbClient::Private
{
    Private(rfbClientPtr client) :
        connected(false),
        controlEnabled(false),
        client(client)
    {}

    bool connected;
    bool controlEnabled;
    rfbClientPtr client;
    QSocketNotifier *notifier;
};

RfbClient::RfbClient(rfbClientPtr client, QObject* parent)
    : QObject(parent), d(new Private(client))
{
    d->notifier = new QSocketNotifier(client->sock, QSocketNotifier::Read, this);
    d->notifier->setEnabled(false);
    connect(d->notifier, SIGNAL(activated(int)), this, SLOT(onSocketActivated()));
}

RfbClient::~RfbClient()
{
    kDebug();
    delete d;
}

QString RfbClient::name() const
{
    return peerAddress(d->client->sock) + ":" + QString::number(peerPort(d->client->sock));
}

//static
bool RfbClient::controlCanBeEnabled()
{
    return KrfbConfig::allowDesktopControl();
}

bool RfbClient::controlEnabled() const
{
    return d->controlEnabled;
}

void RfbClient::setControlEnabled(bool enabled)
{
    if (controlCanBeEnabled() && d->controlEnabled != enabled) {
        d->controlEnabled = enabled;
        Q_EMIT controlEnabledChanged(enabled);
    }
}

void RfbClient::update()
{
    rfbUpdateClient(d->client);

    //This is how we handle disconnection.
    //if rfbUpdateClient() finds out that it can't write to the socket,
    //it closes it and sets it to -1. So, we just have to check this here
    //and call rfbClientConnectionGone() if necessary. This will call
    //the clientGoneHook which in turn will remove this RfbClient instance
    //from the server manager and will call deleteLater() to delete it
    if (d->client->sock == -1) {
        kDebug() << "disconnected during update";
        d->notifier->setEnabled(false);
        rfbClientConnectionGone(d->client);
    }
}

void RfbClient::setOnHold(bool onHold)
{
    d->client->onHold = onHold;
    d->notifier->setEnabled(!onHold);
}

void RfbClient::closeConnection()
{
    d->notifier->setEnabled(false);
    rfbCloseClient(d->client);
    rfbClientConnectionGone(d->client);
}

rfbNewClientAction RfbClient::doHandle()
{
    kDebug();

    if (!KrfbConfig::askOnConnect()) {
        KNotification::event("NewConnectionAutoAccepted",
                             i18n("Accepted connection from %1", name()));

        setStatusConnected();
        return RFB_CLIENT_ACCEPT;
    }

    KNotification::event("NewConnectionOnHold",
                         i18n("Received connection from %1, on hold (waiting for confirmation)",
                              name()));

    ConnectionDialog *dialog = new ConnectionDialog(0);
    dialog->setRemoteHost(name());
    dialog->setAllowRemoteControl(KrfbConfig::allowDesktopControl());

    connect(dialog, SIGNAL(okClicked()), SLOT(dialogAccepted()));
    connect(dialog, SIGNAL(cancelClicked()), SLOT(dialogRejected()));

    dialog->show();

    return RFB_CLIENT_ON_HOLD;
}

bool RfbClient::isConnected() const
{
    return d->connected;
}

void RfbClient::setStatusConnected()
{
    if (!d->connected) {
        d->connected = true;
        Q_EMIT connected(this);
    }
}

void RfbClient::handleKeyboardEvent(bool down, rfbKeySym keySym)
{
    if (d->controlEnabled) {
        EventHandler::handleKeyboard(down, keySym);
    }
}

void RfbClient::handleMouseEvent(int buttonMask, int x, int y)
{
    if (d->controlEnabled) {
        EventHandler::handlePointer(buttonMask, x, y);
    }
}

bool RfbClient::checkPassword(const QByteArray & encryptedPassword)
{
    Q_UNUSED(encryptedPassword);

    return d->client->screen->authPasswdData == (void*)0;
}

bool RfbClient::vncAuthCheckPassword(const QByteArray& password, const QByteArray& encryptedPassword) const
{
    if (password.isEmpty() && encryptedPassword.isEmpty()) {
        return true;
    }

    char passwd[MAXPWLEN];
    unsigned char challenge[CHALLENGESIZE];

    memcpy(challenge, d->client->authChallenge, CHALLENGESIZE);
    bzero(passwd, MAXPWLEN);

    if (!password.isEmpty()) {
        strncpy(passwd, password,
                (MAXPWLEN <= password.size()) ? MAXPWLEN : password.size());
    }

    rfbEncryptBytes(challenge, passwd);
    return memcmp(challenge, encryptedPassword, encryptedPassword.size()) == 0;
}

void RfbClient::onSocketActivated()
{
    //Process not only one, but all pending messages.
    //poll() idea/code copied from vino:
    // Copyright (C) 2003 Sun Microsystems, Inc.
    // License: GPL v2 or later
    struct pollfd pollfd = { d->client->sock, POLLIN|POLLPRI, 0 };

    while(poll(&pollfd, 1, 0) == 1) {
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

void RfbClient::dialogAccepted()
{
    ConnectionDialog *dialog = qobject_cast<ConnectionDialog *>(sender());

    if (!dialog) {
        kWarning() << "Wrong type of sender.";
        return;
    }

    setOnHold(false);
    setControlEnabled(dialog->cbAllowRemoteControl->isChecked());
    setStatusConnected();
}

void RfbClient::dialogRejected()
{
    kDebug() << "refused connection";
    closeConnection();
}

#include "rfbclient.moc"
