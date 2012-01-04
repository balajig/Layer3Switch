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

/* Max bit/byte length of IPv4 address. */
#define IPV4_MAX_BYTELEN    4
#define IPV4_MAX_BITLEN    32
#define IPV4_MAX_PREFIXLEN 32

static const u_char maskbit[] = {0x00, 0x80, 0xc0, 0xe0, 0xf0,
                                 0xf8, 0xfc, 0xfe, 0xff};


uint32_t ip_2_uint32 (uint8_t *ipaddress, int byte_order)
{
	uint32_t  byte[4];

	memset (byte, 0, sizeof(byte));

	sscanf ((char *)ipaddress, "%u.%u.%u.%u", &byte[0], &byte[1], &byte[2], &byte[3]);

	if (byte_order) /*Network*/
		return (byte[0] << 24) | (byte[1] << 16) | (byte[2] << 8) | (byte[3]);
	else /*Host*/
		return (byte[3] << 24) | (byte[2] << 16) | (byte[1] << 8) | (byte[0]);
}

void uint32_2_ipstring (uint32_t ipAddress, uint8_t *addr)
{
	int i = 0;
	for (i=0; i < 4; i++) {
		addr[i] = (ipAddress >> (i * 8) ) & 0xFF;
	}
}
/* Convert masklen into IP address's netmask. */
void masklen2ip (int masklen, uint32_t *netmask)
{
	u_char *pnt;
	int bit;
	int offset;

	memset (netmask, 0, sizeof (uint32_t));
	pnt = (unsigned char *) netmask;

	offset = masklen / 8;
	bit = masklen % 8;

	while (offset--)
		*pnt++ = 0xff;

	if (bit)
		*pnt = maskbit[bit];
}

/* Convert IP address's netmask into integer. We assume netmask is
   sequential one. Argument netmask should be network byte order. */
u_char ip_masklen (uint32_t netmask)
{
	u_char len;
	u_char *pnt;
	u_char *end;
	u_char val;

	len = 0;
	pnt = (u_char *) &netmask;
	end = pnt + 4;

	while ((pnt < end) && (*pnt == 0xff))
	{
		len+= 8;
		pnt++;
	} 

	if (pnt < end)
	{
		val = *pnt;
		while (val)
		{
			len++;
			val <<= 1;
		}
	}
	return len;
}

uint32_t ipv4_network_addr (uint32_t hostaddr, int masklen)
{
	uint32_t mask;

	masklen2ip (masklen, &mask);
	return hostaddr & mask;
}

	in_addr_t
ipv4_broadcast_addr (in_addr_t hostaddr, int masklen)
{
	uint32_t mask;

	masklen2ip (masklen, &mask);
	return (masklen != IPV4_MAX_PREFIXLEN-1) ?
		/* normal case */
		(hostaddr | ~mask) :
		/* special case for /31 */
		(hostaddr ^ ~mask);
}

/* Utility function to convert ipv4 netmask to prefixes 
   ex.) "1.1.0.0" "255.255.0.0" => "1.1.0.0/16"
   ex.) "1.0.0.0" NULL => "1.0.0.0/8"                   */
int netmask_str2prefix_str (const char *net_str, const char *mask_str, char *prefix_str)
{
	struct in_addr network;
	struct in_addr mask;
	u_char prefixlen;
	u_int32_t destination;
	int ret;

	ret = inet_aton (net_str, &network);
	if (! ret)
		return 0;

	if (mask_str)
	{
		ret = inet_aton (mask_str, &mask);
		if (! ret)
			return 0;

		prefixlen = ip_masklen (mask.s_addr);
	}
	else 
	{
		destination = ntohl (network.s_addr);

		if (network.s_addr == 0)
			prefixlen = 0;
		else if (IN_CLASSC (destination))
			prefixlen = 24;
		else if (IN_CLASSB (destination))
			prefixlen = 16;
		else if (IN_CLASSA (destination))
			prefixlen = 8;
		else
			return 0;
	}

	sprintf (prefix_str, "%s/%d", net_str, prefixlen);

	return 1;
}
