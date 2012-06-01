#include "common_types.h"
#include "ifmgmt.h"
#include "rib.h"
#include "zebra.h"
#include "table.h"

struct rib * rib_match_ipv4 (struct in_addr addr);
void show_ip_route (struct route_node *rn, struct rib *rib);


/*
 * Search the network interface and gateway address to forward the packet.
 * Return also the IP adress of the interface,
 */
if_t *route_lookup (uint32_t ip)
{
	struct rib * rib_new;
	struct in_addr addr;
	if_t   *outif = NULL;
	
	addr.s_addr = ip;

	rib_new = rib_match_ipv4 (addr);

	if (!rib_new)
		return NULL;

	outif = if_lookup_by_index (rib_new->nexthop->ifindex);

	if (outif->ip_addr.addr == ip) {
		outif = get_loopback_if ();
	}
	return outif;
}

int  cli_show_ip_route (void)
{
	struct route_table *table;
	struct route_node *rn;
	struct rib *rib;
	int first = 1;

	table = vrf_table (AFI_IP, SAFI_UNICAST, 0);
	if (! table)
	  return 0;

	/* Show all IPv4 routes. */
	for (rn = route_top (table); rn; rn = route_next (rn))
	  for (rib = rn->info; rib; rib = rib->next)
	    {
	     if (first)
	       {
		 cli_printf (SHOW_ROUTE_V4_HEADER);
		 first = 0;
	       }
	     show_ip_route (rn, rib);
	    }
	return 0;
}

int route_add_gateway (uint32_t network, uint32_t mask, uint32_t gateway_addr)
{
	uint8_t addr[4];
	char gateway[32];
	char  prefix_str[32];

	convert_uint32_str_ip_mask(prefix_str, ntohl(network), ntohl(mask));
	convert_uint32_str_ip (gateway, ntohl (gateway_addr));

	zebra_static_ipv4(1,prefix_str,NULL,gateway,NULL,NULL);
	return 0;
}
