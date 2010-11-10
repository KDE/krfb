/*
    Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "pendingrfbclient.h"
#include "connectiondialog.h"
#include "krfbconfig.h"
#include <KDebug>
#include <KNotification>

PendingRfbClient::PendingRfbClient(rfbClientPtr client, QObject *parent)
    : QObject(parent), m_client(0), m_rfbClient(client)
{
    QMetaObject::invokeMethod(this, "processNewClient", Qt::QueuedConnection);
}

void PendingRfbClient::accept()
{
    kDebug() << "accepted connection";

    Q_ASSERT(m_client);
    m_client->setOnHold(false);
    Q_EMIT finished(m_client);
    deleteLater();
}

void PendingRfbClient::reject()
{
    kDebug() << "refused connection";

    rfbCloseClient(m_rfbClient);
    rfbClientConnectionGone(m_rfbClient);
    Q_EMIT finished(NULL);
    deleteLater();
}

void PendingRfbClient::showUserConfirmationDialog()
{
    kDebug();
    Q_ASSERT(m_client);

    if (!KrfbConfig::askOnConnect()) {
        KNotification::event("NewConnectionAutoAccepted",
                             i18n("Accepted connection from %1", m_client->name()));
        accept();
    } else {
        KNotification::event("NewConnectionOnHold",
                            i18n("Received connection from %1, on hold (waiting for confirmation)",
                                m_client->name()));

        ConnectionDialog *dialog = new ConnectionDialog(0);
        dialog->setRemoteHost(m_client->name());
        dialog->setAllowRemoteControl(KrfbConfig::allowDesktopControl());

        connect(dialog, SIGNAL(okClicked()), SLOT(dialogAccepted()));
        connect(dialog, SIGNAL(cancelClicked()), SLOT(dialogRejected()));

        dialog->show();
    }
}

void PendingRfbClient::dialogAccepted()
{
    ConnectionDialog *dialog = qobject_cast<ConnectionDialog *>(sender());
    Q_ASSERT(dialog);
    m_client->setControlEnabled(dialog->cbAllowRemoteControl->isChecked());
    accept();
}

void PendingRfbClient::dialogRejected()
{
    reject();
}
