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

#ifndef KRFB_ABSTRACTCONNECTIONCONTROLLER_H
#define KRFB_ABSTRACTCONNECTIONCONTROLLER_H

#include <QtCore/QObject>

#include "rfb.h"

class AbstractRfbServer;

class AbstractConnectionController : public QObject
{
    Q_OBJECT

public:
    AbstractConnectionController(struct _rfbClientRec *_cl, AbstractRfbServer *parent);

    virtual ~AbstractConnectionController();

    virtual bool handleCheckPassword(rfbClientPtr cl, const char *response, int len) = 0;
    virtual void handleKeyEvent(bool down, rfbKeySym keySym);
    virtual void handlePointerEvent(int bm, int x, int y);
    virtual void handleClientGone() = 0;
    virtual void clipboardToServer(const QString &);

    virtual enum rfbNewClientAction handleNewClient() = 0;

    virtual void setControlEnabled(bool enable);

    virtual void setControlCanBeEnabled(bool canBeEnabled);
    virtual bool controlCanBeEnabled() const;

Q_SIGNALS:
    void sessionEstablished(QString);
    void clientDisconnected(AbstractConnectionController *);

protected Q_SLOTS:
    void dialogAccepted();
    void dialogRejected();

protected:
    QString remoteIp;
    struct _rfbClientRec *cl;
    bool controlEnabled;
    bool m_controlCanBeEnabled;
};


#endif  // Header Guard

