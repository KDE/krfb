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
#ifndef RFBSERVERMANAGER_H
#define RFBSERVERMANAGER_H

#include "rfb.h"
#include <QtCore/QObject>

class RfbClient;
class RfbServerManagerStatic;
class RfbServer;

class RfbServerManager : public QObject
{
    Q_OBJECT
public:
    static RfbServerManager *instance();

Q_SIGNALS:
    void clientConnected(RfbClient *cc);
    void clientDisconnected(RfbClient *cc);

private Q_SLOTS:
    void init();
    void updateScreens();
    void cleanup();

private:
    void registerServer(RfbServer *server);
    void unregisterServer(RfbServer *server);

    rfbScreenInfoPtr newScreen();

    void addClient(RfbClient *cc);
    void removeClient(RfbClient *cc);

    RfbServerManager();
    virtual ~RfbServerManager();
    Q_DISABLE_COPY(RfbServerManager)

    friend class RfbServer;
    friend class RfbServerManagerStatic;

    struct Private;
    Private *const d;
};

#endif // RFBSERVERMANAGER_H
