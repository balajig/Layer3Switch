
/* 
 * Interface functions.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 * 
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "common_types.h"
#include "ifmgmt.h"
#include "rtm.h"
#include <zebra.h>

#include "linklist.h"
#include "vector.h"
#include "sockunion.h"
#include "prefix.h"
#include "memory.h"
#include "table.h"

int if_zebra_new_hook (struct interface *ifp);
int if_zebra_delete_hook (struct interface *ifp);
int if_cmp_func (struct interface *ifp1, struct interface *ifp2);
void if_delete_retain (struct interface *ifp);
struct connected * connected_delete_by_prefix (struct interface *ifp, struct prefix *p);
struct connected * connected_lookup_address (struct interface *ifp, struct in_addr dst);
struct connected * connected_new (void);
struct connected * connected_add_by_prefix (struct interface *ifp, struct prefix *p, 
                         struct prefix *destination);
unsigned int if_nametoindex (const char *name);
char * if_indextoname (unsigned int ifindex, char *name);
void connected_add_ipv4 (struct interface *ifp, int flags, uint32_t *host_addr, 
		    u_char prefixlen, uint32_t *bcast_addr, 
		    const char *label);
int
connected_delete_ipv4 (struct interface *ifp, int flags, struct in_addr *addr,
		       u_char prefixlen, struct in_addr *broad);

/* For interface multicast configuration. */
#define IF_ZEBRA_MULTICAST_UNSPEC 0
#define IF_ZEBRA_MULTICAST_ON     1
#define IF_ZEBRA_MULTICAST_OFF    2
    
/* For interface shutdown configuration. */
#define IF_ZEBRA_SHUTDOWN_UNSPEC 0
#define IF_ZEBRA_SHUTDOWN_ON     1
#define IF_ZEBRA_SHUTDOWN_OFF    2


/* Called when new interface is added. */
int if_zebra_new_hook (struct interface *ifp)
{
  struct zebra_if *zebra_if;

  zebra_if = XCALLOC (MTYPE_TMP, sizeof (struct zebra_if));

  zebra_if->multicast = IF_ZEBRA_MULTICAST_UNSPEC;
  zebra_if->shutdown = IF_ZEBRA_SHUTDOWN_UNSPEC;

#ifdef RTADV
  {
    /* Set default router advertise values. */
    struct rtadvconf *rtadv;

    rtadv = &zebra_if->rtadv;

    rtadv->AdvSendAdvertisements = 0;
    rtadv->MaxRtrAdvInterval = RTADV_MAX_RTR_ADV_INTERVAL;
    rtadv->MinRtrAdvInterval = RTADV_MIN_RTR_ADV_INTERVAL;
    rtadv->AdvIntervalTimer = 0;
    rtadv->AdvManagedFlag = 0;
    rtadv->AdvOtherConfigFlag = 0;
    rtadv->AdvHomeAgentFlag = 0;
    rtadv->AdvLinkMTU = 0;
    rtadv->AdvReachableTime = 0;
    rtadv->AdvRetransTimer = 0;
    rtadv->AdvCurHopLimit = 0;
    rtadv->AdvDefaultLifetime = RTADV_ADV_DEFAULT_LIFETIME;
    rtadv->HomeAgentPreference = 0;
    rtadv->HomeAgentLifetime = RTADV_ADV_DEFAULT_LIFETIME;
    rtadv->AdvIntervalOption = 0;
    rtadv->DefaultPreference = RTADV_PREF_MEDIUM;

    rtadv->AdvPrefixList = list_new ();
  }    
#endif /* RTADV */

  /* Initialize installed address chains tree. */
  zebra_if->ipv4_subnets = route_table_init ();

  ifp->info = zebra_if;
  return 0;
}

/* Called when interface is deleted. */
int
if_zebra_delete_hook (struct interface *ifp)
{
  struct zebra_if *zebra_if;
  
  if (ifp->info)
    {
      zebra_if = ifp->info;

      /* Free installed address chains tree. */
      if (zebra_if->ipv4_subnets)
	route_table_finish (zebra_if->ipv4_subnets);

      XFREE (zebra_if);
    }

  return 0;
}



