/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Collabora Ltd <info@collabora.co.uk>
   SPDX-FileContributor: George Goldberg <george.goldberg@collabora.co.uk>
   SPDX-FileCopyrightText: 2007 Alessandro Praduroux <pradu@pradu.it>
   SPDX-FileCopyrightText: 2001-2003 Tim Jansen <tim@tjansen.de>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SOCKETHELPERS_H
#define SOCKETHELPERS_H

#include <QString>

QString peerAddress(int sock);
unsigned short peerPort(int sock);

QString localAddress(int sock);
unsigned short localPort(int sock);

#endif
