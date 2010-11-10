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
#include "invitationsrfbserver.h"
#include "krfbconfig.h"
#include "invitationmanager.h"
#include "rfbservermanager.h"
#include <QtCore/QTimer>
#include <QtNetwork/QHostInfo>
#include <KNotification>
#include <KLocale>
#include <KMessageBox>
#include <KUser>
#include <DNSSD/PublicService>

//************

class InvitationsRfbClient : public RfbClient
{
public:
    InvitationsRfbClient(rfbClientPtr client, QObject* parent = 0)
        : RfbClient(client, parent) {}

    virtual rfbNewClientAction doHandle();
};

rfbNewClientAction InvitationsRfbClient::doHandle()
{
    bool allowUninvited = KrfbConfig::allowUninvitedConnections();

    if (!allowUninvited && InvitationManager::self()->activeInvitations() == 0) {
        KNotification::event("UnexpectedConnection",
                             i18n("Refused uninvited connection attempt from %1", name()));
        return RFB_CLIENT_REFUSE;
    }

    return RfbClient::doHandle();
}

//***********

static bool doCheckPassword(const QString &p, unsigned char *ochallenge, const char *response, int len)
{
    if ((len == 0) && (p.length() == 0)) {
        return true;
    }

    char passwd[MAXPWLEN];
    unsigned char challenge[CHALLENGESIZE];

    memcpy(challenge, ochallenge, CHALLENGESIZE);
    bzero(passwd, MAXPWLEN);

    if (!p.isNull()) {
        strncpy(passwd, p.toLatin1(),
                (MAXPWLEN <= p.length()) ? MAXPWLEN : p.length());
    }

    rfbEncryptBytes(challenge, passwd);
    return memcmp(challenge, response, len) == 0;
}

//***********


void InvitationsRfbServer::init()
{
    InvitationsRfbServer *server;
    server = new InvitationsRfbServer;
    server->setListeningPort(KrfbConfig::port());
    server->setListeningAddress("0.0.0.0");  // Listen on all available network addresses
    server->setPasswordRequired(true);
    QTimer::singleShot(0, server, SLOT(startAndCheck()));
}

RfbClient* InvitationsRfbServer::newClient(rfbClientPtr client)
{
    return new InvitationsRfbClient(client, this);
}

bool InvitationsRfbServer::checkPassword(RfbClient* client, const char* encryptedPassword, int len)
{
    bool allowUninvited = KrfbConfig::allowUninvitedConnections();
    QString password =  KrfbConfig::uninvitedConnectionPassword();

    bool authd = false;
    kDebug() << "about to start autentication";

    if (allowUninvited) {
        authd = doCheckPassword(password, client->rfbClient()->authChallenge, encryptedPassword, len);
    }

    if (!authd) {
        QList<Invitation> invlist = InvitationManager::self()->invitations();

        foreach(const Invitation & it, invlist) {
            kDebug() << "checking password";

            if (doCheckPassword(it.password(), client->rfbClient()->authChallenge,
                                encryptedPassword, len)
                && it.isValid())
            {
                authd = true;
                InvitationManager::self()->removeInvitation(it);
                break;
            }
        }
    }

    if (!authd) {
        if (InvitationManager::self()->invitations().size() > 0) {
            KNotification::event("InvalidPasswordInvitations",
                                 i18n("Failed login attempt from %1: wrong password",
                                      client->name()));
        } else {
            KNotification::event("InvalidPassword",
                                 i18n("Failed login attempt from %1: wrong password",
                                      client->name()));
        }

        return false;
    }

    return true;
}

void InvitationsRfbServer::startAndCheck()
{
    if (!start()) {
        KMessageBox::error(0, i18n("Failed to start the krfb server. Invitation-based sharing "
                                   "will not work. Try setting another port in the settings and "
                                   "restart krfb."));
    } else {
        //publish service
        if (KrfbConfig::publishService()) {
            DNSSD::PublicService *service = new DNSSD::PublicService(i18n("%1@%2 (shared desktop)",
                    KUser().loginName(),
                    QHostInfo::localHostName()),
                    "_rfb._tcp",
                    listeningPort());
            service->publishAsync();
        }

        //disconnect when qApp quits
        connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(stop()));
    }
}

#include "invitationsrfbserver.moc"
