/**
 * @file
 * lwIP network interface abstraction
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "common_types.h"
#include "lwip/opt.h"

#include "lwip/def.h"
#include "lwip/ip_addr.h"
#include "lwip/ip6_addr.h"
#include "lwip/netif.h"
#include "lwip/tcp_impl.h"
#include "lwip/snmp.h"
#include "lwip/igmp.h"
#include "netif/etharp.h"
#include "lwip/stats.h"
#if ENABLE_LOOPBACK
#include "lwip/sys.h"
#if LWIP_NETIF_LOOPBACK_MULTITHREADING
#include "lwip/tcpip.h"
#endif /* LWIP_NETIF_LOOPBACK_MULTITHREADING */
#endif /* ENABLE_LOOPBACK */

#if LWIP_AUTOIP
#include "lwip/autoip.h"
#endif /* LWIP_AUTOIP */
#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif /* LWIP_DHCP */
#if LWIP_IPV6_DHCP6
#include "lwip/dhcp6.h"
#endif /* LWIP_IPV6_DHCP6 */
#if LWIP_IPV6_MLD
#include "lwip/mld6.h"
#endif /* LWIP_IPV6_MLD */
#include "ifmgmt.h"

#if LWIP_NETIF_STATUS_CALLBACK
#define NETIF_STATUS_CALLBACK(n) do{ if (n->status_callback) { (n->status_callback)(n); }}while(0)
#else
#define NETIF_STATUS_CALLBACK(n)
#endif /* LWIP_NETIF_STATUS_CALLBACK */

#if LWIP_NETIF_LINK_CALLBACK
#define NETIF_LINK_CALLBACK(n) do{ if (n->link_callback) { (n->link_callback)(n); }}while(0)
#else
#define NETIF_LINK_CALLBACK(n)
#endif /* LWIP_NETIF_LINK_CALLBACK */


void ethernetif_init (struct interface *netif);
int if_lo_setup (int);
err_t low_level_output (struct interface *netif, struct pbuf *p);
int if_zebra_new_hook (struct interface *ifp);;

#if LWIP_HAVE_LOOPIF
static struct interface loop_if;

struct interface * get_loopback_if (void)
{
	return &loop_if;
}
err_t if_loopif_init(void)
{
  struct interface  *netif = &loop_if;

  ip_addr_t loop_ipaddr, loop_netmask, loop_gw;
  IP4_ADDR(&loop_gw, 127,0,0,1);
  IP4_ADDR(&loop_ipaddr, 127,0,0,1);
  IP4_ADDR(&loop_netmask, 255,0,0,0);

  /* initialize the snmp variables and counters inside the struct interface
 *    * ifSpeed: no assumption can be made!
 *       */
  //NETIF_INIT_SNMP(netif, snmp_ifType_softwareLoopback, 0);

  netif->ifDescr[0] = 'l';
  netif->ifDescr[1] = 'o';
  netif->ifDescr[2] = '\0';
  netif->output = netif_loop_output;

  if_set_addr (netif, &loop_ipaddr, &loop_netmask, &loop_gw);

  //if_lo_init ();
  return ERR_OK;
}

#endif

err_t
low_level_output (struct interface *netif, struct pbuf *p)
{
    struct pbuf        *q;
    int port = netif->ifIndex;
    uint8_t  *packet = malloc (netif->ifMtu);
    int len = 0;

    if (!packet)
	return ERR_MEM;

#if ETH_PAD_SIZE
    pbuf_header (p, -ETH_PAD_SIZE);    /* drop the padding word */
#endif

    for (q = p; q != NULL; q = q->next)
    {
	memcpy (packet + len, q->payload, q->len);
	len += q->len;
    }
    send_packet (packet, port, len);


#if ETH_PAD_SIZE
    pbuf_header (p, ETH_PAD_SIZE);    /* reclaim the padding word */
#endif

   free (packet);

    return ERR_OK;
}



