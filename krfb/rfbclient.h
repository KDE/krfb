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
#ifndef RFBCLIENT_H
#define RFBCLIENT_H

#include "rfb.h"
#include <QtCore/QObject>

class RfbClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool controlEnabled READ controlEnabled WRITE setControlEnabled NOTIFY controlEnabledChanged)
public:
    RfbClient(rfbClientPtr client, QObject *parent = 0);

    /** Returns a name for the client, to be shown to the user */
    virtual QString name();

    static bool controlCanBeEnabled();
    bool controlEnabled() const;
    void setControlEnabled(bool enabled);

    ///returns the internal rfbClient
    rfbClientPtr rfbClient() const { return m_client; }

Q_SIGNALS:
    void controlEnabledChanged(bool enabled);
    void connected(RfbClient *self);

protected:
    friend class RfbServerManager;
    ///called by RfbServerManager to begin handling the client
    virtual rfbNewClientAction doHandle();

    bool isConnected() const { return m_connected; }
    void setStatusConnected(); ///call to declare the client as connected

private Q_SLOTS:
    void dialogAccepted();
    void dialogRejected();

private:
    bool m_connected;
    bool m_controlEnabled;
    rfbClientPtr m_client;
};

#endif // RFBCLIENT_H
