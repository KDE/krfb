/*
 * libuuid - library for generating UUIDs
 * 
 * Copyright (C) 1996, 1997, 1998, 1999 Theodore Ts'o.
 * Copyright (C) 2002 Tim Jansen
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU 
 * Library General Public License.
 * %End-Header%
 *
 * 2002-12-15, tim@tjansen.de: 
 *   merged all *.c files, 
 *   replaced all function that are not needed to generate a time uuid,
 *   added createUUID()
 */

#include "uuid.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#ifndef _SVID_SOURCE
#define _SVID_SOURCE
#endif

#include <fcntl.h>
#include <errno.h>
#ifdef HAVE_SRANDOM
#define srand(x) 	srandom(x)
#define rand() 		random()
#endif

typedef unsigned char uuid_t[16];
typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int __u32;


struct uuid {
        __u32   time_low;
        __u16   time_mid;
        __u16   time_hi_and_version;
        __u16   clock_seq;
        __u8    node[6];
};

void uuid_unpack(const uuid_t in, struct uuid *uu)
{
        const __u8      *ptr = in;
        __u32           tmp;

        tmp = *ptr++;
        tmp = (tmp << 8) | *ptr++;
        tmp = (tmp << 8) | *ptr++;
        tmp = (tmp << 8) | *ptr++;
        uu->time_low = tmp;

        tmp = *ptr++;
        tmp = (tmp << 8) | *ptr++;
        uu->time_mid = tmp;

        tmp = *ptr++;
        tmp = (tmp << 8) | *ptr++;
        uu->time_hi_and_version = tmp;

        tmp = *ptr++;
        tmp = (tmp << 8) | *ptr++;
        uu->clock_seq = tmp;

        memcpy(uu->node, ptr, 6);
}

static int get_random_fd(void)
{
	struct timeval	tv;
	static int	fd = -2;
	int		i;

	if (fd == -2) {
		gettimeofday(&tv, 0);
		fd = open("/dev/urandom", O_RDONLY);
		if (fd == -1)
			fd = open("/dev/random", O_RDONLY | O_NONBLOCK);
		srand((getpid() << 16) ^ getuid() ^ tv.tv_sec ^ tv.tv_usec);
	}
	/* Crank the random number generator a few times */
	gettimeofday(&tv, 0);
	for (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
	return fd;
}


/*
 * Generate a series of random bytes.  Use /dev/urandom if possible,
 * and if not, use srandom/random.
 */
static void get_random_bytes(void *buf, int nbytes)
{
	int i, fd = get_random_fd();
	int lose_counter = 0;
	char *cp = (char *) buf;

	if (fd >= 0) {
		while (nbytes > 0) {
			i = read(fd, cp, nbytes);
			if (i <= 0) {
				if (lose_counter++ > 16)
					break;
				continue;
			}
			nbytes -= i;
			cp += i;
			lose_counter = 0;
		}
	}

	/* XXX put something better here if no /dev/random! */
	for (i = 0; i < nbytes; i++)
		*cp++ = rand() & 0xFF;
	return;
}


/* Assume that the gettimeofday() has microsecond granularity */
#define MAX_ADJUSTMENT 10

static int get_clock(__u32 *clock_high, __u32 *clock_low, __u16 *ret_clock_seq)
{
	static int			adjustment = 0;
	static struct timeval		last = {0, 0};
	static __u16			clock_seq;
	struct timeval 			tv;
	unsigned long long		clock_reg;
	
try_again:
	gettimeofday(&tv, 0);
	if ((last.tv_sec == 0) && (last.tv_usec == 0)) {
		get_random_bytes(&clock_seq, sizeof(clock_seq));
		clock_seq &= 0x1FFF;
		last = tv;
		last.tv_sec--;
	}
	if ((tv.tv_sec < last.tv_sec) ||
	    ((tv.tv_sec == last.tv_sec) &&
	     (tv.tv_usec < last.tv_usec))) {
		clock_seq = (clock_seq+1) & 0x1FFF;
		adjustment = 0;
		last = tv;
	} else if ((tv.tv_sec == last.tv_sec) &&
	    (tv.tv_usec == last.tv_usec)) {
		if (adjustment >= MAX_ADJUSTMENT)
			goto try_again;
		adjustment++;
	} else {
		adjustment = 0;
		last = tv;
	}
		
	clock_reg = tv.tv_usec*10 + adjustment;
	clock_reg += ((unsigned long long) tv.tv_sec)*10000000;
	clock_reg += (((unsigned long long) 0x01B21DD2) << 32) + 0x13814000;

	*clock_high = clock_reg >> 32;
	*clock_low = clock_reg;
	*ret_clock_seq = clock_seq;
	return 0;
}

static void uuid_pack(const struct uuid *uu, uuid_t ptr)
{
	__u32	tmp;
	unsigned char	*out = ptr;

	tmp = uu->time_low;
	out[3] = (unsigned char) tmp;
	tmp >>= 8;
	out[2] = (unsigned char) tmp;
	tmp >>= 8;
	out[1] = (unsigned char) tmp;
	tmp >>= 8;
	out[0] = (unsigned char) tmp;
	
	tmp = uu->time_mid;
	out[5] = (unsigned char) tmp;
	tmp >>= 8;
	out[4] = (unsigned char) tmp;

	tmp = uu->time_hi_and_version;
	out[7] = (unsigned char) tmp;
	tmp >>= 8;
	out[6] = (unsigned char) tmp;

	tmp = uu->clock_seq;
	out[9] = (unsigned char) tmp;
	tmp >>= 8;
	out[8] = (unsigned char) tmp;

	memcpy(out+10, uu->node, 6);
}

static void uuid_generate_time(uuid_t out)
{
	static unsigned char node_id[6];
	struct uuid uu;
	__u32	clock_mid;

	get_random_bytes(node_id, 6);
	get_clock(&clock_mid, &uu.time_low, &uu.clock_seq);
	uu.clock_seq |= 0x8000;
	uu.time_mid = (__u16) clock_mid;
	uu.time_hi_and_version = (clock_mid >> 16) | 0x1000;
	memcpy(uu.node, node_id, 6);
	uuid_pack(&uu, out);
}

static void uuid_unparse(const uuid_t uu, char *out)
{
	struct uuid uuid;

	uuid_unpack(uu, &uuid);
	sprintf(out,
		"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		uuid.time_low, uuid.time_mid, uuid.time_hi_and_version,
		uuid.clock_seq >> 8, uuid.clock_seq & 0xFF,
		uuid.node[0], uuid.node[1], uuid.node[2],
		uuid.node[3], uuid.node[4], uuid.node[5]);
}

QString createUUID() {
	char s[37];
	uuid_t uu;
	uuid_generate_time(uu);
	uuid_unparse(uu, s);
	return QString(s);
}

