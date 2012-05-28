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

static tmtaskid_t dhcptaskid = -1;
static int        dhcpqpool = -1;
static unsigned long dhcp_pqid = -1;
static struct udp_pcb     *pcb;

#define DHCP_RX_PKT      0x1
#define DHCP_PQ_MSG      32

struct dhcpd_msg {
	int32_t            msg_type;
	void               *data;
};

static int dhcp_queue_packet (struct interface *netif, void *data)
{
	struct dhcpd_msg *new = NULL;	

	new = alloc_block (dhcpqpool);

	if (!new) {
		return -1;
	}

	new->msg_type = DHCP_RX_PKT;
	new->data = data;

	if (queue_packet (dhcp_pqid, (void *)new, sizeof(new)) < 0) {
		free_blk (dhcpqpool, new);
		return -1;
	}
	return 0;
}
static void
dhcpd_recv (void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t * addr,
           u16_t port)
{
    struct interface       *netif = (struct interface *) arg;
    if (dhcp_queue_packet (netif, p->payload) < 0) {
	   pbuf_free (p);
    }
}

static void * dhcpd_task (void *arg)
{
	struct dhcpd_msg *msg = NULL;
	
	void (*dhcpd_msg_call_back[]) (void *p) = {NULL, dhcpd_process_packet};

	while (1) {

		if (dequeue_packet (dhcp_pqid, (void *)&msg, sizeof (msg), 0, 0) < 0)
			continue;

		if (dhcpd_msg_call_back [msg->msg_type])
			dhcpd_msg_call_back[msg->msg_type] (msg->data);
		
		free_blk (dhcpqpool, msg);
	}
}

int dhcpd_start (void)
{
	pcb = udp_new ();
	udp_bind (pcb, IP_ADDR_ANY, DHCP_SERVER_PORT);
	udp_connect (pcb, IP_ADDR_ANY, DHCP_CLIENT_PORT);
	udp_recv (pcb, dhcpd_recv, NULL);

	udhcpd_init ();
	read_config (NULL);
	return 0;
}

int dhcpd_init (void)
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

	if (task_create ("dhcpd", 30, 3, 20 * 1024, dhcpd_task, NULL, NULL, 
			 &dhcptaskid) == TSK_FAILURE) {
		printf ("Task creation failed : %s\n", "dhcpd");
		return -1;
	}
	return 0;
}
