/*
 *  Author:
 *  Sasikanth.V        <sasikanth@email.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#include "list.h"

#define SOCKET_QUEUE_SIZE       8

typedef struct _udp_socket_queue_t {
	void            *buf;
	struct  list_head nbuf;
} udp_socket_queue_t;


typedef struct udp_ctrl_blk 
{
	struct list_head   next;
	struct list_head   queue;
	EVT_T              evt;
	int32_t            sock;
	uint32_t           udpEndpointLocalAddress;
	uint32_t           udpEndpointRemoteAddress;
	uint32_t	   udpEndpointLocalAddressType;
	uint32_t	   udpEndpointRemoteAddressType;
	uint16_t           udpEndpointLocalPort;
	uint16_t           udpEndpointRemotePort;
	unsigned char   count;
}udpctrlblk_t;

