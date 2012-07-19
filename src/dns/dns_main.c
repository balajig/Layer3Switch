/*
 *  Author:
 *  Sasikanth.V        <sasikanth@email.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */


#include "lwip/opt.h"
#include "common_types.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"

struct dns_server_tbl {
        ip_addr_t addr;
        uint32_t status;
};


struct dns_server_tbl  dns_servers[DNS_MAX_SERVERS];
TIMER_ID    dns_timer;

/**
 * The DNS resolver client timer - handle retries and timeouts and should
 * be called every DNS_TMR_INTERVAL milliseconds (every second by default).
 */
static void dns_timer_handler (void *arg)
{
	dns_check_entries ();
	mod_timer (dns_timer, milli_secs_to_ticks (DNS_TMR_INTERVAL));
}

int dns_init (void)
{
	memset (dns_servers, 0, sizeof(dns_servers));

	/*Create UDP sock for DNS*/
	dns_socket_init ();

	setup_timer (&dns_timer, dns_timer_handler, NULL);

	mod_timer (dns_timer, milli_secs_to_ticks (DNS_TMR_INTERVAL));

	return 0;
}


int set_dns_server (uint32_t addr, int primary)
{
	int i = 0;

	if (primary) {
		dns_servers[i].addr.addr = addr;
		dns_servers[i].status = 1;
		return 0;
	}
	i = 1;
	while (i < DNS_MAX_SERVERS) {
		if (!dns_servers[i].status) {
			dns_servers[i].addr.addr = addr;
			dns_servers[i].status = 1;
			return 0;
		}
	}
	return -1;
}
