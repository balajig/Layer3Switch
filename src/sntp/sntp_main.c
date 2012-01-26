/*
 *  Author:
 *  Sasikanth.V        <sasikanth@email.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

/*this is based on atinom and SNTP for Virtual PC*/
#include "common_types.h"
#include "sntp.h"
#include "opt.h"
#include "udp.h"
#include "ip_addr.h"

#define MAX_NTP_SERVER 1

static void         sntp_recv (void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t * addr, u16_t port);

double t1 = 0;

ip_addr_t ntp_server[MAX_NTP_SERVER];

static struct udp_pcb *sntp_pcb = NULL;

int sntp_init (void)
{
	sntp_pcb = udp_new ();

	if (sntp_pcb != NULL)
	{
		udp_bind (sntp_pcb, IP_ADDR_ANY, SNTP_UDP_PORT);
		udp_recv (sntp_pcb, sntp_recv, (void *) SNTP_UDP_PORT);
		return 0;
	}
	return -1;
}

int set_sntp_server (uint32_t addr)
{
	int i = 0;

	while (i < MAX_NTP_SERVER) {
		ntp_server[i].addr = addr;
		i++;
	}
	return -1;
}

static void  sntp_recv (void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t * addr, u16_t port)
{
	char           *reply_msg = (ntphdr_t *) p->payload;
	static struct timeval tv2, tv3, tv4, tvnew;
	struct timezone tz;
	double t2, t3, t4, toff, tnew;


	LWIP_UNUSED_ARG (arg);

	LWIP_DEBUGF (SNTP_DEBUG | LWIP_DBG_TRACE,
			("dhcp_recv(pbuf = %p) from SNTP server %" U16_F ".%" U16_F
			 ".%" U16_F ".%" U16_F " port %" U16_F "\n", (void *) p,
			 ip4_addr1_16 (addr), ip4_addr2_16 (addr), ip4_addr3_16 (addr),
			 ip4_addr4_16 (addr), port));
	LWIP_DEBUGF (SNTP_DEBUG | LWIP_DBG_TRACE,
			("pbuf->len = %" U16_F "\n", p->len));
	LWIP_DEBUGF (SNTP_DEBUG | LWIP_DBG_TRACE,
			("pbuf->tot_len = %" U16_F "\n", p->tot_len));

	gettimeofday (&tv4, &tz);

	t4 = (double) tv4.tv_sec + (double) tv4.tv_usec / 1000000;

	/* Calculate the local time offset */
	/* Get the remote Receive Timestamp */
	tv2.tv_sec = fromnetnum (&reply_msg[32]) - EPOCH_DIFF;
	tv2.tv_usec = frac2usec (fromnetnum (&reply_msg[36]));

	t2 = (double) tv2.tv_sec + (double) tv2.tv_usec / 1000000;
	/* Get the remote Transmit Timestamp */

	tv3.tv_sec = fromnetnum (&reply_msg[40]) - EPOCH_DIFF;
	tv3.tv_usec = frac2usec (fromnetnum (&reply_msg[44]));

	t3 = (double) tv3.tv_sec + (double) tv3.tv_usec / 1000000;
	/* The time offset */
	toff = (t2 + t3 - t1 - t4) / 2;

	/* Calculate the new time */
	tnew = t4 + toff;

	tvnew.tv_usec = (long long) (tnew * 1000000) % 1000000;
	tvnew.tv_sec = ((long long) (tnew * 1000000) - tvnew.tv_usec) / 1000000;

	/* Set the time */
	settimeofday (&tvnew, &tz);

	return;

}

/* synctime: Synchronize the time.  See RFC 1361.
return: Time offset that is synchronized */
int synctime (void)
{
	struct pbuf    *p_out = NULL;
	char           *buf = NULL;
	struct timeval  tv1;;
	struct timezone tz;
	int             i = 0;

	while (i < MAX_NTP_SERVER) {

		p_out = pbuf_alloc (PBUF_TRANSPORT, sizeof (ntphdr_t), PBUF_RAM);

		if (p_out == NULL)
		{
			LWIP_DEBUGF (SNTP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
					("synctime(): could not allocate pbuf\n"));
			return ERR_MEM;
		}

		udp_connect (sntp_pcb, &ntp_server[i], SNTP_UDP_PORT);

		buf = p_out->payload;

		/*TODO: Mdofiy this packet construct*/
		/* Pad zeroes */
		for (i = 0; i < 61; i++)
			buf[i] = 0;

		/* 00 001 011 - leap, ntp ver, client.  See RFC 1361. */
		buf[0] = (0 << 6) | (1 << 3) | 3;

		/* Get the local sent time - Originate Timestamp */
		gettimeofday (&tv1, &tz);

		t1 = (double) tv1.tv_sec + (double) tv1.tv_usec / 1000000;

		memcpy (&buf[40], tonetnum ((unsigned long) tv1.tv_sec + EPOCH_DIFF), 4);
		memcpy (&buf[44], tonetnum (usec2frac (tv1.tv_usec)), 4);

		udp_send (sntp_pcb, p_out);

		i++;
	}

	return 0;
}

/* fromnetnum: Convert from a network number to a C number.
return: the number in unsigned long. */
unsigned long fromnetnum (const char *oct)
{
	return ((unsigned char) oct[0] << 24 | (unsigned char) oct[1] << 16 | 
		(unsigned char) oct[2] << 8 | (unsigned char) oct[3]);
}

/* tonetnum: Convert from a C number to a network number.
return: the number in network octet.  */
const char *tonetnum (unsigned long num)
{
	static char oct[5] = "0000";
	oct[0] = (num >> 24) & 255;
	oct[1] = (num >> 16) & 255;
	oct[2] = (num >> 8) & 255;
	oct[3] = num & 255;
	return oct;
}

/* usec2frac: Convert from microsecond to fraction of a second.
return: Fraction of a second. */
unsigned long usec2frac (long usec)
{
	return (unsigned long) (((long long) usec << 32) / 1000000);
}

/* usec2frac: Convert from fraction of a second to microsecond
return: microsecond. */
long frac2usec (unsigned long frac)
{
	return (long) (((long long) frac * 1000000) >> 32);
}
