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
#include "rfbserver.h"
#include "events.h"
#include "rfbservermanager.h"

struct RfbServer::Private
{
    QByteArray listeningAddress;
    int listeningPort;
    bool passwordRequired;
    int id;
};

RfbServer::RfbServer(QObject *parent)
    : QObject(parent), d(new Private)
{
    d->listeningAddress = "0.0.0.0";
    d->listeningPort = 0;
    d->passwordRequired = true;
    d->id = -1;
}

RfbServer::~RfbServer()
{
    stop();
    delete d;
}

void RfbServer::handleKeyboardEvent(RfbClient* client, rfbBool down, rfbKeySym keySym)
{
    if (client->controlEnabled()) {
        EventHandler::handleKeyboard(down, keySym);
    }
}

void RfbServer::handleMouseEvent(RfbClient* client, int buttonMask, int x, int y)
{
    if (client->controlEnabled()) {
        EventHandler::handlePointer(buttonMask, x, y);
    }
}

/** Default implementation returns false if a password is required.
 * Reimplement to do more useful stuff
 */
bool RfbServer::checkPassword(RfbClient* client, const char* encryptedPassword, int len)
{
    Q_UNUSED(client);
    Q_UNUSED(encryptedPassword);
    Q_UNUSED(len);

    return !d->passwordRequired;
}

QByteArray RfbServer::listeningAddress() const
{
    return d->listeningAddress;
}

int RfbServer::listeningPort() const
{
    return d->listeningPort;
}

bool RfbServer::passwordRequired() const
{
    return d->passwordRequired;
}

void RfbServer::setListeningAddress(const QByteArray& address)
{
    d->listeningAddress = address;
}

void RfbServer::setListeningPort(int port)
{
    d->listeningPort = port;
}

void RfbServer::setPasswordRequired(bool passwordRequired)
{
    d->passwordRequired = passwordRequired;
}

bool RfbServer::start()
{
    if (d->id == -1) {
        return (d->id = RfbServerManager::instance()->startServer(this)) != -1;
    } else {
        return false; //server has already started
    }
}

void RfbServer::stop(bool disconnectClients)
{
    RfbServerManager::instance()->stopServer(d->id, disconnectClients);
    if (disconnectClients) {
        d->id = -1;
    }
}

#include "rfbserver.moc"