void interface_init (struct interface *netif, void *state, netif_input_fn input)
{
    static u8_t         netifnum = 0;

#if LWIP_IPV6
  u32_t i;
#endif

  /* reset new interface configuration state */
  ip_addr_set_zero(&netif->ip_addr);
  ip_addr_set_zero(&netif->netmask);
  ip_addr_set_zero(&netif->gw);
#if LWIP_IPV6
  for (i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
    ip6_addr_set_zero(&netif->ip6_addr[i]);
    netif_ip6_addr_set_state(netif, i, IP6_ADDR_INVALID);
  }
#endif /* LWIP_IPV6 */

    netif->flags = 0;
#if LWIP_DHCP
    /* netif not under DHCP control by default */
    netif->dhcp = NULL;
#endif /* LWIP_DHCP */
#if LWIP_AUTOIP
    /* netif not under AutoIP control by default */
    netif->autoip = NULL;
#endif /* LWIP_AUTOIP */
#if LWIP_IPV6_AUTOCONFIG
  /* IPv6 address autoconfiguration not enabled by default */
  netif->ip6_autoconfig_enabled = 0;
#endif /* LWIP_IPV6_AUTOCONFIG */
#if LWIP_IPV6_SEND_ROUTER_SOLICIT
  netif->rs_count = LWIP_ND6_MAX_MULTICAST_SOLICIT;
#endif /* LWIP_IPV6_SEND_ROUTER_SOLICIT */
#if LWIP_IPV6_DHCP6
  /* netif not under DHCPv6 control by default */
  netif->dhcp6 = NULL;
#endif /* LWIP_IPV6_DHCP6 */
#if LWIP_NETIF_STATUS_CALLBACK
    netif->status_callback = NULL;
#endif /* LWIP_NETIF_STATUS_CALLBACK */
#if LWIP_NETIF_LINK_CALLBACK
    netif->link_callback = NULL;
#endif /* LWIP_NETIF_LINK_CALLBACK */
#if LWIP_IGMP
    netif->igmp_mac_filter = NULL;
#endif /* LWIP_IGMP */
#if LWIP_IPV6 && LWIP_IPV6_MLD
  netif->mld_mac_filter = NULL;
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */
#if ENABLE_LOOPBACK
    netif->loop_first = NULL;
    netif->loop_last = NULL;
#endif /* ENABLE_LOOPBACK */

    /* remember netif specific state information data */
    netif->state = state;
    netif->num = netifnum++;
    netif->input = input;
#if LWIP_NETIF_HWADDRHINT
    netif->addr_hint = NULL;
#endif /* LWIP_NETIF_HWADDRHINT */
#if ENABLE_LOOPBACK && LWIP_LOOPBACK_MAX_PBUFS
    netif->loop_cnt_current = 0;
#endif /* ENABLE_LOOPBACK && LWIP_LOOPBACK_MAX_PBUFS */

#ifdef CONFIG_OPENSWITCH_TCP_IP
    ethernetif_init (netif);
#else
    netif->linkoutput = low_level_output;
#endif

    if_zebra_new_hook (netif);

#if 0
    /* call user specified initialization function for netif */
    if (init (netif) != ERR_OK)
    {
        return NULL;
    }
#endif

    /* add this netif to the list */
#ifdef CONFIG_OPENSWITCH_TCP_IP
    snmp_inc_iflist ();
#endif

#if 0
    /* start IGMP processing */
    if (netif->flags & NETIF_FLAG_IGMP)
    {
        igmp_start (netif);
    }
#endif /* LWIP_IGMP */

    LWIP_DEBUGF (NETIF_DEBUG, ("netif: added interface %c%c IP addr ",
                               netif->ifDescr[0], netif->ifDescr[1]));
    LWIP_DEBUGF (NETIF_DEBUG, ("\n"));
    return;
}

/**
 * Change IP address configuration for a network interface (including netmask
 * and default gateway).
 *
 * @param netif the network interface to change
 * @param ipaddr the new IP address
 * @param netmask the new netmask
 * @param gw the new default gateway
 */
void
if_set_addr (struct interface *netif, ip_addr_t * ipaddr, ip_addr_t * netmask,
                ip_addr_t * gw)
{
    if_set_ipaddr (netif, ipaddr);
    if_set_netmask (netif, netmask);
    if_set_gw (netif, gw);
}
/**
 * Change the IP address of a network interface
 *
 * @param netif the network interface to change
 * @param ipaddr the new IP address
 *
 * @note call if_set_addr() if you also want to change netmask and
 * default gateway
 */
