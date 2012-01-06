#ifndef __ROUTE_H_
#define	__ROUTE_H_ 1
#include "common_types.h"
#include "ifmgmt.h"

typedef struct _route_t {
	struct _route_t *next;
	if_t	*netif;
	unsigned char	ipaddr [4];
	unsigned char	broadcast [4];
	unsigned char	netaddr [4];
	unsigned char	gateway [4];
	unsigned char	gwifaddr [4];
	unsigned char	masklen;
} route_t;

if_t *route_lookup (uint32_t ip);
if_t *route_lookup_internal (unsigned char *ipaddr, unsigned char **gateway, unsigned char **if_ipaddr);
if_t *route_lookup_self (unsigned char *ipaddr, unsigned char *broadcast);
unsigned char *route_lookup_ipaddr (unsigned char *ipaddr, if_t *);

int route_add_if (unsigned char *ipaddr, unsigned char masklen, if_t *netif);
int route_add_gateway (unsigned char *ipaddr, unsigned char masklen, unsigned char *gateway);

void route_setup (struct _route_t *r, unsigned char *ipaddr, unsigned char masklen, unsigned char *gateway);

#endif /* !__ROUTE_H_ */
