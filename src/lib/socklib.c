/*
 *  Author:
 *  Sasikanth.V        <sasikanth@email.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#include "common_types.h"
#include "sockets.h"
#include "socks.h"

#ifdef UNDER_DEV

#define MAX_SOCK_LAYER 12

struct sock_layer {
	sync_lock_t  lock;
	int          sock_id;
	int          familiy;
	int 	     flags;
	uint32_t     addr;
	int          index;
}SOCK_T;

static SOCK_T *sock_layer[MAX_SOCK_LAYER];
static sync_lock_t  lock;

struct sock_client {
	int       familiy;
	int       sock
	uint32_t  addr;
	int       flags;
	union proto {    
		struct sock_tcp {
			int port;
			int listen;
		}tcp;
		struct sock_udp {
			int  port;
		}udp;
	}p;
	int       protocol;
};

enum {
	SOCK_CLIENT_BIND  = 0x1,
	SOCK_CLIENT_CONNECT = 0x2,
	SOCK_CLIENT_NEW_THREAD = 0x4,
	SOCK_CLIENT_BLOCKING = 0x8,
};

int sock_client_init (struct sock_client *client)
{
	if (!client)
		return 0;
	client->familiy    = -1;
	client->flags      =  0;
	client->addr       =  0;
}

int sock_client_register (struct sock_client *new)
{
	sync_lock (&lock);


	sync_unlock (&lock);
}

/*Caller must hold the main sock layer lock*/
SOCK_T * alloc_new_sock_layer  (void)
{
	int i = MAX_SOCK_LAYER;
	
	while (i >= 0) {
		if (!sock_layer[i]) {
			SOCK_T *p = tm_calloc (1, sizeof (SOCK_T));
			if (!p) {
				printf ("Error : Out of memory");
				return NULL;
			}
			sock_layer[i] = p;
			return p;
		}
	}
	return NULL;
}

void release_sock_layer (SOCK_T *p)
{
	if (p) {
		sock_layer[p->index] = NULL;
		tm_free (p, sizeof (*p));
	}
}

#endif

int udp_v4_create (struct sockaddr_in *addr)
{
	int sock = -1;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (!sock) {
		perror ("Socket : ");
		return -1;
	}

	if(bind(sock, (struct sockaddr *) addr, sizeof(struct sockaddr))) {
		close (sock);
		return -1;
	}

	return sock;
}

int udp_v4_connect (int sock, struct sockaddr_in *addr)
{
	int rval = -1;
	int size = sizeof(struct sockaddr);

	if (!getsockname(sock, (struct sockaddr *) addr, &size))
		if (!connect(sock, (struct sockaddr *) addr, sizeof(struct sockaddr)))
			rval = 0;

	return rval;
}

int udp_close (int sock)
{
	close (sock);
	return 0;
}
