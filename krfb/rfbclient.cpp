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
#include <KDebug>
#include <KNotification>

RfbClient::RfbClient(rfbClientPtr client, QObject* parent)
    : QObject(parent),
      m_connected(false),
      m_controlEnabled(false),
      m_client(client)
{
}

QString RfbClient::name()
{
    return peerAddress(m_client->sock) + ":" + QString::number(peerPort(m_client->sock));
}

//static
bool RfbClient::controlCanBeEnabled()
{
    return KrfbConfig::allowDesktopControl();
}

bool RfbClient::controlEnabled() const
{
    return m_controlEnabled;
}

void RfbClient::setControlEnabled(bool enabled)
{
    if (controlCanBeEnabled() && m_controlEnabled != enabled) {
        m_controlEnabled = enabled;
        Q_EMIT controlEnabledChanged(enabled);
    }
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

void RfbClient::setStatusConnected()
{
    if (!m_connected) {
        m_connected = true;
        Q_EMIT connected(this);
    }
}

void RfbClient::dialogAccepted()
{
    ConnectionDialog *dialog = qobject_cast<ConnectionDialog *>(sender());

    if (!dialog) {
        kWarning() << "Wrong type of sender.";
        return;
    }

    // rfbStartOnHoldClient(cl);
    m_client->onHold = false;
    setControlEnabled(dialog->cbAllowRemoteControl->isChecked());
    setStatusConnected();
}

void RfbClient::dialogRejected()
{
    kDebug() << "refused connection";
    rfbRefuseOnHoldClient(m_client);
}

#include "rfbclient.moc"
