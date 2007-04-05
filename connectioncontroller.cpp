/* This file is part of the KDE project
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
             (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/
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

#include "connectioncontroller.h"
#include "connectioncontroller.moc"

#include "events.h"
#include "invitationmanager.h"
#include "krfbserver.h"
#include "framebuffer.h"

#include <X11/Xutil.h>

const int UPDATE_TIME = 100;

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

static enum rfbNewClientAction newClientHook(struct _rfbClientRec *cl)
{
    ConnectionController *cc = static_cast<ConnectionController *>(cl->screen->screenData);
    return cc->handleNewClient(cl);
}

static rfbBool passwordCheck(rfbClientPtr cl,
                             const char* encryptedPassword,
                             int len)
{
    ConnectionController *cc = static_cast<ConnectionController *>(cl->screen->screenData);
    return cc->handleCheckPassword(cl, encryptedPassword, len);
}

static void keyboardHook(rfbBool down, rfbKeySym keySym, rfbClientPtr cl)
{
    ConnectionController *cc = static_cast<ConnectionController *>(cl->screen->screenData);
    cc->handleKeyEvent(down ? true : false, keySym);
}

static void pointerHook(int bm, int x, int y, rfbClientPtr cl)
{
    ConnectionController *cc = static_cast<ConnectionController *>(cl->screen->screenData);
    cc->handlePointerEvent(bm, x, y);
}

static void clientGoneHook(rfbClientPtr cl)
{
    ConnectionController *cc = static_cast<ConnectionController *>(cl->screen->screenData);
    cc->handleClientGone();
}

static void clipboardHook(char* str,int len, rfbClientPtr cl)
{
    ConnectionController *cc = static_cast<ConnectionController *>(cl->screen->screenData);
    cc->clipboardToServer(QString::fromUtf8(str, len));
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


ConnectionController::ConnectionController(int connFd, KrfbServer *parent)
    : QObject(parent), fd(connFd), server(parent), fb(0)
{
    fb = new FrameBuffer(QApplication::desktop()->winId(), this);
}


ConnectionController::~ConnectionController()
{
}

bool ConnectionController::handleCheckPassword(rfbClientPtr cl, const char *response, int len)
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


enum rfbNewClientAction ConnectionController::handleNewClient(rfbClientPtr cl)
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

void ConnectionController::sendKNotifyEvent(const QString & name, const QString & desc)
{
    emit notification(name, desc);
}

void ConnectionController::sendSessionEstablished()
{
    emit sessionEstablished("BAH");
}

void ConnectionController::handleKeyEvent(bool down, rfbKeySym keySym)
{
    KeyboardEvent ev(down, keySym);
    ev.exec();
}

void ConnectionController::handlePointerEvent(int bm, int x, int y)
{
    PointerEvent ev(bm, x, y);
    ev.exec();
}

void ConnectionController::handleClientGone()
{
    rfbCloseClient(client);
}

void ConnectionController::clipboardToServer(const QString &s)
{
    ClipboardEvent ev(this, s);
    ev.exec();
}


void ConnectionController::run()
{
    kDebug() << "starting server connection" << endl;

    connect(this, SIGNAL(sessionEstablished(QString)), server, SIGNAL(sessionEstablished(QString)));
    connect(this, SIGNAL(notification(QString,QString)), server, SLOT(handleNotifications(QString, QString)));


    int w = fb->width();
    int h = fb->height();
    int depth = fb->depth();

    rfbLogEnable(0);
    screen = rfbGetScreen(0, 0, w, h, 8, 3,depth / 8);

    screen->screenData = (void *)this;
    screen->paddedWidthInBytes = w * 4;

    fb->getServerFormat(screen->serverFormat);

    screen->frameBuffer = fb->data();
    screen->autoPort = true;
    screen->inetdSock = fd;

    // server hooks
    screen->newClientHook = newClientHook;

    screen->kbdAddEvent = keyboardHook;
    screen->ptrAddEvent = pointerHook;
    screen->newClientHook = newClientHook;
    screen->passwordCheck = passwordCheck;
    screen->setXCutText = clipboardHook;

    screen->desktopName = i18n("%1@%2 (shared desktop)", KUser().loginName(), QHostInfo::localHostName()).toLatin1();

    if (!myCursor) {
        myCursor = rfbMakeXCursor(19, 19, (char*) cur, (char*) mask);
    }
    screen->cursor = myCursor;

    rfbInitServer(screen);

    while (true) {
        foreach(QRect r, fb->modifiedTiles()) {
            rfbMarkRectAsModified(screen, r.top(), r.left(), r.left() + r.width(), r.top() + r.height());
        }
        fb->modifiedTiles().clear();
        rfbProcessEvents(screen, 100);
        qApp->processEvents();
    }

    /*
    QTimer *t = new QTimer();
    connect(t, SIGNAL(timeout()), SLOT(processEvents()));
    rfbProcessEvents(screen, 10);
    t->start(UPDATE_TIME);
    */

}


void ConnectionController::processEvents()
{
}

