/* This file is part of the KDE project
   Copyright (C) 2009 Collabora Ltd <info@collabora.co.uk>
    @author George Goldberg <george.goldberg@collabora.co.uk>
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>

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

#ifndef KRFB_KRFBSERVER_H
#define KRFB_KRFBSERVER_H

#include "abstractrfbserver.h"

class KrfbServer : public AbstractRfbServer
{
    Q_OBJECT
    friend class ServerManager;
public:
    virtual ~KrfbServer();

    virtual enum rfbNewClientAction handleNewClient(struct _rfbClientRec *cl);

public Q_SLOTS:
    void doStartListening();

    virtual void updateSettings();

private:
    KrfbServer();
};


#endif  // Header guard

