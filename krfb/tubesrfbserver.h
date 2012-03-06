/*
    Copyright (C) 2009-2011 Collabora Ltd. <info@collabora.co.uk>
      @author George Goldberg <george.goldberg@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TUBESRFBSERVER_H
#define TUBESRFBSERVER_H

#include "rfbserver.h"
#include <QtNetwork/QHostAddress>
#include <TelepathyQt/Types>

class TubesRfbServer : public RfbServer
{
    Q_OBJECT
public:
    static void init();

    virtual ~TubesRfbServer();

protected:
    TubesRfbServer(QObject *parent = 0);

    virtual PendingRfbClient* newClient(rfbClientPtr client);

private Q_SLOTS:
    void startAndCheck();

    void onTubeRequested();
    void onTubeClosed();

    void onNewTcpConnection(
            const QHostAddress &sourceAddress,
            quint16 sourcePort,
            const Tp::AccountPtr &account,
            const Tp::ContactPtr &contact,
            const Tp::OutgoingStreamTubeChannelPtr &tube);

    void onTcpConnectionClosed(
            const QHostAddress &sourceAddress,
            quint16 sourcePort,
            const Tp::AccountPtr &account,
            const Tp::ContactPtr &contact,
            const QString &error,
            const QString &message,
            const Tp::OutgoingStreamTubeChannelPtr &tube);

private:
    struct Private;
    Private *const d;
};

#endif // TUBESRFBSERVER_H
