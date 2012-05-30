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
