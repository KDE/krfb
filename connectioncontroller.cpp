/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
             (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "connectioncontroller.h"
#include "connectioncontroller.moc"

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

#include "krfbconfig.h"

#include <X11/Xutil.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


static QString peerAddress(int sock) {

    const int ADDR_SIZE = 50;
    struct sockaddr sa;
    socklen_t salen = sizeof(struct sockaddr);
    if (getpeername(sock, &sa, &salen) == 0) {
        if (sa.sa_family == AF_INET) {
            struct sockaddr_in *si = (struct sockaddr_in *)&sa;
            return QString(inet_ntoa(si->sin_addr));
        }
        if (sa.sa_family == AF_INET6) {
            char inetbuf[ADDR_SIZE];
            inet_ntop(sa.sa_family, &sa, inetbuf, ADDR_SIZE);
            return QString(inetbuf);
        }
        return QString("not a network address");
    }
    return QString("unable to determine...");
}

static void clientGoneHook(rfbClientPtr cl)
{
    ConnectionController *cc = static_cast<ConnectionController *>(cl->clientData);
    cc->handleClientGone();
}


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


ConnectionController::ConnectionController(struct _rfbClientRec *_cl, KrfbServer * parent)
    : QObject(parent), cl(_cl)
{
    cl->clientData = (void*)this;
}

ConnectionController::~ConnectionController()
{
}

enum rfbNewClientAction ConnectionController::handleNewClient()
{

    bool askOnConnect = KrfbConfig::askOnConnect();
    bool allowUninvited = KrfbConfig::allowUninvitedConnections();

    remoteIp = peerAddress(cl->sock);

    if (!allowUninvited && InvitationManager::self()->activeInvitations() == 0) {
        KNotification::event("ConnectionAttempted",
                             i18n("Refused uninvited connection attempt from %1",
                                  remoteIp));
        return RFB_CLIENT_REFUSE;
    }

    if (!askOnConnect && InvitationManager::self()->activeInvitations() == 0) {
        KNotification::event("NewConnectionAutoAccepted",
                             i18n("Accepted uninvited connection from %1",
                                  remoteIp));

        emit sessionEstablished(remoteIp);
        return RFB_CLIENT_ACCEPT;
    }

    KNotification::event("NewConnectionOnHold",
                         i18n("Received connection from %1, on hold (waiting for confirmation)",
                              remoteIp));

    cl->clientGoneHook = clientGoneHook;

    ConnectionDialog *dialog = new ConnectionDialog(0);
    dialog->setRemoteHost(remoteIp);
    dialog->setAllowRemoteControl( true );

    connect(dialog, SIGNAL(okClicked()), SLOT(dialogAccepted()));
    connect(dialog, SIGNAL(cancelClicked()), SLOT(dialogRejected()));

    dialog->show();

    return RFB_CLIENT_ON_HOLD;
}

bool ConnectionController::handleCheckPassword(rfbClientPtr cl, const char *response, int len)
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


void ConnectionController::handleKeyEvent(bool down, rfbKeySym keySym)
{
    if (controlEnabled) {
        KeyboardEvent ev(down, keySym);
        ev.exec();
    }
}

void ConnectionController::handlePointerEvent(int bm, int x, int y)
{
    if (controlEnabled) {
        PointerEvent ev(bm, x, y);
        ev.exec();
    }
}

void ConnectionController::handleClientGone()
{
    emit clientDisconnected(this);
    kDebug() << "client gone";
    deleteLater();
}

void ConnectionController::clipboardToServer(const QString &s)
{
    ClipboardEvent ev(this, s);
    ev.exec();
}

void ConnectionController::dialogAccepted()
{
    // rfbStartOnHoldClient(cl);
    cl->onHold = false;
}

void ConnectionController::dialogRejected()
{
    kDebug() << "refused connection";
    rfbRefuseOnHoldClient(cl);
}

void ConnectionController::setControlEnabled(bool enable)
{
    controlEnabled = enable;
}