/* Compare interface names, returning an integer greater than, equal to, or
 * less than 0, (following the strcmp convention), according to the
 * relationship between ifp1 and ifp2.  Interface names consist of an
 * alphabetic prefix and a numeric suffix.  The primary sort key is
 * lexicographic by name, and then numeric by number.  No number sorts
 * before all numbers.  Examples: de0 < de1, de100 < fxp0 < xl0, devpty <
 * devpty0, de0 < del0
 */         
int
if_cmp_func (struct interface *ifp1, struct interface *ifp2)
{
  unsigned int l1, l2;
  long int x1, x2;
  char *p1, *p2;
  int res;

  p1 = ifp1->ifDescr;
  p2 = ifp2->ifDescr;

  while (*p1 && *p2) {
    /* look up to any number */
    l1 = strcspn(p1, "0123456789");
    l2 = strcspn(p2, "0123456789");

    /* name lengths are different -> compare names */
    if (l1 != l2)
      return (strcmp(p1, p2));

    /* Note that this relies on all numbers being less than all letters, so
     * that de0 < del0.
     */
    res = strncmp(p1, p2, l1);

    /* names are different -> compare them */
    if (res)
      return res;

    /* with identical name part, go to numeric part */
    p1 += l1;
    p2 += l1;

    if (!*p1) 
      return -1;
    if (!*p2) 
      return 1;

    x1 = strtol(p1, &p1, 10);
    x2 = strtol(p2, &p2, 10);

    /* let's compare numbers now */
    if (x1 < x2)
      return -1;
    if (x1 > x2)
      return 1;

    /* numbers were equal, lets do it again..
    (it happens with name like "eth123.456:789") */
  }
  if (*p1)
    return 1;
  if (*p2)
    return -1;
  return 0;
}

/* Free connected structure. */
void            
connected_free (struct connected *connected)
{                       
  if (connected->address)
    prefix_free (connected->address);
        
  if (connected->destination)
    prefix_free (connected->destination);
                        
  if (connected->label)
    XFREE (connected->label);
        
  XFREE (connected);
}

/* Tie an interface address to its derived subnet list of addresses. */
int
if_subnet_add (struct interface *ifp, struct connected *ifc)
{ 
  struct route_node *rn;
  struct zebra_if *zebra_if;
  struct prefix cp;
  struct list *addr_list;

  assert (ifp && ifp->info && ifc);
  zebra_if = ifp->info;
        
  /* Get address derived subnet node and associated address list, while marking
 *      address secondary attribute appropriately. */ 
  cp = *ifc->address;
  apply_mask (&cp);
  rn = route_node_get (zebra_if->ipv4_subnets, &cp);
                                          
  if ((addr_list = rn->info))
    SET_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY);
  else
    {
      UNSET_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY);
      rn->info = addr_list = list_new ();
      route_lock_node (rn);
    }

  /* Tie address at the tail of address list. */
  listnode_add (addr_list, ifc);

  /* Return list element count. */
  return (addr_list->count);
}


/* Untie an interface address from its derived subnet list of addresses. */
int
if_subnet_delete (struct interface *ifp, struct connected *ifc)
{
  struct route_node *rn;
  struct zebra_if *zebra_if;
  struct list *addr_list;

  assert (ifp && ifp->info && ifc);
  zebra_if = ifp->info;

  /* Get address derived subnet node. */
  rn = route_node_lookup (zebra_if->ipv4_subnets, ifc->address);
  if (! (rn && rn->info))
    return -1;
  route_unlock_node (rn);

  /* Untie address from subnet's address list. */
  addr_list = rn->info;
  listnode_delete (addr_list, ifc);
  route_unlock_node (rn);

  /* Return list element count, if not empty. */
  if (addr_list->count)
    {
      /* If deleted address is primary, mark subsequent one as such and distribute. */
      if (! CHECK_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY))
        {
          ifc = listgetdata (listhead (addr_list));
          //zebra_interface_address_delete_update (ifp, ifc);
          UNSET_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY);
          //zebra_interface_address_add_update (ifp, ifc);
        }

      return addr_list->count;
    }

  /* Otherwise, free list and route node. */
  list_free (addr_list);
  rn->info = NULL;
  route_unlock_node (rn);

  return 0;
}
struct interface * if_connect_init (struct interface *ifp)
{
  ifp->connected = list_new ();
  ifp->connected->del = (void (*) (void *)) connected_free;

