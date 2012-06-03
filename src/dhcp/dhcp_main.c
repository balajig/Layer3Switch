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


int dhcp_init (void);
void dhcp_process (struct interface *netif, void *p);


#define DHCP_RX_PKT      0x1
#define DHCP_PQ_MSG      32

struct dhcpd_msg {
	int32_t            msg_type;
	void               *data;
	struct  interface  *netif;
};

static tmtaskid_t dhcptaskid = -1;
static int        dhcpqpool = -1;
static unsigned long dhcp_pqid = -1;

void * dhcp_task (void *arg)
{
	struct dhcpd_msg *msg = NULL;
	
	void (*dhcpd_msg_call_back[]) (struct interface *, void *p) = {NULL, dhcp_process};
									
	while (1) {

		if (dequeue_packet (dhcp_pqid, (void *)&msg, sizeof (msg), 0, 0, 0) < 0)
			continue;

		if (dhcpd_msg_call_back [msg->msg_type])
			dhcpd_msg_call_back[msg->msg_type] (msg->netif, msg->data);
		
		free_blk (dhcpqpool, msg);
	}
}

int dhcp_queue_packet (struct interface *netif, void *data)
{
	struct dhcpd_msg *new = NULL;	

	new = alloc_block (dhcpqpool);

	if (!new) {
		return -1;
	}

	new->msg_type = DHCP_RX_PKT;
	new->netif = netif;
	new->data = data;

	if (queue_packet (dhcp_pqid, (void *)new, sizeof(new)) < 0) {
		free_blk (dhcpqpool, new);
		return -1;
	}
	return 0;
}

int dhcp_init (void)
{
	dhcp_pqid = pqueue_create ();

	if (!dhcp_pqid) {
		return -1;
	}

        dhcpqpool = mem_pool_create ("DHCP", DHCP_PQ_MSG * sizeof (struct dhcpd_msg), 
                                     DHCP_PQ_MSG, 0);
        if (dhcpqpool < 0) {
		pqueue_destroy (dhcp_pqid);
                return -1;
        }

	if (task_create ("dhcp", 30, 3, 20 * 1024, dhcp_task, NULL, NULL, 
			 &dhcptaskid) == TSK_FAILURE) {
		printf ("Task creation failed : %s\n", "dhcp");
		return -1;
	}
	return 0;
}
