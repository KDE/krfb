/*
 *  Find network adapter's IP address
 *  Copyright (C) 2002 Tim Jansen <tim@tjansen.de>
 *  This code is based on KPhone/libdissipate's SIPUtil class.
 *  Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <config.h>
#include <qmap.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/time.h>
/* WirLab 29.1.2002 */
#include <sys/errno.h>
/* WirLab 31.1.2002 */
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/*--*/
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#ifdef USE_SOLARIS
#include <sys/sockio.h>
#endif


/* max number of network interfaces*/
#define MAX_IF 16


QMap<QString,QString> kinetaddr_getAllInterfaces(bool includeLoopback)
{
	QMap<QString,QString> map;
	int sock, err, if_count, i;
	struct ifconf netconf;
	char buffer[32*MAX_IF];

	netconf.ifc_len = 32 * MAX_IF;
	netconf.ifc_buf = buffer;
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	err = ioctl(sock, SIOCGIFCONF, &netconf);
	if (err < 0) 
		return map;
	close(sock);

	if_count = netconf.ifc_len / 32;
	for(i = 0; i < if_count; i++) {
		if(includeLoopback || 
		   (strcmp( netconf.ifc_req[i].ifr_name, "lo" ) != 0)) {
			map.insert(netconf.ifc_req[i].ifr_name,
				   inet_ntoa(((struct sockaddr_in*)(&netconf.ifc_req[i].ifr_addr))->sin_addr));
		}
	}
	return map;
}


