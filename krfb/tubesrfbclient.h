/*
    Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
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
#ifndef TUBESRFBCLIENT_H
#define TUBESRFBCLIENT_H

#include "rfbclient.h"
#include <TelepathyQt/Contact>

class TubesRfbClient : public RfbClient
{
public:
    TubesRfbClient(rfbClientPtr client, const Tp::ContactPtr & contact, QObject* parent = 0)
        : RfbClient(client, parent), m_contact(contact) {}

    virtual QString name() const;

private:
    Tp::ContactPtr m_contact;
};


class PendingTubesRfbClient : public PendingRfbClient
{
    Q_OBJECT
public:
    PendingTubesRfbClient(rfbClientPtr client, QObject* parent = 0)
        : PendingRfbClient(client, parent), m_processNewClientCalled(false) {}

    void setContact(const Tp::ContactPtr & contact);

protected Q_SLOTS:
    virtual void processNewClient();

private Q_SLOTS:
    void showConfirmationDialog();
    void dialogAccepted();

private:
    Tp::ContactPtr m_contact;
    bool m_processNewClientCalled;
};

#endif // TUBESRFBCLIENT_H
