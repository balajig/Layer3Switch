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
#include "ifmgmt.h"
#include "dhcp.h"

static void dhcp_timer_coarse (void *arg)
{
    struct interface *p = (struct interface *)arg;

    dhcp_coarse_tmr (p);

    mod_timer (p->dhcp->coarse_timer, DHCP_COARSE_TIMER_SECS * tm_get_ticks_per_second ());
}

static void dhcp_timer_fine (void *arg)
{
    struct interface *p = (struct interface *)arg;

    dhcp_fine_tmr (p);

    mod_timer (p->dhcp->fine_timer, milli_secs_to_ticks (DHCP_FINE_TIMER_MSECS));
}

void dhcp_if_start_fine_timer (struct interface *p)
{
    mod_timer (p->dhcp->coarse_timer, DHCP_COARSE_TIMER_SECS * tm_get_ticks_per_second ());
}

void dhcp_if_start_coarse_timer (struct interface *p)
{
    mod_timer (p->dhcp->fine_timer, milli_secs_to_ticks (DHCP_FINE_TIMER_MSECS));
}

int dhcp_setup_if_timers (struct interface *p)
{
	setup_timer(&p->dhcp->coarse_timer, dhcp_timer_coarse, p);
	setup_timer(&p->dhcp->fine_timer, dhcp_timer_fine, p);

	return 0;
}