  return ifp;
}
void
if_delete_retain (struct interface *ifp)
{
  /* Free connected address list */
  list_delete (ifp->connected);
}

/* Print if_addr structure. */
static void __attribute__ ((unused))
connected_log (struct connected *connected, char *str)
{
  struct prefix *p;
  struct interface *ifp;
  char logbuf[BUFSIZ];
  char buf[BUFSIZ];
  
  ifp = connected->ifp;
  p = connected->address;

  snprintf (logbuf, BUFSIZ, "%s interface %s %s %s/%d ", 
	    str, ifp->ifDescr, prefix_family_str (p),
	    inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
	    p->prefixlen);

  p = connected->destination;
  if (p)
    {
      strncat (logbuf, inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
	       BUFSIZ - strlen(logbuf));
    }
  //zlog (NULL, LOG_INFO, "%s", logbuf);
}

/* If two connected address has same prefix return 1. */
static int
connected_same_prefix (struct prefix *p1, struct prefix *p2)
{
  if (p1->family == p2->family)
    {
      if (p1->family == AF_INET &&
	  IPV4_ADDR_SAME (&p1->u.prefix4, &p2->u.prefix4))
	return 1;
#ifdef HAVE_IPV6
      if (p1->family == AF_INET6 &&
	  IPV6_ADDR_SAME (&p1->u.prefix6, &p2->u.prefix6))
	return 1;
#endif /* HAVE_IPV6 */
    }
  return 0;
}

struct connected *
connected_delete_by_prefix (struct interface *ifp, struct prefix *p)
{
  struct listnode *node;
  struct listnode *next;
  struct connected *ifc;

  /* In case of same prefix come, replace it with new one. */
  for (node = listhead (ifp->connected); node; node = next)
    {
      ifc = listgetdata (node);
      next = node->next;

      if (connected_same_prefix (ifc->address, p))
	{
	  listnode_delete (ifp->connected, ifc);
	  return ifc;
	}
    }
  return NULL;
}

/* Find the IPv4 address on our side that will be used when packets
   are sent to dst. */
struct connected *
connected_lookup_address (struct interface *ifp, struct in_addr dst)
{
  struct prefix addr;
  struct listnode *cnode;
  struct connected *c;
  struct connected *match;

  addr.family = AF_INET;
  addr.u.prefix4 = dst;
  addr.prefixlen = IPV4_MAX_BITLEN;

  match = NULL;

  for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, c))
    {
      if (c->address && (c->address->family == AF_INET) &&
	  prefix_match(CONNECTED_PREFIX(c), &addr) &&
	  (!match || (c->address->prefixlen > match->address->prefixlen)))
	match = c;
    }
  return match;
}
/* Allocate connected structure. */
struct connected *
connected_new (void)
{       
  return XCALLOC (MTYPE_CONNECTED, sizeof (struct connected));
}

struct connected *
connected_add_by_prefix (struct interface *ifp, struct prefix *p, 
                         struct prefix *destination)
{
  struct connected *ifc;

  /* Allocate new connected address. */
  ifc = connected_new ();
  ifc->ifp = ifp;

  /* Fetch interface address */
  ifc->address = prefix_new();
  memcpy (ifc->address, p, sizeof(struct prefix));

  /* Fetch dest address */
  if (destination)
    {
      ifc->destination = prefix_new();
      memcpy (ifc->destination, destination, sizeof(struct prefix));
    }

