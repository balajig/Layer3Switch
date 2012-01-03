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
#include "udp_ctrl.h"

#define MAX_UDP_CTRL_BLOCKS      256
#define UDP_PKT_RX_ON_SOCK       0x1 

static int udp_pool_id = -1;

static struct list_head udp_sock_list;


int udp_init (void)
{
	INIT_LIST_HEAD (&udp_sock_list);

	udp_pool_id = mem_pool_create ("UDP", MAX_UDP_CTRL_BLOCKS * sizeof(udpctrlblk_t), 
                                       MAX_UDP_CTRL_BLOCKS, 0);
	if (udp_pool_id < 0) {
		return -1;
	}

	return 0;
}

unsigned long udp_open (int family, uint16_t local_port, uint16_t remote_port, uint32_t remote_ip)
{
	udpctrlblk_t *new = NULL;

	new =  alloc_block (udp_pool_id);

	if (!new) 
		return 0;

	memset (new, 0, sizeof(new));

	INIT_LIST_HEAD (&new->next);
	INIT_LIST_HEAD (&new->queue);

	new->udpEndpointLocalAddressType = family;
	new->udpEndpointLocalPort = local_port;

	if (remote_ip)
		new->udpEndpointRemoteAddress = remote_ip;
	else
		new->udpEndpointRemoteAddress = 0xffffffff;

	new->udpEndpointRemotePort = remote_port;

	EventInit (&new->evt);

	list_add_tail (&new->next, &udp_sock_list);

	return (unsigned long)new;
}

int udp_close (unsigned long udpblk)
{
	udpctrlblk_t * p = (udpctrlblk_t *)udpblk;

	if (!p)
		return -1;

	list_del (&p->next);
	
	free_blk (udp_pool_id, p);

	return 0;
	
}

int udp_v4bind (unsigned long udpblk, uint32_t ipaddr, uint16_t local_port)
{
	udpctrlblk_t * p = (udpctrlblk_t *)udpblk;

	if (!p)
		return -1;

	p->udpEndpointLocalAddress = ipaddr;
	p->udpEndpointLocalPort = local_port;

	return 0;
}

int udp_queue_packet (unsigned long udpblk , uint8_t *buf)
{
	udpctrlblk_t * p = (udpctrlblk_t *)udpblk;
	udp_socket_queue_t  *qbuf = NULL;

	if (!p)
		return -1;

	qbuf = malloc (sizeof(udp_socket_queue_t));

	if (!qbuf)
		return -1;

	INIT_LIST_HEAD (&qbuf->nbuf);
	qbuf->buf = buf;

	list_add_tail (&qbuf->nbuf, &p->queue);

	EvtSnd (&p->evt, UDP_PKT_RX_ON_SOCK);

	return 0;
}

int udp_recvfrom (unsigned long udpblk, uint8_t **data, size_t datalen)
{
	udpctrlblk_t * p = (udpctrlblk_t *)udpblk;
	udp_socket_queue_t  *qbuf = NULL;

	if (!p)
		return -1;

	while (1) {

		int event = 0;
		if (list_empty (&p->queue)) {
			EvtRx (&p->evt, &event, UDP_PKT_RX_ON_SOCK);
			if ((event & UDP_PKT_RX_ON_SOCK))
				goto udp_packet;

		}
udp_packet:
		if (!list_empty (&p->queue)) {
			qbuf = list_first_entry (&p->queue, udp_socket_queue_t, nbuf);
			if (!qbuf)
				return -1;
			*data = qbuf->buf;
			list_del (&qbuf->nbuf);
			free (qbuf);
			break;
		}
	}

	return 0;
}

int udp_sendto (unsigned long udpblk, uint8_t *data, size_t datalen, uint32_t to_addr, uint16_t to_port)
{
	return 0;
}
