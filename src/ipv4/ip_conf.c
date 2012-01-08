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
#include "ip_addr.h"
#include "ip.h"
        
extern struct ip_addr_entry ip_port[];

int set_ip_address (uint32_t ifindex, uint32_t ipaddress, uint32_t ipmask)
{
	ip_addr_t ipaddr, netmask;
	ip_port[ifindex - 1].Addr = ipaddress;
	ip_port[ifindex - 1].AddrMask = ipmask;
	ip_port[ifindex - 1].Status = 1;

	ipaddr.addr = ipaddress;
	netmask.addr = ipmask;
	if_set_addr (IF_INFO(ifindex - 1), &ipaddr, &netmask, NULL);
	return 0;
}
