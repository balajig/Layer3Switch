/*
 *  Author:
 *  Sasikanth.V        <sasikanth@email.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

/*this is based on atinom*/
#include "common_types.h"
#include "sntp.h"
#include "opt.h"
#include "udp.h"
#include "ip_addr.h"

#undef SNTP_DEBUG

static void         sntp_recv (void *arg, struct udp_pcb *pcb, struct pbuf *p,
                               ip_addr_t * addr, u16_t port);

static struct udp_pcb *sntp_pcb = NULL;

int sntp_init (void)
{
    sntp_pcb = udp_new ();

    if (sntp_pcb != NULL)
    {
        udp_recv (sntp_pcb, sntp_recv, (void *) SNTP_UDP_PORT);
        udp_bind (sntp_pcb, IP_ADDR_ANY, SNTP_UDP_PORT);
	return 0;
    }
    return -1;
}

static void  sntp_recv (void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t * addr, u16_t port)
{
    ntphdr_t           *reply_msg = (ntphdr_t *) p->payload;

    LWIP_UNUSED_ARG (arg);

    LWIP_DEBUGF (SNTP_DEBUG | LWIP_DBG_TRACE,
                 ("dhcp_recv(pbuf = %p) from SNTP server %" U16_F ".%" U16_F
                  ".%" U16_F ".%" U16_F " port %" U16_F "\n", (void *) p,
                  ip4_addr1_16 (addr), ip4_addr2_16 (addr), ip4_addr3_16 (addr),
                  ip4_addr4_16 (addr), port));
    LWIP_DEBUGF (SNTP_DEBUG | LWIP_DBG_TRACE,
                 ("pbuf->len = %" U16_F "\n", p->len));
    LWIP_DEBUGF (SNTP_DEBUG | LWIP_DBG_TRACE,
                 ("pbuf->tot_len = %" U16_F "\n", p->tot_len));
}