void
if_set_ipaddr (struct interface *netif, ip_addr_t * ipaddr)
{
    /* TODO: Handling of obsolete pcbs */
    /* See:  http://mail.gnu.org/archive/html/lwip-users/2003-03/msg00118.html */
#if LWIP_TCP
    struct tcp_pcb     *pcb;
    struct tcp_pcb_listen *lpcb;

  /* address is actually being changed? */
  if (ipaddr && (ip_addr_cmp(ipaddr, &(netif->ip_addr))) == 0) {
    /* extern struct tcp_pcb *tcp_active_pcbs; defined by tcp.h */
    LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_STATE, ("netif_set_ipaddr: netif address being changed\n"));
    pcb = tcp_active_pcbs;
    while (pcb != NULL) {
      /* PCB bound to current local interface address? */
      if (ip_addr_cmp(ipX_2_ip(&pcb->local_ip), &(netif->ip_addr))
#if LWIP_AUTOIP
        /* connections to link-local addresses must persist (RFC3927 ch. 1.9) */
        && !ip_addr_islinklocal(ipX_2_ip(&pcb->local_ip))
#endif /* LWIP_AUTOIP */
        ) {
        /* this connection must be aborted */
        struct tcp_pcb *next = pcb->next;
        LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_STATE, ("netif_set_ipaddr: aborting TCP pcb %p\n", (void *)pcb));
        tcp_abort(pcb);
        pcb = next;
      } else {
        pcb = pcb->next;
      }
    }
    for (lpcb = tcp_listen_pcbs.listen_pcbs; lpcb != NULL; lpcb = lpcb->next) {
      /* PCB bound to current local interface address? */
      if ((!(ip_addr_isany(ipX_2_ip(&lpcb->local_ip)))) &&
          (ip_addr_cmp(ipX_2_ip(&lpcb->local_ip), &(netif->ip_addr)))) {
        /* The PCB is listening to the old ipaddr and
         * is set to listen to the new one instead */
        ip_addr_set(ipX_2_ip(&lpcb->local_ip), ipaddr);
      }
    }
  }
#endif
  snmp_delete_ipaddridx_tree(netif);
  snmp_delete_iprteidx_tree(0,netif);
  /* set new IP address to netif */
  ip_addr_set(&(netif->ip_addr), ipaddr);
  snmp_insert_ipaddridx_tree(netif);
  snmp_insert_iprteidx_tree(0,netif);

  LWIP_DEBUGF(NETIF_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE, ("netif: IP address of interface %c%c set to %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
    netif->name[0], netif->name[1],
    ip4_addr1_16(&netif->ip_addr),
    ip4_addr2_16(&netif->ip_addr),
    ip4_addr3_16(&netif->ip_addr),
    ip4_addr4_16(&netif->ip_addr)));
}

/**
 * Change the default gateway for a network interface
 *
 * @param netif the network interface to change
 * @param gw the new default gateway
 *
 * @note call if_set_addr() if you also want to change ip address and netmask
 */
void
if_set_gw (struct interface *netif, ip_addr_t * gw)
{
    ip_addr_set (&(netif->gw), gw);
    LWIP_DEBUGF (NETIF_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE,
                 ("netif: GW address of interface %c%c set to %" U16_F ".%"
                  U16_F ".%" U16_F ".%" U16_F "\n", netif->ifDescr[0],
                  netif->ifDescr[1], ip4_addr1_16 (&netif->gw),
                  ip4_addr2_16 (&netif->gw), ip4_addr3_16 (&netif->gw),
                  ip4_addr4_16 (&netif->gw)));
}

/**
 * Change the netmask of a network interface
 *
 * @param netif the network interface to change
 * @param netmask the new netmask
 *
 * @note call if_set_addr() if you also want to change ip address and
 * default gateway
 */
void
if_set_netmask (struct interface *netif, ip_addr_t * netmask)
{
#ifdef CONFIG_OPENSWITCH_TCP_IP
    snmp_delete_iprteidx_tree (0, netif);
#endif
    /* set new netmask to netif */
    ip_addr_set (&(netif->netmask), netmask);
#ifdef CONFIG_OPENSWITCH_TCP_IP
    snmp_insert_iprteidx_tree (0, netif);
#endif
    LWIP_DEBUGF (NETIF_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_STATE,
                 ("netif: netmask of interface %c%c set to %" U16_F ".%" U16_F
                  ".%" U16_F ".%" U16_F "\n", netif->ifDescr[0], netif->ifDescr[1],
                  ip4_addr1_16 (&netif->netmask),
                  ip4_addr2_16 (&netif->netmask),
                  ip4_addr3_16 (&netif->netmask),
                  ip4_addr4_16 (&netif->netmask)));
}

