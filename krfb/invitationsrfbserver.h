/*
    SPDX-FileCopyrightText: 2009-2010 Collabora Ltd <info@collabora.co.uk>
    SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>
    SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef INVITATIONSRFBSERVER_H
#define INVITATIONSRFBSERVER_H

#include "rfbserver.h"

namespace KWallet {
    class Wallet;
}

namespace KDNSSD {
    class PublicService;
}

class InvitationsRfbServer : public RfbServer
{
    Q_OBJECT
public:
    static InvitationsRfbServer *instance;
    static void init();

    const QString& desktopPassword() const;
    void setDesktopPassword(const QString&);
    const QString& unattendedPassword() const;
    void setUnattendedPassword(const QString&);
    bool allowUnattendedAccess() const;

Q_SIGNALS:
    void passwordChanged(const QString&);

public Q_SLOTS:
    bool start() override;
    void stop() override;
    void toggleUnattendedAccess(bool allow);
    void openKWallet();
    void closeKWallet();
    void saveSecuritySettings();

protected:
    InvitationsRfbServer();
    ~InvitationsRfbServer() override;
    PendingRfbClient* newClient(rfbClientPtr client) override;

private Q_SLOTS:
    void walletOpened(bool);

private:
    KDNSSD::PublicService *m_publicService = nullptr;
    bool m_allowUnattendedAccess;
    QString m_desktopPassword;
    QString m_unattendedPassword;
    KWallet::Wallet *m_wallet = nullptr;

    QString readableRandomString(int);
    void readPasswordFromConfig();
    Q_DISABLE_COPY(InvitationsRfbServer)
};

#endif // INVITATIONSRFBSERVER_H
