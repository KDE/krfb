/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Collabora Ltd <info@collabora.co.uk>
   SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>
   SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>
   SPDX-FileCopyrightText: 2001-2003 Tim Jansen <tim@tjansen.de>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sockethelpers.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

QString peerAddress(int sock)
{
    const int ADDR_SIZE = 50;
    struct sockaddr sa = {};
    socklen_t salen = sizeof(struct sockaddr);

    if (getpeername(sock, &sa, &salen) == 0) {
        if (sa.sa_family == AF_INET) {
            auto si = (struct sockaddr_in *)&sa;
            return QString::fromLatin1(inet_ntoa(si->sin_addr));
        }

        if (sa.sa_family == AF_INET6) {
            char inetbuf[ADDR_SIZE];
            inet_ntop(sa.sa_family, &sa, inetbuf, ADDR_SIZE);
            return QString::fromLatin1(inetbuf);
        }

        return QStringLiteral("not a network address");
    }

    return QStringLiteral("unable to determine...");
}

unsigned short peerPort(int sock)
{
    struct sockaddr sa = {};
    socklen_t salen = sizeof(struct sockaddr);

    if (getpeername(sock, &sa, &salen) == 0) {
        auto si = (struct sockaddr_in *)&sa;
        return ntohs(si->sin_port);
    }

    return 0;
}

QString localAddress(int sock)
{
    const int ADDR_SIZE = 50;
    struct sockaddr sa = {};
    socklen_t salen = sizeof(struct sockaddr);

    if (getsockname(sock, &sa, &salen) == 0) {
        if (sa.sa_family == AF_INET) {
            auto si = (struct sockaddr_in *)&sa;
            return QString::fromLatin1(inet_ntoa(si->sin_addr));
        }

        if (sa.sa_family == AF_INET6) {
            char inetbuf[ADDR_SIZE];
            inet_ntop(sa.sa_family, &sa, inetbuf, ADDR_SIZE);
            return QString::fromLatin1(inetbuf);
        }

        return QStringLiteral("not a network address");
    }

    return QStringLiteral("unable to determine...");
}

unsigned short localPort(int sock)
{
    struct sockaddr sa = {};
    socklen_t salen = sizeof(struct sockaddr);

    if (getsockname(sock, &sa, &salen) == 0) {
        auto si = (struct sockaddr_in *)&sa;
        return ntohs(si->sin_port);
    }

    return 0;
}
