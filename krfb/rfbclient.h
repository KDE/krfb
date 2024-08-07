/*
    SPDX-FileCopyrightText: 2009-2010 Collabora Ltd <info@collabora.co.uk>
    SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>
    SPDX-FileContributor: George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RFBCLIENT_H
#define RFBCLIENT_H

#include "rfb.h"
#include <QObject>

class QSocketNotifier;

class RfbClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool controlEnabled READ controlEnabled WRITE setControlEnabled NOTIFY controlEnabledChanged)
    Q_PROPERTY(bool onHold READ isOnHold WRITE setOnHold NOTIFY holdStatusChanged)
public:
    explicit RfbClient(rfbClientPtr client, QObject *parent = nullptr);
    ~RfbClient() override;

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

    rfbClientPtr getRfbClientPtr();
    virtual void handleKeyboardEvent(bool down, rfbKeySym keySym);
    virtual void handleMouseEvent(int buttonMask, int x, int y);

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
    explicit PendingRfbClient(rfbClientPtr client, QObject *parent = nullptr);
    ~PendingRfbClient() override;

Q_SIGNALS:
    void finished(RfbClient *client);

protected Q_SLOTS:
    virtual void processNewClient() = 0;

    void accept(RfbClient *newClient);
    void reject();

protected:

    friend class RfbServer; //Following two methods are handled by RfbServer

    /** This method is supposed to check if the provided \a encryptedPassword
     * matches the criteria for authenticating the client.
     * The default implementation returns false if a password is required.
     * Reimplement to do more useful stuff.
     */
    virtual bool checkPassword(const QByteArray & encryptedPassword);

    /** This method checks if the \a encryptedPassword that was sent from the remote
     * user matches the \a password that you have specified locally to be the password
     * for this connection. This assumes that the standard VNC authentication mechanism
     * is used. Returns true if the password matches or false otherwise.
     */
    bool vncAuthCheckPassword(const QByteArray & password, const QByteArray & encryptedPassword) const;

    rfbClientPtr m_rfbClient;

private:
    void onSocketActivated();

    QSocketNotifier *const m_notifier;
};

#endif // RFBCLIENT_H
