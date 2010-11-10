/* This file is part of the KDE project
 *   Copyright (C) 2009 Collabora Ltd <info@collabora.co.uk>
 *    @author George Goldberg <george.goldberg@collabora.co.uk>
 *   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
 *   Copyright (C) 2001-2003 by Tim Jansen <tim@tjansen.de>
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

#include "abstractconnectioncontroller.h"
#include "abstractrfbserver.h"
#include "connectiondialog.h"
#include "events.h"

#include <KDebug>

#include <string.h>

static void clientGoneHook(rfbClientPtr cl)
{
    AbstractConnectionController *cc = static_cast<AbstractConnectionController *>(cl->clientData);
    cc->handleClientGone();
}

AbstractConnectionController::AbstractConnectionController(struct _rfbClientRec *_cl,
        AbstractRfbServer *parent)
    : QObject(parent),
      cl(_cl)
{
    kDebug();

    // Set the rfbClientRec client data to point to this ConnectionController so the callbacks
    // can get the right object.
    cl->clientData = (void *)this;

    // Set the client gone hook. The ConnectionController is responsible for determining what
    // actions to take based on the state of the client in handleClientGone().
    cl->clientGoneHook = clientGoneHook;
}

AbstractConnectionController::~AbstractConnectionController()
{
    kDebug();
}

void AbstractConnectionController::dialogAccepted()
{
    ConnectionDialog *dialog = qobject_cast<ConnectionDialog *>(sender());

    if (!dialog) {
        kWarning() << "Wrong type of sender.";
        return;
    }

    // rfbStartOnHoldClient(cl);
    cl->onHold = false;
    setControlEnabled(dialog->cbAllowRemoteControl->isChecked());
    setControlCanBeEnabled(dialog->cbAllowRemoteControl->isChecked());
    emit sessionEstablished(remoteIp);
}

void AbstractConnectionController::dialogRejected()
{
    kDebug() << "refused connection";
    rfbRefuseOnHoldClient(cl);
}

void AbstractConnectionController::handleKeyEvent(bool down, rfbKeySym keySym)
{
    if (controlEnabled) {
        EventHandler::handleKeyboard(down, keySym);
    }
}

void AbstractConnectionController::handlePointerEvent(int bm, int x, int y)
{
    if (controlEnabled) {
        EventHandler::handlePointer(bm, x, y);
    }
}

void AbstractConnectionController::clipboardToServer(const QString &s)
{
    //TODO implement me
}

void AbstractConnectionController::setControlEnabled(bool enable)
{
    if (m_controlCanBeEnabled) {
        controlEnabled = enable;
    }
}

void AbstractConnectionController::setControlCanBeEnabled(bool canBeEnabled)
{
    m_controlCanBeEnabled = canBeEnabled;
}

bool AbstractConnectionController::controlCanBeEnabled() const
{
    return m_controlCanBeEnabled;
}


#include "abstractconnectioncontroller.moc"