  /* Add connected address to the interface. */
  listnode_add (ifp->connected, ifc);
  return ifc;
}

#ifndef HAVE_IF_NAMETOINDEX
unsigned int
if_nametoindex (const char *name)
{
  struct interface *ifp;

  return ((ifp = if_lookup_by_name(name)) != NULL)
  	 ? ifp->ifIndex : 0;
}
#endif

#ifndef HAVE_IF_INDEXTONAME
char *
if_indextoname (unsigned int ifindex, char *name)
{
  struct interface *ifp;

  if (!(ifp = if_lookup_by_index(ifindex)))
    return NULL;
  strncpy (name, ifp->ifDescr, IFNAMSIZ);
  return ifp->ifDescr;
}
#endif

int connected_route_add (struct interface *ifp,  uint32_t *addr, uint32_t *mask, int flags UNUSED_PARAM) 
{
		int masklen = u32ip_masklen (*mask);
		uint32_t  bcastaddr = ipv4_broadcast_addr(*addr, masklen);
		connected_add_ipv4 (ifp, ZEBRA_IFA_PEER, addr, masklen, &bcastaddr, NULL);
		return 0;
}

int connected_route_delete (struct interface *ifp,  uint32_t *addr, uint32_t *mask, int flags UNUSED_PARAM) 
{
		int masklen = u32ip_masklen (*mask);
		struct in_addr  bcastaddr, addr_s;
		bcastaddr.s_addr = ipv4_broadcast_addr(*addr, masklen);
		addr_s.s_addr = *addr;
		if(connected_delete_ipv4 (ifp, ZEBRA_IFA_PEER, &addr_s, masklen, &bcastaddr))
		  return 1;
		return 0;
}

#if 0 /* this route_table of struct connected's is unused
       * however, it would be good to use a route_table rather than
       * a list..
       */
/* Interface looking up by interface's address. */
/* Interface's IPv4 address reverse lookup table. */
struct route_table *ifaddr_ipv4_table;
/* struct route_table *ifaddr_ipv6_table; */

static void
ifaddr_ipv4_add (struct in_addr *ifaddr, struct interface *ifp)
{
  struct route_node *rn;
  struct prefix_ipv4 p;

  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_PREFIXLEN;
  p.prefix = *ifaddr;

  rn = route_node_get (ifaddr_ipv4_table, (struct prefix *) &p);
  if (rn)
    {
      route_unlock_node (rn);
      zlog_info ("ifaddr_ipv4_add(): address %s is already added",
		 inet_ntoa (*ifaddr));
      return;
    }
  rn->info = ifp;
}

static void
ifaddr_ipv4_delete (struct in_addr *ifaddr, struct interface *ifp)
{
  struct route_node *rn;
  struct prefix_ipv4 p;

  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_PREFIXLEN;
  p.prefix = *ifaddr;

  rn = route_node_lookup (ifaddr_ipv4_table, (struct prefix *) &p);
  if (! rn)
    {
      zlog_info ("ifaddr_ipv4_delete(): can't find address %s",
		 inet_ntoa (*ifaddr));
      return;
    }
  rn->info = NULL;
  route_unlock_node (rn);
  route_unlock_node (rn);
}

/* Lookup interface by interface's IP address or interface index. */
static struct interface *
ifaddr_ipv4_lookup (struct in_addr *addr, unsigned int ifindex)
{
  struct prefix_ipv4 p;
  struct route_node *rn;
  struct interface *ifp;

  if (addr)
    {
      p.family = AF_INET;
      p.prefixlen = IPV4_MAX_PREFIXLEN;
      p.prefix = *addr;

      rn = route_node_lookup (ifaddr_ipv4_table, (struct prefix *) &p);
      if (! rn)
	return NULL;
      
      ifp = rn->info;
      route_unlock_node (rn);
      return ifp;
    }
  else
    return if_lookup_by_index(ifindex);
}
#endif /* ifaddr_ipv4_table */
