/*
    Copyright (C) 2010 Collabora Ltd. <info@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef PENDINGRFBCLIENT_H
#define PENDINGRFBCLIENT_H

#include "rfbclient.h"
#include <QtCore/QObject>

class PendingRfbClient : public QObject
{
    Q_OBJECT
public:
    PendingRfbClient(rfbClientPtr client, QObject* parent = 0);

Q_SIGNALS:
    void finished(RfbClient *client);

protected Q_SLOTS:
    virtual void processNewClient() = 0;

    void showUserConfirmationDialog();

    void accept();
    void reject();

protected:
    RfbClient *m_client;
    rfbClientPtr m_rfbClient;
};

#endif // PENDINGRFBCLIENT_H
