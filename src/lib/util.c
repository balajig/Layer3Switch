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

void convert_uint32_str_ip_mask (char *str, uint32_t ip, uint32_t mask)
{
	uint8_t  addr[4];

	uint32_2_ipstring (ntohl(ip), addr);

	sprintf (str, "%d.%d.%d.%d/%d", addr[0], addr[1], addr[2], addr[3], u32ip_masklen(ntohl(mask)));
}

void convert_uint32_str_ip (char *str, uint32_t ip)
{
	uint8_t  addr[4];

	uint32_2_ipstring (ntohl(ip), addr);

	sprintf (str, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
}

void uint32_2_ipstring (uint32_t ipAddress, uint8_t *addr)
{
	int i = 0;
	for (i=0; i < 4; i++) {
		addr[i] = (ipAddress >> (i * 8) ) & 0xFF;
	}
}
/* Convert masklen into IP address's netmask. */
void u32masklen2ip (int masklen, uint32_t *netmask)
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
u_char u32ip_masklen (uint32_t netmask)
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

uint32_t u32ipv4_network_addr (uint32_t hostaddr, int masklen)
{
	uint32_t mask;

	u32masklen2ip (masklen, &mask);
	return hostaddr & mask;
}

/* Convert "[x]x[:][x]x[:][x]x[:][x]x" hex string to binary, no more than COUNT bytes */
char*  hex2bin(char *dst, const char *str, int count)
{
    errno = EINVAL;
    while (*str && count) {
        uint8_t val;
        uint8_t c = *str++;
        if (isdigit(c))
            val = c - '0';
        else if ((c|0x20) >= 'a' && (c|0x20) <= 'f')
            val = (c|0x20) - ('a' - 10);
        else
            return NULL;
        val <<= 4;
        c = *str;
        if (isdigit(c))
            val |= c - '0';
        else if ((c|0x20) >= 'a' && (c|0x20) <= 'f')
            val |= (c|0x20) - ('a' - 10);
        else if (c == ':' || c == '\0')
            val >>= 4;
        else
            return NULL;

        *dst++ = val;
        if (c != '\0')
            str++;
        if (*str == ':')
            str++;
        count--;
    }
    errno = (*str ? ERANGE : 0);
    return dst;
}
