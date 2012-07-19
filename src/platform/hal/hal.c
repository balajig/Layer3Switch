/*
 *  Author:
 *  Sasikanth.V        <sasikanth@email.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "common_types.h"
#include "ifmgmt.h"
#include "lwip/pbuf.h"

#define IS_MAC_UCAST_MAC(addr)  (!(addr[0] & 0xFF))

int hal_interface_down(struct interface *fp);
int hal_interface_up(struct interface *fp);
extern int make_if_down (struct interface *fp);
extern int make_if_up (struct interface *fp);
void process_pkt (void  *pkt, int len, uint16_t port);
int stp_rcv_bpdu (void *pkt, int port, int vlanid, int len);
int mac_address_update (MACADDRESS mac_addr, int32_t port_no, uint16_t vlan_id);
int is_dest_stp_group_address (MACADDRESS mac);
struct pbuf * allocate_and_cpy_buf_2_pbuf (uint8_t *buf, int len);
err_t ethernet_input (struct pbuf *p, struct interface *netif);

int hal_interface_down (struct interface *fp)
{
	if (make_if_down (fp) < 0) {
		printf ("Unable to make interface down\n");
		return -1;
	}
	return 0;
}

int hal_interface_up (struct interface *fp)
{
	if (make_if_up (fp) < 0) {
		printf ("Unable to make interface UP\n");
		return -1;
	}
	return 0;
}

struct pbuf * allocate_and_cpy_buf_2_pbuf (uint8_t *buf, int len)
{
    struct pbuf        *p = NULL;

    p = pbuf_alloc (PBUF_RAW, len, PBUF_POOL);

    if (p != NULL)
    {
	if (pbuf_take (p, buf, len) != ERR_OK) {
		pbuf_free (p);
		return NULL;	
	}
    }
    return p;
}

void process_pkt (void  *pkt, int len, uint16_t port)
{
	struct ether_hdr *eth = (struct ether_hdr *)pkt;
    	struct pbuf        *p = NULL;
	int vlanid = 1;

	if ((IF_OPER_STATUS(port) == IF_DOWN) ||
	    (IF_ADMIN_STATUS(port) == IF_DOWN)) {
		return;
	}

	if (IS_MAC_UCAST_MAC (eth->smac.addr)) {
	 	mac_address_update (eth->smac, (uint32_t)port, 1);
#if 0
		if (unknown_umac (eth->dmac)) {
			/*flood the packet*/
		}
#endif
	} 
	if (is_dest_stp_group_address (eth->dmac)) {
		stp_rcv_bpdu (pkt, port, vlanid, len);
		return;
	}

#ifdef CONFIG_OPENSWITCH_TCP_IP
    	p = allocate_and_cpy_buf_2_pbuf (pkt, len);
	if (p)
		ethernet_input (p, IF_INFO (port));
#endif
}
