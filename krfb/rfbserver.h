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
#ifndef RFBSERVER_H
#define RFBSERVER_H

#include "rfb.h"
#include "rfbclient.h"
#include <QtCore/QRect>

class RfbServer : public QObject
{
    Q_OBJECT
public:
    RfbServer(QObject *parent = 0);
    virtual ~RfbServer();

    QByteArray listeningAddress() const;
    int listeningPort() const;
    bool passwordRequired() const;

    void setListeningAddress(const QByteArray & address);
    void setListeningPort(int port);
    void setPasswordRequired(bool passwordRequired);

public Q_SLOTS:
    bool start();
    void stop(bool disconnectClients = true);

    void updateScreen(const QList<QRect> & modifiedTiles);
    void updateCursorPosition(const QPoint & position);

private Q_SLOTS:
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
