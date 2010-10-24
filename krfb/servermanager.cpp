/* This file is part of the KDE project
   Copyright (C) 2009 Collabora Ltd <info@collabora.co.uk>
    @author George Goldberg <george.goldberg@collabora.co.uk>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "servermanager.h"

#include "sockethelpers.h"
#include "rfb.h"

#include <KDebug>
#include <KGlobal>

class ServerManagerStatic
{
public:
    ServerManager instance;
};

K_GLOBAL_STATIC(ServerManagerStatic, serverManagerStatic)

ServerManager::ServerManager()
{
    kDebug();
}

ServerManager::~ServerManager()
{
    kDebug();
}

ServerManager *ServerManager::instance()
{
    kDebug();

    return &serverManagerStatic->instance;
}

KrfbServer *ServerManager::serverForClient(struct _rfbClientRec *cl)
{
    kDebug();

    foreach(KrfbServer * server, m_servers) {
        if ((server->listeningAddress() == localAddress(cl->sock)) &&
                (server->listeningPort() == localPort(cl->sock))) {
            return server;
        } else if ((server->listeningAddress() == "0.0.0.0") &&
                   (server->listeningPort() == localPort(cl->sock))) {
            return server;
        }
    }

    kWarning() << "No server found for this incoming client :/ probably will crash.";
    return 0;
}

KrfbServer *ServerManager::newServer()
{
    kDebug();

    KrfbServer *ret = new KrfbServer;
    m_servers.insert(ret);

    return ret;
}

void ServerManager::updateServers()
{
    kDebug();

    // Inform all servers that the configuration has been changed.
    foreach(KrfbServer * server, m_servers) {
        server->updateSettings();
    }
}


#include "servermanager.moc"

