/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2002 Tim Jansen <tim@tjansen.de>
 *  based on code from KInetSocketAddress:
 *    Copyright (C) 2000,2001 Thiago Macieira <thiagom@mail.com>
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
#include <kdebug.h>
#include "kinetaddr.h"

/*
 * This code is based on KPhone/libdissipate's SIPUtil class.
 * Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
 *
 * The code is far from being perfect, and gateway detection only works on 
 * Linux. Later there should be a way to configure the addresses using 
 * a KCM.
 * 
 */

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


static char *local_address = NULL;
static char *inet_address = NULL;

/* max number of network interfaces*/
#define MAX_IF 8

/* Path to the route entry in proc filesystem */
#define PROCROUTE "/proc/net/route"

#ifndef SIOCGIFCOUNT
#define SIOCGIFCOUNT 0x8935
#endif

char iface[16];


/* This searches the proc routing entry for the interface the default gateway
 * is on, and returns the name of that interface.
 */
char *getdefaultdev()
{
	FILE *fp = fopen( PROCROUTE, "r");
	char buff[4096], gate_addr[128], net_addr[128];
	char mask_addr[128];
	int irtt, window, mss, num, metric, iflags, refcnt, use;
	char i;
	if( !fp ) {
		return NULL;
	}
	i=0;
// cruise through the list, and find the gateway interface
	while( fgets(buff, 1023, fp) ) {
		num = sscanf(buff, "%s %s %s %X %d %d %d %s %d %d %d\n",
			iface, net_addr, gate_addr, &iflags, &refcnt, &use, &metric,
			&mask_addr, &mss, &window, &irtt);
		i++;
		if( i == 1) continue;

		if( iflags & RTF_GATEWAY )
			return iface;
	}
	fclose(fp);
/* didn't find a default gateway */
	return NULL;
}


static void findAddresses( void )
{
	int sock, err, if_count, i, j = 0;
	struct ifconf netconf;
	char buffer[32*MAX_IF];

	char if_name[10][21];
	char if_addr[10][21];
	char *default_ifName;

	netconf.ifc_len = 32 * MAX_IF;
	netconf.ifc_buf = buffer;
	sock=socket( PF_INET, SOCK_DGRAM, 0 );
	err=ioctl( sock, SIOCGIFCONF, &netconf );
	if ( err < 0 ) 
		kdDebug() << "KInetAddress: Error in ioctl: "<<errno<<"." << endl;
	close( sock );

	if_count = netconf.ifc_len / 32;
	for( i = 0; i < if_count; i++ ) {
		if( strcmp( netconf.ifc_req[i].ifr_name, "lo" ) != 0 ) {
			strncpy( if_name[j], netconf.ifc_req[i].ifr_name, 20 );
			strncpy( if_addr[j], inet_ntoa(((struct sockaddr_in*)(&netconf.ifc_req[i].ifr_addr))->sin_addr), 20 );
			j++;
		}
	}

	if(j == 0) {
		local_address = "localhost";
		inet_address = "localhost";
		return;
	}
	if(j == 1) {
		local_address = strdup( if_addr[0] );
		inet_address = local_address;
		return;
	}

	/* take default gateway interface for inet address if available */ 
	default_ifName = getdefaultdev();
	if( default_ifName) {
		for( i = 0; i < j; i++ ) {
			if( strcmp(if_name[i], default_ifName) == 0 ) {
				inet_address = strdup(if_addr[i]);
				break;
			}
		}
	}

	/* use first ppp connection for inet, first non-ppp for local */
	for( i = 0; i < j; i++ ) {
		if((strncmp(if_name[i], "ppp", 3) == 0) ||
		   (strncmp(if_name[i], "ippp", 4) == 0)) {
			if (!inet_address) {
				inet_address = strdup(if_addr[i]);
			}
		}
		else if (!local_address) {
			local_address = strdup(if_addr[i]);
		}
	}

	/* if nothing did work, just take anything */
	if (!inet_address)
		inet_address = strdup(if_addr[0]);
	if (!local_address)
		local_address = strdup(if_addr[0]);
}


KInetAddress* KInetAddress::getPrivateInetAddress() {
	if ( inet_address == NULL ) {
		findAddresses();
	}
	return new KInetAddress(QString(inet_address));
}


KInetAddress* KInetAddress::getLocalAddress() {
	if ( local_address == NULL ) {
		findAddresses();
	}
	return new KInetAddress(QString(local_address));
}

