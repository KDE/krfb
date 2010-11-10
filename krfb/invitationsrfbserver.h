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
#ifndef INVITATIONSRFBSERVER_H
#define INVITATIONSRFBSERVER_H

#include "rfbserver.h"

class InvitationsRfbServer : public RfbServer
{
    Q_OBJECT
public:
    static void init();
    static void cleanup();

protected:
    InvitationsRfbServer() : RfbServer(0) {}

    virtual RfbClient* newClient(rfbClientPtr client);
    virtual bool checkPassword(RfbClient* client, const char* encryptedPassword, int len);

private Q_SLOTS:
    void startAndCheck();

private:
    Q_DISABLE_COPY(InvitationsRfbServer)
};

#endif // INVITATIONSRFBSERVER_H
