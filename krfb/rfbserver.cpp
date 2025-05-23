/*
    SPDX-FileCopyrightText: 2009-2010 Collabora Ltd <info@collabora.co.uk>
    SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>
    SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rfbserver.h"
#include "rfbservermanager.h"
#include "krfbdebug.h"
#include <QSocketNotifier>
#include <QApplication>
#include <QClipboard>
#include <QPointer>
#include <QtGui/private/qtx11extras_p.h>

struct RfbServer::Private
{
    QByteArray listeningAddress;
    int listeningPort;
    bool passwordRequired;
    rfbScreenInfoPtr screen;
    QPointer<QSocketNotifier> ipv4notifier;
    QPointer<QSocketNotifier> ipv6notifier;
};

RfbServer::RfbServer(QObject *parent)
    : QObject(parent), d(new Private)
{
    d->listeningAddress = "0.0.0.0";
    d->listeningPort = 0;
    d->passwordRequired = true;
    d->screen = nullptr;

    RfbServerManager::instance()->registerServer(this);
}

RfbServer::~RfbServer()
{
    if (d->screen) {
        rfbScreenCleanup(d->screen);
    }
    delete d;

    RfbServerManager::instance()->unregisterServer(this);
}

QByteArray RfbServer::listeningAddress() const
{
    return d->listeningAddress;
}

int RfbServer::listeningPort() const
{
    return d->listeningPort;
}

bool RfbServer::passwordRequired() const
{
    return d->passwordRequired;
}

void RfbServer::setListeningAddress(const QByteArray& address)
{
    d->listeningAddress = address;
}

void RfbServer::setListeningPort(int port)
{
    d->listeningPort = port;
}

void RfbServer::setPasswordRequired(bool passwordRequired)
{
    d->passwordRequired = passwordRequired;
}

bool RfbServer::start()
{
    if (!d->screen) {
        d->screen = RfbServerManager::instance()->newScreen();
        if (!d->screen) {
            qCDebug(KRFB) << "Unable to get rbfserver screen";
            return false;
        }

        // server hooks
        d->screen->screenData = this;
        d->screen->newClientHook = newClientHook;
        d->screen->kbdAddEvent = keyboardHook;
        d->screen->ptrAddEvent = pointerHook;
        d->screen->passwordCheck = passwordCheck;
        d->screen->setXCutText = clipboardHook;
        d->screen->setXCutTextUTF8 = clipboardHookUtf8;
    } else {
        //if we already have a screen, stop listening first
        rfbShutdownServer(d->screen, false);
    }

    if (listeningAddress() != "0.0.0.0") {
        strncpy(d->screen->thisHost, listeningAddress().constData(), 254);
    }

    if (listeningPort() == 0) {
        d->screen->autoPort = 1;
    }

    d->screen->port = listeningPort();
    d->screen->ipv6port = listeningPort();

    // Disable/Enable password checking
    if (passwordRequired()) {
        d->screen->authPasswdData = (void *)1;
    } else {
        d->screen->authPasswdData = (void *)nullptr;
    }

    qCDebug(KRFB) << "Starting server. Listen port:" << listeningPort()
             << "Listen Address:" << listeningAddress()
             << "Password enabled:" << passwordRequired();

    rfbInitServer(d->screen);

    if (!rfbIsActive(d->screen)) {
        qCDebug(KRFB) << "Failed to start server";
        rfbShutdownServer(d->screen, false);
        return false;
    };

    d->ipv4notifier = new QSocketNotifier(d->screen->listenSock, QSocketNotifier::Read, this);
    connect(d->ipv4notifier, &QSocketNotifier::activated, this, &RfbServer::onListenSocketActivated);
    if (d->screen->listen6Sock > 0) {
        // we're also listening on additional IPv6 socket, get events from there
        d->ipv6notifier = new QSocketNotifier(d->screen->listen6Sock, QSocketNotifier::Read, this);
        connect(d->ipv6notifier, &QSocketNotifier::activated, this, &RfbServer::onListenSocketActivated);
    }

    if (QX11Info::isPlatformX11()) {
        connect(QApplication::clipboard(), &QClipboard::dataChanged,
                this, &RfbServer::krfbSendServerCutText);
    }

    return true;
}

void RfbServer::stop()
{
    if (d->screen) {
        rfbShutdownServer(d->screen, true);
        for (auto notifier : {d->ipv4notifier, d->ipv6notifier}) {
            if (notifier) {
                notifier->setEnabled(false);
                notifier->deleteLater();
            }
        }
    }
}

void RfbServer::updateFrameBuffer(char *fb, int width, int height, int depth)
{
    int bpp = depth >> 3;

    if (bpp != 1 && bpp != 2 && bpp != 4) {
        bpp = 4;
    }

    rfbNewFramebuffer(d->screen, fb, width, height, 8, 3, bpp);
}

void RfbServer::updateScreen(const QList<QRect> & modifiedTiles)
{
    if (d->screen) {
        QList<QRect>::const_iterator it = modifiedTiles.constBegin();
        for(; it != modifiedTiles.constEnd(); ++it) {
            rfbMarkRectAsModified(d->screen, it->x(), it->y(), it->right(), it->bottom());
        }
    }
}

/*
 * Code copied from vino's bundled libvncserver:
 *  SPDX-FileCopyrightText: 2000, 2001 Const Kaplinsky. All Rights Reserved.
 *  SPDX-FileCopyrightText: 1999 AT &T Laboratories Cambridge.  All Rights Reserved.
 *  License: GPL v2 or later
 */