/**
 * Bring an interface up, available for processing
 * traffic.
 * 
 * @note: Enabling DHCP on a down interface will make it come
 * up once configured.
 * 
 * @see dhcp_start()
 */
void
if_set_up (struct interface *netif)
{
    if (!(netif->flags & NETIF_FLAG_UP))
    {
        netif->flags |= NETIF_FLAG_UP;

#if LWIP_SNMP
        snmp_get_sysuptime (&netif->ts);
#endif /* LWIP_SNMP */

        NETIF_STATUS_CALLBACK (netif);

        if (netif->flags & NETIF_FLAG_LINK_UP)
        {
#ifdef CONFIG_OPENSWITCH_TCP_IP
#if LWIP_ARP
            /* For Ethernet network interfaces, we would like to send a "gratuitous ARP" */
            if (netif->flags & (NETIF_FLAG_ETHARP))
            {
                etharp_gratuitous (netif);
            }
#endif /* LWIP_ARP */
#endif

#if 0
            /* resend IGMP memberships */
            if (netif->flags & NETIF_FLAG_IGMP)
            {
                igmp_report_groups (netif);
            }
#endif /* LWIP_IGMP */
#if LWIP_IPV6 && LWIP_IPV6_MLD
      /* send mld memberships */
      mld6_report_groups( netif);
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

#if LWIP_IPV6_SEND_ROUTER_SOLICIT
      /* Send Router Solicitation messages. */
      netif->rs_count = LWIP_ND6_MAX_MULTICAST_SOLICIT;
#endif /* LWIP_IPV6_SEND_ROUTER_SOLICIT */

    }
  }
}

/**
 * Bring an interface down, disabling any traffic processing.
 *
 * @note: Enabling DHCP on a down interface will make it come
 * up once configured.
 * 
 * @see dhcp_start()
 */
void
netif_set_down (struct interface *netif)
{
    if (netif->flags & NETIF_FLAG_UP)
    {
        netif->flags &= ~NETIF_FLAG_UP;
#if LWIP_SNMP
        snmp_get_sysuptime (&netif->ts);
#endif

#if LWIP_ARP
    if (netif->flags & NETIF_FLAG_ETHARP) {
      etharp_cleanup_netif(netif);
    }
#endif /* LWIP_ARP */
    NETIF_STATUS_CALLBACK(netif);
  }
}

#if LWIP_NETIF_STATUS_CALLBACK
/**
 * Set callback to be called when interface is brought up/down
 */
void
if_set_status_callback (struct interface *netif,
                           if_status_callback_fn status_callback)
{
    if (netif)
    {
        netif->status_callback = status_callback;
    }
}
#endif /* LWIP_NETIF_STATUS_CALLBACK */

/**
 * Called by a driver when its link goes up
 */
void
if_set_link_up (struct interface *netif)
{
    if (!(netif->flags & NETIF_FLAG_LINK_UP))
    {
        netif->flags |= NETIF_FLAG_LINK_UP;

#if LWIP_DHCP
        if (netif->dhcp)
        {
            dhcp_network_changed (netif);
        }
#endif /* LWIP_DHCP */

#if LWIP_AUTOIP
        if (netif->autoip)
        {
            autoip_network_changed (netif);
        }
#endif /* LWIP_AUTOIP */

        if (netif->flags & NETIF_FLAG_UP)
        {
#if LWIP_ARP
            /* For Ethernet network interfaces, we would like to send a "gratuitous ARP" */
            if (netif->flags & NETIF_FLAG_ETHARP)
            {
                etharp_gratuitous (netif);
            }
#endif /* LWIP_ARP */

#if LWIP_IGMP
            /* resend IGMP memberships */
            if (netif->flags & NETIF_FLAG_IGMP)
            {
                igmp_report_groups (netif);
            }
#endif /* LWIP_IGMP */
#if LWIP_IPV6 && LWIP_IPV6_MLD
      /* send mld memberships */
      mld6_report_groups( netif);
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */
    }
    NETIF_LINK_CALLBACK(netif);
  }
}

