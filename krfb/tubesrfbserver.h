/*
    Copyright (C) 2009-2010 Collabora Ltd. <info@collabora.co.uk>
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
#include <TelepathyQt4/Channel>

class TubesRfbServer : public RfbServer
{
    Q_OBJECT
public:
    TubesRfbServer(const Tp::ChannelPtr & channel, QObject* parent = 0);
    virtual ~TubesRfbServer();

    virtual PendingRfbClient* newClient(rfbClientPtr client);

private Q_SLOTS:
    void close();
    void cleanup();
    void onChannelReady(Tp::PendingOperation *op);
    void onContactsUpgraded(Tp::PendingOperation *op);
    void offerTube();
    void onOfferTubeFinished(QDBusPendingCallWatcher *watcher);
    void onTubeStateChanged(uint state);
    void onNewRemoteConnection(uint handle, QDBusVariant connectionParam, uint connectionId);
    void onChannelInvalidated(Tp::DBusProxy *proxy,
                              const QString &errorName,
                              const QString &errorMessage);

private:
    struct Private;
    Private *const d;
};

#endif // TUBESRFBSERVER_H
