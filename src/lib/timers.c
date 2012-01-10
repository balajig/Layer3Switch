/**
 * @file
 * Stack-internal timers implementation.
 * This file includes timer callbacks for stack-internal timers as well as
 * functions to set up or stop timers and check for expired timers.
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
 *         Simon Goldschmidt
 *
 */

#include "opt.h"

#include "timers.h"
#include "tcp_impl.h"
#include "def.h"
#include "memp.h"
#include "tcpip.h"

#include "ip_frag.h"
#include "netif/etharp.h"
#include "dhcp.h"
#include "autoip.h"
#include "igmp.h"
#include "dns.h"

#if LWIP_TCP
/** global variable that shows if the tcp timer is currently scheduled or not */
static int          tcpip_tcp_timer_active;

/**
 * Timer callback function that calls tcp_tmr() and reschedules itself.
 *
 * @param arg unused argument
 */
static void
tcpip_tcp_timer (void *arg)
{
    LWIP_UNUSED_ARG (arg);

    /* call TCP timer handler */
    tcp_tmr ();
    /* timer still needed? */
    if (tcp_active_pcbs || tcp_tw_pcbs)
    {
        /* restart timer */
        //sys_timeout (TCP_TMR_INTERVAL, tcpip_tcp_timer, NULL);
    }
    else
    {
        /* disable timer */
        tcpip_tcp_timer_active = 0;
    }
}

/**
 * Called from TCP_REG when registering a new PCB:
 * the reason is to have the TCP timer only running when
 * there are active (or time-wait) PCBs.
 */
void
tcp_timer_needed (void)
{
    /* timer is off but needed again? */
    if (!tcpip_tcp_timer_active && (tcp_active_pcbs || tcp_tw_pcbs))
    {
        /* enable and start timer */
        tcpip_tcp_timer_active = 1;
        //sys_timeout (TCP_TMR_INTERVAL, tcpip_tcp_timer, NULL);
    }
}
#endif /* LWIP_TCP */

#if IP_REASSEMBLY
/**
 * Timer callback function that calls ip_reass_tmr() and reschedules itself.
 *
 * @param arg unused argument
 */
static void
ip_reass_timer (void *arg)
{
    LWIP_UNUSED_ARG (arg);
    LWIP_DEBUGF (TIMERS_DEBUG, ("tcpip: ip_reass_tmr()\n"));
    ip_reass_tmr ();
    //sys_timeout (IP_TMR_INTERVAL, ip_reass_timer, NULL);
}
#endif /* IP_REASSEMBLY */

/** Initialize this module */
void
sys_timeouts_init (void)
{
#if IP_REASSEMBLY
    //sys_timeout (IP_TMR_INTERVAL, ip_reass_timer, NULL);
#endif /* IP_REASSEMBLY */
}