/**
 * Called by a driver when its link goes down
 */
void
if_set_link_down (struct interface *netif)
{
    if (netif->flags & NETIF_FLAG_LINK_UP)
    {
        netif->flags &= ~NETIF_FLAG_LINK_UP;
        NETIF_LINK_CALLBACK (netif);
    }
}

#if LWIP_NETIF_LINK_CALLBACK
/**
 * Set callback to be called when link is brought up/down
 */
void
if_set_link_callback (struct interface *netif,
                         if_status_callback_fn link_callback)
{
    if (netif)
    {
        netif->link_callback = link_callback;
    }
}
#endif /* LWIP_NETIF_LINK_CALLBACK */

#if ENABLE_LOOPBACK
/**
 * Send an IP packet to be received on the same netif (loopif-like).
 * The pbuf is simply copied and handed back to netif->input.
 * In multithreaded mode, this is done directly since netif->input must put
 * the packet on a queue.
 * In callback mode, the packet is put on an internal queue and is fed to
 * netif->input by if_poll().
 *
 * @param netif the lwip network interface structure
 * @param p the (IP) packet to 'send'
 * @param ipaddr the ip address to send the packet to (not used)
 * @return ERR_OK if the packet has been sent
 *         ERR_MEM if the pbuf used to copy the packet couldn't be allocated
 */
err_t
netif_loop_output (struct interface *netif, struct pbuf *p, ip_addr_t * ipaddr)
{
    struct pbuf        *r;
    err_t               err;
    //struct pbuf        *last;
#if LWIP_LOOPBACK_MAX_PBUFS
    u8_t                clen = 0;
#endif /* LWIP_LOOPBACK_MAX_PBUFS */
    /* If we have a loopif, SNMP counters are adjusted for it,
     * if not they are adjusted for 'netif'. */
#if LWIP_SNMP
#if LWIP_HAVE_LOOPIF
    struct interface       *stats_if = &loop_if;
#else /* LWIP_HAVE_LOOPIF */
    struct interface       *stats_if = netif;
#endif /* LWIP_HAVE_LOOPIF */
#endif /* LWIP_SNMP */

    SYS_ARCH_DECL_PROTECT (lev);
    LWIP_UNUSED_ARG (ipaddr);

    /* Allocate a new pbuf */
    r = pbuf_alloc (PBUF_LINK, p->tot_len, PBUF_RAM);
    if (r == NULL)
    {
        LINK_STATS_INC (link.memerr);
        LINK_STATS_INC (link.drop);
#ifdef CONFIG_OPENSWITCH_TCP_IP
        snmp_inc_ifoutdiscards (stats_if);
#endif
        return ERR_MEM;
    }
#if 0
#if LWIP_LOOPBACK_MAX_PBUFS
    clen = pbuf_clen (r);
    /* check for overflow or too many pbuf on queue */
    if (((netif->loop_cnt_current + clen) < netif->loop_cnt_current) ||
        ((netif->loop_cnt_current + clen) > LWIP_LOOPBACK_MAX_PBUFS))
    {
        pbuf_free (r);
        LINK_STATS_INC (link.memerr);
        LINK_STATS_INC (link.drop);
        snmp_inc_ifoutdiscards (stats_if);
        return ERR_MEM;
    }
    netif->loop_cnt_current += clen;
#endif /* LWIP_LOOPBACK_MAX_PBUFS */
#endif

    /* Copy the whole pbuf queue p into the single pbuf r */
    if ((err = pbuf_copy (r, p)) != ERR_OK)
    {
        pbuf_free (r);
        LINK_STATS_INC (link.memerr);
        LINK_STATS_INC (link.drop);
#ifdef CONFIG_OPENSWITCH_TCP_IP
        snmp_inc_ifoutdiscards (stats_if);
#endif
        return err;
    }

#ifdef CONFIG_OPENSWITCH_TCP_IP
    /* loopback packets are always IP packets! */
    if (ip_input (r, netif) != ERR_OK)
    {
	    pbuf_free (r);
    }
#else

#endif

#if 0
    /* Put the packet on a linked list which gets emptied through calling
       if_poll(). */

    /* let last point to the last pbuf in chain r */
    for (last = r; last->next != NULL; last = last->next);

    SYS_ARCH_PROTECT (lev);
    if (netif->loop_first != NULL)
    {
        LWIP_ASSERT ("if first != NULL, last must also be != NULL",
                     netif->loop_last != NULL);
        netif->loop_last->next = r;
        netif->loop_last = last;
    }
    else
    {
        netif->loop_first = r;
        netif->loop_last = last;
    }
    SYS_ARCH_UNPROTECT (lev);
#endif
    LINK_STATS_INC (link.xmit);
#ifdef CONFIG_OPENSWITCH_TCP_IP
    snmp_add_ifoutoctets (stats_if, p->tot_len);
    snmp_inc_ifoutucastpkts (stats_if);
#endif

#if 0
#if LWIP_NETIF_LOOPBACK_MULTITHREADING
    /* For multithreading environment, schedule a call to if_poll */
    tcpip_callback ((tcpip_callback_fn) if_poll, netif);
#endif /* LWIP_NETIF_LOOPBACK_MULTITHREADING */
#endif
    return ERR_OK;
}