void krfb_rfbSetCursorPosition(rfbScreenInfoPtr screen, rfbClientPtr client, int x, int y)
{
    rfbClientIteratorPtr iterator;
    rfbClientPtr cl;

    if (x == screen->cursorX || y == screen->cursorY)
        return;

    LOCK(screen->cursorMutex);
    screen->cursorX = x;
    screen->cursorY = y;
    UNLOCK(screen->cursorMutex);

    /* Inform all clients about this cursor movement. */
    iterator = rfbGetClientIterator(screen);
    while ((cl = rfbClientIteratorNext(iterator)) != nullptr) {
        cl->cursorWasMoved = true;
    }
    rfbReleaseClientIterator(iterator);

    /* The cursor was moved by this client, so don't send CursorPos. */
    if (client) {
        client->cursorWasMoved = false;
    }
}

void RfbServer::updateCursorPosition(const QPoint & position)
{
    if (d->screen) {
        krfb_rfbSetCursorPosition(d->screen, nullptr, position.x(), position.y());
    }
}

void RfbServer::krfbSendServerCutText()
{
    if (d->screen) {
        QString text = QApplication::clipboard()->text();
        QByteArray utf8Bytes = text.toUtf8();
        QByteArray latin1Bytes = text.toLatin1();
        // Try to send the clipboard text to the client in UTF-8. Use Latin1 if UTF-8 is not supported.
        rfbSendServerCutTextUTF8(d->screen, utf8Bytes.data(), utf8Bytes.length(), latin1Bytes.data(), latin1Bytes.length());
    }
}

void RfbServer::onListenSocketActivated()
{
    rfbProcessNewConnection(d->screen);
}

void RfbServer::pendingClientFinished(RfbClient *client)
{
    //qDebug();
    if (client) {
        RfbServerManager::instance()->addClient(client);
        client->getRfbClientPtr()->clientGoneHook = clientGoneHook;
    }
}

//static
rfbNewClientAction RfbServer::newClientHook(rfbClientPtr cl)
{
    //qDebug() << "New client";
    auto server = static_cast<RfbServer*>(cl->screen->screenData);

    PendingRfbClient *pendingClient = server->newClient(cl);
    connect(pendingClient, &PendingRfbClient::finished,
            server, &RfbServer::pendingClientFinished);

    return RFB_CLIENT_ON_HOLD;
}

//static
void RfbServer::clientGoneHook(rfbClientPtr cl)
{
    //qDebug() << "client gone";
    auto client = static_cast<RfbClient*>(cl->clientData);

    RfbServerManager::instance()->removeClient(client);
    client->deleteLater();
}

//static
rfbBool RfbServer::passwordCheck(rfbClientPtr cl, const char *encryptedPassword, int len)
{
    auto client = static_cast<PendingRfbClient*>(cl->clientData);
    Q_ASSERT(client);
    return client->checkPassword(QByteArray::fromRawData(encryptedPassword, len));
}

//static
void RfbServer::keyboardHook(rfbBool down, rfbKeySym keySym, rfbClientPtr cl)
{
    auto client = static_cast<RfbClient*>(cl->clientData);
    client->handleKeyboardEvent(down ? true : false, keySym);
}

//static
void RfbServer::pointerHook(int bm, int x, int y, rfbClientPtr cl)
{
    auto client = static_cast<RfbClient*>(cl->clientData);
    client->handleMouseEvent(bm, x, y);
}

//static
void RfbServer::clipboardHook(char *str, int len, rfbClientPtr /*cl*/)
{
    QString text = QString::fromLatin1(str, len);
    QApplication::clipboard()->setText(text);
}

void RfbServer::clipboardHookUtf8(char *str, int len, rfbClientPtr /*cl*/)
{
    // The last byte is a null terminator
    QString text = QString::fromUtf8(str, len - 1);
    QApplication::clipboard()->setText(text);
}
