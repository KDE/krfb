/* This file is part of the KDE project
   Copyright (C) 2009 Collabora Ltd <info@collabora.co.uk>
    @author George Goldberg <george.goldberg@collabora.co.uk>
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
   Copyright (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "krfbconnectioncontroller.h"

#include <QX11Info>
#include <QHostInfo>
#include <QApplication>
#include <QDesktopWidget>
#include <QTcpSocket>
#include <QTimer>

#include <KConfig>
#include <KGlobal>
#include <KUser>
#include <KNotification>
#include <KLocale>
#include <KDebug>


#include "invitationmanager.h"
#include "connectiondialog.h"
#include "events.h"
#include "krfbserver.h"
#include "sockethelpers.h"

#include "krfbconfig.h"

#include <X11/Xutil.h>

static bool checkPassword(const QString &p, unsigned char *ochallenge, const char *response, int len)
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

KrfbConnectionController::KrfbConnectionController(struct _rfbClientRec *_cl,
                                                   AbstractRfbServer * parent)
  : AbstractConnectionController(_cl, parent),
    m_clientGoneRequiresAction(false)
{
    kDebug();
}

KrfbConnectionController::~KrfbConnectionController()
{
    kDebug();
}

enum rfbNewClientAction KrfbConnectionController::handleNewClient()
{
    kDebug();

    bool askOnConnect = KrfbConfig::askOnConnect();
    bool allowUninvited = KrfbConfig::allowUninvitedConnections();

    remoteIp = peerAddress(cl->sock);

    if (!allowUninvited && InvitationManager::self()->activeInvitations() == 0) {
        KNotification::event("UnexpectedConnection",
                             i18n("Refused uninvited connection attempt from %1",
                                  remoteIp));
        return RFB_CLIENT_REFUSE;
    }

    // In the remaining cases, the connection will be at least partially established, so we need
    // the clientGoneHook to be called when the connection ends.
    m_clientGoneRequiresAction = true;

    if (!askOnConnect) {
        KNotification::event("NewConnectionAutoAccepted",
                             i18n("Accepted uninvited connection from %1",
                                  remoteIp));

        emit sessionEstablished(remoteIp);
        return RFB_CLIENT_ACCEPT;
    }

    KNotification::event("NewConnectionOnHold",
                         i18n("Received connection from %1, on hold (waiting for confirmation)",
                              remoteIp));

    ConnectionDialog *dialog = new ConnectionDialog(0);
    dialog->setRemoteHost(remoteIp);
    dialog->setAllowRemoteControl(KrfbConfig::allowDesktopControl());

    connect(dialog, SIGNAL(okClicked()), SLOT(dialogAccepted()));
    connect(dialog, SIGNAL(cancelClicked()), SLOT(dialogRejected()));

    dialog->show();

    return RFB_CLIENT_ON_HOLD;
}

bool KrfbConnectionController::handleCheckPassword(rfbClientPtr cl, const char *response, int len)
{
    bool allowUninvited = KrfbConfig::allowUninvitedConnections();
    QString password =  KrfbConfig::uninvitedConnectionPassword();

    bool authd = false;
    kDebug() << "about to start autentication";

    if (allowUninvited) {
        authd = checkPassword(password, cl->authChallenge, response, len);
    }

    if (!authd) {
        QList<Invitation> invlist = InvitationManager::self()->invitations();

        foreach(const Invitation &it, invlist) {
            kDebug() << "checking password";
            if (checkPassword(it.password(), cl->authChallenge, response, len) && it.isValid()) {
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
                                  remoteIp));
        } else {
            KNotification::event("InvalidPassword",
                             i18n("Failed login attempt from %1: wrong password",
                                  remoteIp));
        }
        return false;
    }


    return true;
}

void KrfbConnectionController::handleClientGone()
{
    if (m_clientGoneRequiresAction) {
        emit clientDisconnected(this);
        kDebug() << "client gone";
        deleteLater();
    }
}


#include "krfbconnectioncontroller.moc"

