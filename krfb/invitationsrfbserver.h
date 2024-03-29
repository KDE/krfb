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
