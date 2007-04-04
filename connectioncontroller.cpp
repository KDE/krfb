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
#include <QPixmap>
#include <QMutexLocker>
#include <QTimer>
#include <QRegion>
#include <QBitmap>

#include <KConfig>
#include <KGlobal>
#include <KUser>
#include <KNotification>
#include <KLocale>

#include "connectioncontroller.h"
#include "connectioncontroller.moc"

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
 : QThread(parent), fd(connFd), server(parent), fb(0)
{
#if 0
    framebufferImage = XGetImage(QX11Info::display(),  QApplication::desktop()->winId(),
                                0, 0,
                                QApplication::desktop()->width(),
                                QApplication::desktop()->height(),
                                AllPlanes,
                                ZPixmap);
#endif

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), SLOT(updateFrameBuffer()));
    fbImage = QPixmap::grabWindow(QApplication::desktop()->winId()).toImage();
    updateFrameBuffer();
    t->start(UPDATE_TIME);
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

    fblock.lock();

#if 0
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

    kDebug() << "image depth: " << framebufferImage->bits_per_pixel << endl;
    kDebug() << "image size: " << w << "x" << h << endl;
    kDebug() << "bytes per line: " << framebufferImage->bytes_per_line << endl;

#else

    int w = fbImage.width();
    int h = fbImage.height();

    //fb = (char *)fbImage.bits();

    rfbLogEnable(0);
    screen = rfbGetScreen(0, 0, w, h,
                            8,
                            3,
                            fbImage.depth()/8);
    screen->paddedWidthInBytes = fbImage.bytesPerLine();

    screen->serverFormat.bitsPerPixel = fbImage.depth();
    screen->serverFormat.depth = fbImage.depth();
    screen->serverFormat.trueColour = true;

    screen->serverFormat.bigEndian = false;
    screen->serverFormat.redShift = 16;
    screen->serverFormat.greenShift = 8;
    screen->serverFormat.blueShift = 0;
    screen->serverFormat.redMax   = 0xff;
    screen->serverFormat.greenMax = 0xff;
    screen->serverFormat.blueMax  = 0xff;

    kDebug() << "acquired framebuffer" << endl;

    kDebug() << "image format: " << fbImage.format() << endl;
    kDebug() << "image depth: " << fbImage.depth() << endl;
    kDebug() << "image size: " << fbImage.rect() << endl;
    kDebug() << "bytes per line: " << fbImage.bytesPerLine() << endl;
#endif

    screen->frameBuffer = fb;
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
    fblock.unlock();

    QTimer *t = new QTimer();
    connect(t, SIGNAL(timeout()), SLOT(processEvents()));
    rfbProcessEvents(screen, 1000);
    t->start(UPDATE_TIME);
    exec();
}


// WARNING: this function is called in the main GUI Thread, not in the connection
// thread! it can perform GUI calls but needs some care to avoid messing
// with the shared data.
void ConnectionController::updateFrameBuffer()
{
    QMutexLocker ml(&fblock);
    QImage img = QPixmap::grabWindow(QApplication::desktop()->winId()).toImage();
    QSize imgSize = img.size();

    if (!fb){
        fb = new char[img.numBytes()];
    }

    // verify what part of the image need to be marked as changed
    // fbImage is the previous version of the image,
    // img is the current one

#if 1 // This is actually slower than updating the whole desktop...

    QImage map(imgSize, QImage::Format_Mono);
    map.fill(0);

    for (int x = 0; x < imgSize.width(); x++) {
        for (int y = 0; y < imgSize.height(); y++) {
            if (img.pixel(x,y) != fbImage.pixel(x,y)) {
                map.setPixel(x,y,1);
            }
        }
    }

    QRegion r(QBitmap::fromImage(map));
#else


#endif

    tiles = tiles + r.rects();

    memcpy(fb, (const char*)img.bits(), img.numBytes());
    fbImage = img;
}

void ConnectionController::processEvents()
{
    QMutexLocker ml(&fblock);
    foreach(QRect r, tiles) {
        rfbMarkRectAsModified(screen, r.top(), r.left(), r.left() + r.width(), r.top() + r.height());
    }
    tiles.clear();

    rfbProcessEvents(screen, 1000);
}

