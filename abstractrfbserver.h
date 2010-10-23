/* This file is part of the KDE project
 *   Copyright (C) 2009 Collabora Ltd <info@collabora.co.uk>
 *    @author George Goldberg <george.goldberg@collabora.co.uk>
 *   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; see the file COPYING.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */

#ifndef KRFB_ABSTRACTRFBSERVER_H
#define KRFB_ABSTRACTRFBSERVER_H

#include "framebuffer.h"

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>

#include "rfb.h"

class AbstractConnectionController;

class AbstractRfbServer : public QObject
{
    Q_OBJECT
    friend class ServerManager;

public:
    virtual ~AbstractRfbServer();

    virtual enum rfbNewClientAction handleNewClient(struct _rfbClientRec *cl) = 0;
    virtual bool checkX11Capabilities();

Q_SIGNALS:
    void sessionEstablished(QString);
    void sessionFinished();
    void desktopControlSettingChanged(bool);

public Q_SLOTS:
    virtual void processRfbEvents();
    virtual void shutdown();
    virtual void enableDesktopControl(bool);
    virtual void updateSettings() = 0;
    virtual void updatePassword();
    virtual void clientDisconnected(AbstractConnectionController *);

    virtual QString listeningAddress() const;
    virtual unsigned int listeningPort() const;

protected:
    AbstractRfbServer();

    virtual void setListeningAddress(const QByteArray &address);
    virtual void setListeningPort(int port);
    virtual void setFrameBuffer(QSharedPointer<FrameBuffer> frameBuffer);
    virtual void setDesktopName(const QByteArray &desktopName);
    virtual void setPasswordRequired(bool passwordRequired);
    virtual void startListening();

    virtual void appendController(AbstractConnectionController *c);
    virtual int numClients() const;
    virtual void startFrameBufferMonitor();
    virtual void incrementNumClients();
    void addController(AbstractConnectionController *cc);

private:
    Q_DISABLE_COPY(AbstractRfbServer)

    class AbstractRfbServerPrivate;
    AbstractRfbServerPrivate * const d;

};


#endif  // Header guard

