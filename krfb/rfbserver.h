/*
    SPDX-FileCopyrightText: 2009-2010 Collabora Ltd <info@collabora.co.uk>
    SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>
    SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RFBSERVER_H
#define RFBSERVER_H

#include "rfb.h"
#include "rfbclient.h"
#include <QRect>

class RfbServer : public QObject
{
    Q_OBJECT
public:
    explicit RfbServer(QObject *parent = nullptr);
    ~RfbServer() override;

    QByteArray listeningAddress() const;
    int listeningPort() const;
    bool passwordRequired() const;

    void setListeningAddress(const QByteArray & address);
    void setListeningPort(int port);
    void setPasswordRequired(bool passwordRequired);

public Q_SLOTS:
    virtual bool start();
    virtual void stop();

    void updateFrameBuffer(char *fb, int width, int height, int depth);
    void updateScreen(const QList<QRect> & modifiedTiles);
    void updateCursorPosition(const QPoint & position);

private Q_SLOTS:
    void krfbSendServerCutText();
    void onListenSocketActivated();
    void pendingClientFinished(RfbClient *client);

protected:
    virtual PendingRfbClient *newClient(rfbClientPtr client) = 0;

private:
    static rfbNewClientAction newClientHook(rfbClientPtr cl);
    static void clientGoneHook(rfbClientPtr cl);

    static rfbBool passwordCheck(rfbClientPtr cl, const char *encryptedPassword, int len);
    static void keyboardHook(rfbBool down, rfbKeySym keySym, rfbClientPtr cl);
    static void pointerHook(int bm, int x, int y, rfbClientPtr cl);
    static void clipboardHook(char *str, int len, rfbClientPtr cl);

    Q_DISABLE_COPY(RfbServer)

    struct Private;
    Private *const d;
};

#endif // RFBSERVER_H
