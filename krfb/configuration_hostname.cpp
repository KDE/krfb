#include "configuration.h"

/*
 * Most of this code has been taken from KPhone/libdissipate's SIPUtil class.
 * Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
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
#include <qmessagebox.h>
#include <qstring.h>
/*--*/
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef USE_SOLARIS
#include <sys/sockio.h>
#endif


static char *dissipate_our_fqdn = NULL;

/* max number of network interfaces*/
#define MAX_IF 5

/* Path to the route entry in proc filesystem */
#define PROCROUTE "/proc/net/route"

/* file containing the hostname of the machine */
/* This is the name for slackware and redhat */

#define HOSTFILE "/etc/HOSTNAME"

/* and this is the name for debian */
/* #define HOSTFILE "/etc/HOSTNAME" */

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


void findFqdn( void )
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
	if ( err < 0 ) printf( "Error in ioctl: %i.\n", errno );
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
		dissipate_our_fqdn = NULL;
		return;
	}
	if(j == 1) {
		dissipate_our_fqdn = strdup( if_addr[0] );
		return;
	}

	default_ifName = getdefaultdev();
	if( default_ifName != NULL) {
		for( i = 0; i < j; i++ ) {
			if( strcmp( if_name[i], default_ifName ) == 0 ) {
				dissipate_our_fqdn = strdup( if_addr[i] );
				return;
			}
		}
	}
	dissipate_our_fqdn = strdup( if_addr[0] );
}

QString Configuration::hostname() const
{
	if ( dissipate_our_fqdn == NULL ) {
		findFqdn();
	}

	return QString(dissipate_our_fqdn);
}