/**
 * Call if_poll() in the main loop of your application. This is to prevent
 * reentering non-reentrant functions like tcp_input(). Packets passed to
 * if_loop_output() are put on a list that is passed to netif->input() by
 * if_poll().
 */
#if 0
void
if_poll (struct interface *netif)
{
    struct pbuf        *in;
    /* If we have a loopif, SNMP counters are adjusted for it,
     * if not they are adjusted for 'netif'. */
#if LWIP_SNMP
#if LWIP_HAVE_LOOPIF
    struct interface       *stats_if = &loop_if;
#else /* LWIP_HAVE_LOOPIF */
    struct interface       *stats_if = netif;
#endif /* LWIP_HAVE_LOOPIF */
#endif /* LWIP_SNMP */
    SYS_ARCH_DECL_PROTECT (lev);

    do
    {
        /* Get a packet from the list. With SYS_LIGHTWEIGHT_PROT=1, this is protected */
        SYS_ARCH_PROTECT (lev);
        in = netif->loop_first;
        if (in != NULL)
        {
            struct pbuf        *in_end = in;
#if LWIP_LOOPBACK_MAX_PBUFS
            u8_t                clen = pbuf_clen (in);
            /* adjust the number of pbufs on queue */
            LWIP_ASSERT ("netif->loop_cnt_current underflow",
                         ((netif->loop_cnt_current - clen) <
                          netif->loop_cnt_current));
            netif->loop_cnt_current -= clen;
#endif /* LWIP_LOOPBACK_MAX_PBUFS */
            while (in_end->len != in_end->tot_len)
            {
                LWIP_ASSERT ("bogus pbuf: len != tot_len but next == NULL!",
                             in_end->next != NULL);
                in_end = in_end->next;
            }
            /* 'in_end' now points to the last pbuf from 'in' */
            if (in_end == netif->loop_last)
            {
                /* this was the last pbuf in the list */
                netif->loop_first = netif->loop_last = NULL;
            }
            else
            {
                /* pop the pbuf off the list */
                netif->loop_first = in_end->next;
                LWIP_ASSERT ("should not be null since first != last!",
                             netif->loop_first != NULL);
            }
            /* De-queue the pbuf from its successors on the 'loop_' list. */
            in_end->next = NULL;
        }
        SYS_ARCH_UNPROTECT (lev);

        if (in != NULL)
        {
            LINK_STATS_INC (link.recv);
            snmp_add_ifinoctets (stats_if, in->tot_len);
            snmp_inc_ifinucastpkts (stats_if);
            /* loopback packets are always IP packets! */
            if (ip_input (in, netif) != ERR_OK)
            {
                pbuf_free (in);
            }
            /* Don't reference the packet any more! */
            in = NULL;
        }
        /* go on while there is a packet on the list */
    }
    while (netif->loop_first != NULL);
}
#endif
#if !LWIP_NETIF_LOOPBACK_MULTITHREADING
/**
 * Calls if_poll() for every netif on the if_list.
 */
