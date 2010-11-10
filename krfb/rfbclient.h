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
    Q_PROPERTY(bool onHold READ isOnHold WRITE setOnHold NOTIFY holdStatusChanged)
public:
    RfbClient(rfbClientPtr client, QObject *parent = 0);
    virtual ~RfbClient();

    /** Returns a name for the client, to be shown to the user */
    virtual QString name() const;

    static bool controlCanBeEnabled();
    bool controlEnabled() const;
    bool isOnHold() const;

public Q_SLOTS:
    void setControlEnabled(bool enabled);
    void setOnHold(bool onHold);
    void closeConnection();

Q_SIGNALS:
    void controlEnabledChanged(bool enabled);
    void holdStatusChanged(bool onHold);

protected:
    friend class RfbServer; //the following event handling methods are called by RfbServer

    virtual void handleKeyboardEvent(bool down, rfbKeySym keySym);
    virtual void handleMouseEvent(int buttonMask, int x, int y);

    /** This method is supposed to check if the provided \a encryptedPassword
     * matches the criteria for authenticating the client.
     * The default implementation returns false if a password is required.
     * Reimplement to do more useful stuff.
     */
    virtual bool checkPassword(const QByteArray & encryptedPassword);

    /** This method checks if the \a encryptedPassword that was sent from the remote
     * user matches the \a password that you have specified localy to be the password
     * for this connection. This assumes that the standard VNC authentication mechanism
     * is used. Returns true if the password matches or false otherwise.
     */
    bool vncAuthCheckPassword(const QByteArray & password, const QByteArray & encryptedPassword) const;

private Q_SLOTS:
    void onSocketActivated();

private:
    ///called by RfbServerManager to send framebuffer updates
    ///and check for possible disconnection
    void update();
    friend class RfbServerManager;

    struct Private;
    Private *const d;
};


class PendingRfbClient : public QObject
{
    Q_OBJECT
public:
    PendingRfbClient(rfbClientPtr client, QObject *parent = 0);
    virtual ~PendingRfbClient();

Q_SIGNALS:
    void finished(RfbClient *client);

protected Q_SLOTS:
    virtual void processNewClient() = 0;

    void accept(RfbClient *newClient);
    void reject();

protected:
    rfbClientPtr m_rfbClient;
};

#endif // RFBCLIENT_H
