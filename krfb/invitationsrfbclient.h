/*
    Copyright (C) 2010 Collabora Ltd <info@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>

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
#ifndef INVITATIONSRFBCLIENT_H
#define INVITATIONSRFBCLIENT_H

#include "rfbclient.h"

class InvitationsRfbClient : public RfbClient
{
public:
    explicit InvitationsRfbClient(rfbClientPtr client, QObject* parent = nullptr)
        : RfbClient(client, parent) {}
};


class PendingInvitationsRfbClient : public PendingRfbClient
{
    Q_OBJECT
public:
    explicit PendingInvitationsRfbClient(rfbClientPtr client, QObject *parent = nullptr);
    ~PendingInvitationsRfbClient() override;

protected Q_SLOTS:
    void processNewClient() override;
    virtual void onSocketActivated();
    bool checkPassword(const QByteArray & encryptedPassword) override;

private Q_SLOTS:
    void dialogAccepted();

private:
    struct Private;
    Private* const d;
};

#endif // INVITATIONSRFBCLIENT_H
