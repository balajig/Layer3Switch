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

struct sock_client {
	int       type;
	int       familiy;
	uint32_t  addr;
	int       flags;
	union __sock {    
		struct sock_tcp {
			int port;
			int listen;
		}tcp;
		struct sock_udp {
			int       lport;
			int       rport
			uint32_t  raddr;
		}udp;
	}_sock;
	int       protocol;
	int       (*sock_data_cb) (void *data, void *arg);
};

enum {
	SOCK_CLIENT_BIND  = 0x1,
	SOCK_CLIENT_CONNECT = 0x2,
	SOCK_CLIENT_NEW_THREAD = 0x4,
	SOCK_CLIENT_BLOCKING = 0x8,
};

struct sock_layer {
	sync_lock_t         lock;
	int                 sock_id;
	struct sock_client  client;	
}SOCK_T;

static SOCK_T *sock_layer[MAX_SOCK_LAYER];
static sync_lock_t  lock;
static tmtaskid_t sockmgrid;

static fd_set

static void sockmgr (void *arg)
{
	fd_set rfds;
	struct timeval tv;
	int retval;

	while (1) {


	}
}

int sock_mgr_init (void)
{
	if (task_create ("SockMgr", 90, 3, 20 * 1024, sockmgr, NULL, NULL, 
			 &sockmgrid) == TSK_FAILURE) {
		printf ("Task creation failed : SockMgr\n", );
		return -1;
	}
}

void sock_client_init (struct sock_client *client)
{
	if (!client)
		return 0;
	memset (client, 0, sizeof (*client));
}

int sock_client_register (struct sock_client *client)
{
	SOCK_T  *new;

	if (!client)
		return -1;

	if ((client->familiy < AF_INET) || (client->familiy > AF_MAX)) {
		return -1;
	}

	sync_lock (&lock);

	sync_unlock (&lock);
}

static int tcp_sock_register ()
{


}

static int udp_sock_register (struct sock_client *client)
{
        int sock = -1;
	struct sockaddr_in addr;

        sock = socket(client->familiy, SOCK_DGRAM, IPPROTO_UDP);

        if (!sock) {
                perror ("Socket : ");
                return -1;
        }

	if (client->flags & SOCK_CLIENT_BIND) {
		memset (&addr, 0, sizeof (addr));
		addr.sin_family = client->familiy;
		addr.sin_port = htons (client->_sock.udp.lport);
		addr.sin_addr.s_addr = client->addr;

		if(bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr))) {
			close (sock);
                	perror ("Bind : ");
			return -1;
		}
	}

	if (client->flags & SOCK_CLIENT_CONNECT) {
		memset (&addr, 0, sizeof (addr));

		addr.sin_family = client->familiy;
		addr.sin_port = htons (client->_sock.udp.rport);
		addr.sin_addr.s_addr = client->_sock.udp.raddr;

		if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr))) {
			close (sock);
                	perror ("connect : ");
			return -1;
		}
	}

	return sock;
}

/*Caller must hold the main sock layer lock*/
static SOCK_T * alloc_new_sock_layer  (void)
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

static void release_sock_layer (SOCK_T *p)
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
