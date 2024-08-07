/*
    SPDX-FileCopyrightText: 2010 Collabora Ltd <info@collabora.co.uk>
    SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    bool checkPassword(const QByteArray & encryptedPassword) override;

private Q_SLOTS:
    void dialogAccepted();

private:
    struct Private;
    Private* const d;
};

#endif // INVITATIONSRFBCLIENT_H
