/*
 *  Author:
 *  Sasikanth.V        <sasikanth@email.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#include "common.h"
#include "ifmgmt.h"

int dhcpd_process_packet (struct dhcp_packet *packet);
int dhcpd_start (void);
int udp_v4_create (struct sockaddr_in *addr);
int udhcpd_init (void);
void read_config(const char *file);
int dhcpd_init (void);

static tmtaskid_t dhcptaskid = -1;

static int dhsock = -1;

static void * dhcpd_task (void *arg UNUSED_PARAM)
{
	struct dhcp_packet packet;
	struct sockaddr_in fromsock;
	socklen_t  fromlen = sizeof(fromsock);
	ssize_t len = 0;

	while (1) {
		len = recvfrom (dhsock, &packet, sizeof (packet), 0, 
				(struct sockaddr *)&fromsock, &fromlen);

		if (len > 0) {
			dhcpd_process_packet (&packet);
		}

	}
	return NULL;
}

int dhcpd_start (void)
{
	struct sockaddr_in addr;

	memset (&addr, 0, sizeof (addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons (DHCP_SERVER_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	dhsock = udp_v4_create (&addr);

	udhcpd_init ();

	read_config (NULL);

	if (task_create ("dhcpd", 30, 3, 20 * 1024, dhcpd_task, NULL, NULL, 
			 &dhcptaskid) == TSK_FAILURE) {
		printf ("Task creation failed : %s\n", "dhcpd");
		return -1;
	}

	return 0;
}

int dhcpd_init (void)
{
	return 0;
}