#if 0
void
if_poll_all (void)
{
    struct interface       *netif = if_list;
    /* loop through netifs */
    while (netif != NULL)
    {
        if_poll (netif);
        /* proceed to next network interface */
        netif = netif->next;
    }
}
#endif
#endif /* !LWIP_NETIF_LOOPBACK_MULTITHREADING */
#endif /* ENABLE_LOOPBACK */
int set_ip_address (uint32_t ifindex, uint32_t ipaddress, uint32_t ipmask)
{
	ip_addr_t ipaddr, netmask;
	ipaddr.addr = ipaddress;
	netmask.addr = ipmask;
	if_set_addr (IF_INFO(ifindex), &ipaddr, &netmask, &ipaddr);
	return 0;
}

int if_lo_setup (int portnum)
{
	int mini_index = CONFIG_MAX_PHY_PORTS + portnum - 1;
	struct interface  *netif = &port_cdb[mini_index];

	if (!netif || netif->ifIndex)
		return -1;

	sprintf ((char *)netif->ifDescr, "%s%d","lo", portnum);
	netif->ifIndex = mini_index + 1;
	netif->ifType = 2;
	netif->ifMtu = 16436;
	netif->ifSpeed = 10;
	netif->ifAdminStatus = IF_UP;
	netif->ifOperStatus = IF_UP;
	netif->ifLastChange = 0;
	netif->ifInOctets = 0;
	netif->ifInUcastPkts = 0;
	netif->ifInDiscards = 0;
	netif->ifInErrors = 0;
	netif->ifInUnknownProtos = 0;
	netif->ifOutOctets = 0;
	netif->ifOutUcastPkts = 0;
	netif->ifOutDiscards = 0;
	netif->ifOutErrors = 0;
	netif->pstp_info = NULL;
	netif->flags = NETIF_FLAG_BROADCAST |  NETIF_FLAG_LINK_UP | NETIF_FLAG_LOOPBACK;
	if_zebra_new_hook (netif);
	if_connect_init (netif);
	return 0;
}
#if LWIP_IPV6
s8_t
netif_matches_ip6_addr(struct interface * netif, ip6_addr_t * ip6addr)
{
  s8_t i;
  for (i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
    if (ip6_addr_cmp(netif_ip6_addr(netif, i), ip6addr)) {
      return i;
    }
  }
  return -1;
}

void
netif_create_ip6_linklocal_address(struct interface * netif, u8_t from_mac_48bit)
{
  u8_t i, addr_index;

  /* Link-local prefix. */
  netif->ip6_addr[0].addr[0] = PP_HTONL(0xfe800000ul);
  netif->ip6_addr[0].addr[1] = 0;

  /* Generate interface ID. */
  if (from_mac_48bit) {
    /* Assume hwaddr is a 48-bit IEEE 802 MAC. Convert to EUI-64 address. Complement Group bit. */
    netif->ip6_addr[0].addr[2] = htonl((((u32_t)(netif->ifPhysAddress[0] ^ 0x02)) << 24) |
        ((u32_t)(netif->ifPhysAddress[1]) << 16) |
        ((u32_t)(netif->ifPhysAddress[2]) << 8) |
        (0xff));
    netif->ip6_addr[0].addr[3] = htonl((0xfeul << 24) |
        ((u32_t)(netif->ifPhysAddress[3]) << 16) |
        ((u32_t)(netif->ifPhysAddress[4]) << 8) |
        (netif->ifPhysAddress[5]));
  }
  else {
    /* Use hwaddr directly as interface ID. */
    netif->ip6_addr[0].addr[2] = 0;
    netif->ip6_addr[0].addr[3] = 0;

    addr_index = 3;
    for (i = 0; i < 8; i++) {
      if (i == 4) {
        addr_index--;
      }
      netif->ip6_addr[0].addr[addr_index] |= ((u32_t)(netif->ifPhysAddress[netif->ifPhysAddress_len - i - 1])) << (8 * (i & 0x03));
    }
  }

  /* Set address state. */
#if LWIP_IPV6_DUP_DETECT_ATTEMPTS
  /* Will perform duplicate address detection (DAD). */
  netif->ip6_addr_state[0] = IP6_ADDR_TENTATIVE;
#else
  /* Consider address valid. */
  netif->ip6_addr_state[0] = IP6_ADDR_PREFERRED;
#endif /* LWIP_IPV6_AUTOCONFIG */
}
#endif /* LWIP_IPV6 */
