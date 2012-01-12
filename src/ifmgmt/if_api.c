/*
 *  This file based on quagga if.c 99% remains same as original
 */
#include "common_types.h"
#include "ifmgmt.h"
#include "prefix.h"
#include "linklist.h"
#include "rtm.h"

enum GET_IF {   
	GET_IF_BY_NAME = 1, 
	GET_IF_BY_IPADDR,
	GET_IF_BY_IFINDEX
};
static struct interface * get_if (void *key, uint8_t key_type);

/* Interface existance check by index. */
struct interface * if_lookup_by_index (unsigned int index)
{
	return get_if (index, GET_IF_BY_IFINDEX);
}

const char * ifindex2ifname (unsigned int index)
{
	struct interface *ifp;

	return ((ifp = if_lookup_by_index(index)) != NULL) ?
		ifp->ifDescr : "unknown";
}

/* Interface existance check by interface name. */
struct interface * if_lookup_by_name (const char *name)
{
	return get_if (index, GET_IF_BY_NAME);
}


unsigned int ifname2ifindex (const char *name)
{
	struct interface *ifp;

	return ((ifp = if_lookup_by_name(name)) != NULL) ? ifp->ifIndex
		: -1;
}

/* Lookup interface by IPv4 address. */
struct interface * if_lookup_exact_address (struct in_addr src)
{
	struct listnode *node;
	struct listnode *cnode;
	struct interface *ifp;
	struct prefix *p;
	struct connected *c;
        int idx = 0;

        while (idx < get_max_ports ()) {
		for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, c))
		{
			p = c->address;

			if (p && p->family == AF_INET)
			{
				if (IPV4_ADDR_SAME (&p->u.prefix4, &src))
					return ifp;
			}	      
		}
                idx++;
	}
	return NULL;
}

/* Lookup interface by IPv4 address. */
struct interface * if_lookup_address (struct in_addr src)
{
	struct listnode *node;
	struct prefix addr;
	int bestlen = 0;
	struct listnode *cnode;
	struct interface *ifp;
	struct connected *c;
	struct interface *match = NULL;
        int idx = 0;

	addr.family = AF_INET;
	addr.u.prefix4 = src;
	addr.prefixlen = IPV4_MAX_BITLEN;

        while (idx < get_max_ports ()) {
		for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, c))
		{
			if (c->address && (c->address->family == AF_INET) &&
					prefix_match(CONNECTED_PREFIX(c), &addr) &&
					(c->address->prefixlen > bestlen))
			{
				bestlen = c->address->prefixlen;
				match = ifp;
			}
		}
                idx++;
        }
	return match;
}

/* Get interface by name if given name interface doesn't exist create
   one. */
struct interface * if_get_by_name (const char *name)
{
	struct interface *ifp;

	return ((ifp = if_lookup_by_name(name)) != NULL) ? ifp : NULL;
}

/* Is interface running? */
int if_is_running (struct interface *ifp)
{
	return ifp->ifOperStatus;
}

int if_is_up (struct interface *ifp)
{
	return ifp->ifAdminStatus;
}

int if_is_operative (struct interface *ifp)
{
	return (ifp->ifAdminStatus && ifp->ifOperStatus);
	//return (ifp->ifAdminStatus && ifp->ifOperStatus || !CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION));
}

/* Is this loopback interface ? */
int if_is_loopback (struct interface *ifp)
{
	//return (ifp->flags & (IFF_LOOPBACK|IFF_NOXMIT|IFF_VIRTUAL));
	return 0;
}

/* Does this interface support broadcast ? */
int if_is_broadcast (struct interface *ifp)
{
	//return ifp->flags & IFF_BROADCAST;
	return 1;
}

/* Does this interface support broadcast ? */
int if_is_pointopoint (struct interface *ifp)
{
	//return ifp->flags & IFF_POINTOPOINT;
	return 1;
}

/* Does this interface support multicast ? */
int if_is_multicast (struct interface *ifp)
{
	//return ifp->flags & IFF_MULTICAST;
	return 1;
}

static struct interface * get_if (void *key, uint8_t key_type)
{
	int idx = 0;

	while (idx < get_max_ports ()) {
		switch (key_type) {
			case GET_IF_BY_NAME:
				if (!strcmp ((char *)key, port_cdb[idx].ifDescr))
					return &port_cdb[idx];
				break;
			case GET_IF_BY_IFINDEX:
				if ((unsigned int)key == port_cdb[idx].ifIndex)
					return &port_cdb[idx];
				break;
		}
		idx++;
	}
	return NULL;
}