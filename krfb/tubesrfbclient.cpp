/*
    Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
      @author George Goldberg <george.goldberg@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "tubesrfbclient.h"
#include "krfbconfig.h"
#include "connectiondialog.h"
#include <KNotification>
#include <KLocale>

QString TubesRfbClient::name() const
{
    return m_contact->alias();
}


void PendingTubesRfbClient::setContact(const Tp::ContactPtr & contact)
{
    m_contact = contact;
    if (m_processNewClientCalled) {
        //processNewClient has already been called, so we need to act here
        showConfirmationDialog();
    }
}

void PendingTubesRfbClient::processNewClient()
{
    if (!m_contact) {
        //no associated contact yet, hold.
        m_processNewClientCalled = true; //act when a contact is set
    } else {
        //we have a contact, begin handling
        showConfirmationDialog();
    }
}

void PendingTubesRfbClient::showConfirmationDialog()
{
    QString name = m_contact->alias();

    if (!KrfbConfig::askOnConnect()) {
        KNotification::event("NewConnectionAutoAccepted",
                             i18n("Accepted connection from %1", name));
        accept(new TubesRfbClient(m_rfbClient, m_contact, parent()));
    } else {
        KNotification::event("NewConnectionOnHold",
                            i18n("Received connection from %1, on hold (waiting for confirmation)",
                                name));

        TubesConnectionDialog *dialog = new TubesConnectionDialog(0);
        dialog->setContactName(name);
        dialog->setAllowRemoteControl(KrfbConfig::allowDesktopControl());

        connect(dialog, SIGNAL(okClicked()), SLOT(dialogAccepted()));
        connect(dialog, SIGNAL(cancelClicked()), SLOT(reject()));

        dialog->show();
    }
}

void PendingTubesRfbClient::dialogAccepted()
{
    TubesConnectionDialog *dialog = qobject_cast<TubesConnectionDialog *>(sender());
    Q_ASSERT(dialog);

    TubesRfbClient *client = new TubesRfbClient(m_rfbClient, m_contact, parent());
    client->setControlEnabled(dialog->allowRemoteControl());
    accept(client);
}

#include "tubesrfbclient.moc"
