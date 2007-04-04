/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
             (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include "invitationmanager.h"
#include "krfbserver.h"

#include <QThreadStorage>
#include <QX11Info>
#include <QHostInfo>
#include <QApplication>
#include <QDesktopWidget>
#include <QTcpSocket>

#include <KConfig>
#include <KGlobal>
#include <KUser>
#include <KNotification>
#include <KLocale>

#include "connectioncontroller.h"
#include "connectioncontroller.moc"

#include <X11/Xutil.h>

static const char* cur=
        "                   "
        " x                 "
        " xx                "
        " xxx               "
        " xxxx              "
        " xxxxx             "
        " xxxxxx            "
        " xxxxxxx           "
        " xxxxxxxx          "
        " xxxxxxxxx         "
        " xxxxxxxxxx        "
        " xxxxx             "
        " xx xxx            "
        " x  xxx            "
        "     xxx           "
        "     xxx           "
        "      xxx          "
        "      xxx          "
        "                   ";

static const char* mask=
        "xx                 "
        "xxx                "
        "xxxx               "
        "xxxxx              "
        "xxxxxx             "
        "xxxxxxx            "
        "xxxxxxxx           "
        "xxxxxxxxx          "
        "xxxxxxxxxx         "
        "xxxxxxxxxxx        "
        "xxxxxxxxxxxx       "
        "xxxxxxxxxx         "
        "xxxxxxxx           "
        "xxxxxxxx           "
        "xx  xxxxx          "
        "    xxxxx          "
        "     xxxxx         "
        "     xxxxx         "
        "      xxx          ";

static rfbCursorPtr myCursor;

QThreadStorage<CurrentController *> controllers;

static enum rfbNewClientAction newClientHook(struct _rfbClientRec *cl)
{
    return controllers.localData()->handleNewClient(cl);
}

static rfbBool passwordCheck(rfbClientPtr cl,
                             const char* encryptedPassword,
                             int len)
{
    return controllers.localData()->handleCheckPassword(cl, encryptedPassword, len);
}

static void keyboardHook(rfbBool down, rfbKeySym keySym, rfbClientPtr)
{
    controllers.localData()->handleKeyEvent(down ? true : false, keySym);
}

static void pointerHook(int bm, int x, int y, rfbClientPtr)
{
    controllers.localData()->handlePointerEvent(bm, x, y);
}

static void clientGoneHook(rfbClientPtr)
{
    controllers.localData()->handleClientGone();
}

static void clipboardHook(char* str,int len, rfbClientPtr)
{
    controllers.localData()->clipboardToServer(QString::fromUtf8(str, len));
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

CurrentController::CurrentController(int fd)
    :connFD(fd)
{
}

bool CurrentController::handleCheckPassword(rfbClientPtr cl, const char *response, int len)
{
    KSharedConfigPtr conf = KGlobal::config();
    KConfigGroup srvconf(conf, "Server");

    bool allowUninvited = srvconf.readEntry("allowUninvitedConnections",false);
    QString password = srvconf.readEntry("uninvitedConnectionPassword",QString());

    bool authd = false;
    kDebug() << "about to start autentication" << endl;

    if (allowUninvited) {
        authd = checkPassword(password, cl->authChallenge, response, len);
    }

    if (!authd) {
        QList<Invitation> invlist = InvitationManager::self()->invitations();

        foreach(Invitation it, invlist) {
            if (checkPassword(it.password(), cl->authChallenge, response, len) && it.isValid()) {
                authd = true;
                //configuration->removeInvitation(it);
                break;
            }
        }
    }

    if (!authd) {
        if (InvitationManager::self()->invitations().size() > 0) {
            sendKNotifyEvent("InvalidPasswordInvitations",
                             i18n("Failed login attempt from %1: wrong password",
                                  remoteIp));
        } else {
            sendKNotifyEvent("InvalidPassword",
                             i18n("Failed login attempt from %1: wrong password",
                                  remoteIp));
        }
        return false;
    }

#if 0
    asyncMutex.lock();
    asyncQueue.append(new SessionEstablishedEvent(this));
    asyncMutex.unlock();
#endif

    return true;
}


enum rfbNewClientAction CurrentController::handleNewClient(rfbClientPtr cl)
{
    KSharedConfigPtr conf = KGlobal::config();
    KConfigGroup srvconf(conf, "Server");

    bool allowDesktopControl = srvconf.readEntry("allowDesktopControl",false);
    bool askOnConnect = srvconf.readEntry("askOnConnect",false);


    client = cl;
    int socket = cl->sock;
    // cl->negotiationFinishedHook = negotiationFinishedHook; ???

    QString host;

#if 0
    // TODO: this drops the connection >.<
    QTcpSocket t;
    t.setSocketDescriptor(socket); //, QAbstractSocket::ConnectedState, QIODevice::NotOpen);
    host = t.peerAddress().toString();
#endif

    remoteIp = host;

    if (!askOnConnect && InvitationManager::self()->invitations().size() == 0) {
        sendKNotifyEvent("NewConnectionAutoAccepted",
                        i18n("Accepted uninvited connection from %1",
                        remoteIp));

        sendSessionEstablished();
        return RFB_CLIENT_ACCEPT;
    }

    sendKNotifyEvent("NewConnectionOnHold",
                    i18n("Received connection from %1, on hold (waiting for confirmation)",
                    remoteIp));

    //cl->screen->authPasswdData = (void *)1;
    cl->clientGoneHook = clientGoneHook;

//     dialog.setRemoteHost(remoteIp);
//     dialog.setAllowRemoteControl( true );
//     dialog.setFixedSize(dialog.sizeHint());
//     dialog.show();
    return RFB_CLIENT_ON_HOLD;
}

void CurrentController::sendKNotifyEvent(const QString & name, const QString & desc)
{
    kDebug() << "notification: " << name << "   " << desc << endl;
    emit notification(name, desc);
}

void CurrentController::sendSessionEstablished()
{
    emit sessionEstablished("BAH");
}

void CurrentController::handleKeyEvent(bool down, KeySym keySym)
{
}

void CurrentController::handlePointerEvent(int bm, int x, int y)
{
}

void CurrentController::handleClientGone()
{
    kDebug() << "Client gone" << endl;
    rfbCloseClient(client);
}

void CurrentController::clipboardToServer(const QString &)
{
}

ConnectionController::ConnectionController(int connFd, KrfbServer *parent)
 : QThread(parent), fd(connFd), server(parent)
{
    framebufferImage = XGetImage(QX11Info::display(),  QApplication::desktop()->winId(),
                                0, 0,
                                QApplication::desktop()->width(),
                                QApplication::desktop()->height(),
                                AllPlanes,
                                ZPixmap);

}


ConnectionController::~ConnectionController()
{
}

void ConnectionController::run()
{
    kDebug() << "starting server connection" << endl;

    CurrentController *cc = new CurrentController(fd);
    controllers.setLocalData(cc);

    connect(cc, SIGNAL(sessionEstablished(QString)), server, SIGNAL(sessionEstablished(QString)));
    connect(cc, SIGNAL(notification(QString,QString)), server, SLOT(handleNotifications(QString, QString)));

    rfbScreenInfoPtr server;

    int w = framebufferImage->width;
    int h = framebufferImage->height;
    char *fb = framebufferImage->data;

    rfbLogEnable(0);
    server = rfbGetScreen(0, 0, w, h,
                          framebufferImage->bits_per_pixel,
                          8,
                          framebufferImage->bits_per_pixel/8);

    kDebug() << "acquired framebuffer" << endl;

    server->paddedWidthInBytes = framebufferImage->bytes_per_line;
    server->frameBuffer = fb;
    server->autoPort = true;
    server->inetdSock = fd;

    // server hooks
    server->newClientHook = newClientHook;

    server->kbdAddEvent = keyboardHook;
    server->ptrAddEvent = pointerHook;
    server->newClientHook = newClientHook;
    server->passwordCheck = passwordCheck;
    server->setXCutText = clipboardHook;

    server->desktopName = i18n("%1@%2 (shared desktop)", KUser().loginName(), QHostInfo::localHostName()).toLatin1();

    if (!myCursor) {
        myCursor = rfbMakeXCursor(19, 19, (char*) cur, (char*) mask);
    }
    server->cursor = myCursor;

    rfbInitServer(server);

    rfbRunEventLoop(server, 1000, false);

}





