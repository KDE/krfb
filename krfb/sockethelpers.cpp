/* This file is part of the KDE project
   Copyright (C) 2009 Collabora Ltd <info@collabora.co.uk>
    @author George Goldberg <george.goldberg@collabora.co.uk>
   Copyright (C) 2007 Alessandro Praduroux <pradu@pradu.it>
   Copyright (C) 2001-2003 by Tim Jansen <tim@tjansen.de>

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

#include "sockethelpers.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

QString peerAddress(int sock)
{

    const int ADDR_SIZE = 50;
    struct sockaddr sa;
    socklen_t salen = sizeof(struct sockaddr);

    if (getpeername(sock, &sa, &salen) == 0) {
        if (sa.sa_family == AF_INET) {
            struct sockaddr_in *si = (struct sockaddr_in *)&sa;
            return QString(inet_ntoa(si->sin_addr));
        }

        if (sa.sa_family == AF_INET6) {
            char inetbuf[ADDR_SIZE];
            inet_ntop(sa.sa_family, &sa, inetbuf, ADDR_SIZE);
            return QString(inetbuf);
        }

        return QString("not a network address");
    }

    return QString("unable to determine...");
}

unsigned short peerPort(int sock)
{

    struct sockaddr sa;
    socklen_t salen = sizeof(struct sockaddr);

    if (getpeername(sock, &sa, &salen) == 0) {
        struct sockaddr_in *si = (struct sockaddr_in *)&sa;
        return ntohs(si->sin_port);
    }

    return 0;
}

QString localAddress(int sock)
{

    const int ADDR_SIZE = 50;
    struct sockaddr sa;
    socklen_t salen = sizeof(struct sockaddr);

    if (getsockname(sock, &sa, &salen) == 0) {
        if (sa.sa_family == AF_INET) {
            struct sockaddr_in *si = (struct sockaddr_in *)&sa;
            return QString(inet_ntoa(si->sin_addr));
        }

        if (sa.sa_family == AF_INET6) {
            char inetbuf[ADDR_SIZE];
            inet_ntop(sa.sa_family, &sa, inetbuf, ADDR_SIZE);
            return QString(inetbuf);
        }

        return QString("not a network address");
    }

    return QString("unable to determine...");
}

unsigned short localPort(int sock)
{

    struct sockaddr sa;
    socklen_t salen = sizeof(struct sockaddr);

    if (getsockname(sock, &sa, &salen) == 0) {
        struct sockaddr_in *si = (struct sockaddr_in *)&sa;
        return ntohs(si->sin_port);
    }

    return 0;
}

