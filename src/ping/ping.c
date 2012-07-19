/* vi: set sw=4 ts=4: */
/*
 * $Id: ping.c,v 1.1 2000/08/21 17:38:46 erich-roncarolo Exp $
 * Mini ping implementation for busybox
 *
 * Copyright (C) 1999 by Randolph Chung <tausq@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * This version of ping is adapted from the ping in netkit-base 0.10,
 * which is:
 *
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
 * 
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "common_types.h"
#include "lwip/ip.h"
#include "lwip/netdb.h"
#include "lwip/icmp.h"
#include "lwip/sockets.h"
#include "socks.h"

int ping_me (char *host);

struct icmp_ra_addr
{
  u_int32_t ira_addr;
  u_int32_t ira_preference;
};


struct icmp
{
  u_int8_t  icmp_type;	/* type of message, see below */
  u_int8_t  icmp_code;	/* type sub code */
  u_int16_t icmp_cksum;	/* ones complement checksum of struct */
  union
  {
    u_char ih_pptr;		/* ICMP_PARAMPROB */
    struct in_addr ih_gwaddr;	/* gateway address */
    struct ih_idseq		/* echo datagram */
    {
      u_int16_t icd_id;
      u_int16_t icd_seq;
    } ih_idseq;
    u_int32_t ih_void;

    /* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
    struct ih_pmtu
    {
      u_int16_t ipm_void;
      u_int16_t ipm_nextmtu;
    } ih_pmtu;

    struct ih_rtradv
    {
      u_int8_t irt_num_addrs;
      u_int8_t irt_wpa;
      u_int16_t irt_lifetime;
    } ih_rtradv;
  } icmp_hun;
#define	icmp_pptr	icmp_hun.ih_pptr
#define	icmp_gwaddr	icmp_hun.ih_gwaddr
#define	icmp_id		icmp_hun.ih_idseq.icd_id
#define	icmp_seq	icmp_hun.ih_idseq.icd_seq
#define	icmp_void	icmp_hun.ih_void
#define	icmp_pmvoid	icmp_hun.ih_pmtu.ipm_void
#define	icmp_nextmtu	icmp_hun.ih_pmtu.ipm_nextmtu
#define	icmp_num_addrs	icmp_hun.ih_rtradv.irt_num_addrs
#define	icmp_wpa	icmp_hun.ih_rtradv.irt_wpa
#define	icmp_lifetime	icmp_hun.ih_rtradv.irt_lifetime
  union
  {
    struct
    {
      u_int32_t its_otime;
      u_int32_t its_rtime;
      u_int32_t its_ttime;
    } id_ts;
    struct
    {
      struct ip_hdr idi_ip;
      /* options and then 64 bits of data */
    } id_ip;
    struct icmp_ra_addr id_radv;
    u_int32_t   id_mask;
    u_int8_t    id_data[1];
  } icmp_dun;
#define	icmp_otime	icmp_dun.id_ts.its_otime
#define	icmp_rtime	icmp_dun.id_ts.its_rtime
#define	icmp_ttime	icmp_dun.id_ts.its_ttime
#define	icmp_ip		icmp_dun.id_ip.idi_ip
#define	icmp_radv	icmp_dun.id_radv
#define	icmp_mask	icmp_dun.id_mask
#define	icmp_data	icmp_dun.id_data
};


static char *hostname = NULL;
static struct sockaddr_in pingaddr;
static int pingsock = -1;
static int datalen = DEFDATALEN;

static long ntransmitted = 0, nreceived = 0, nrepeats = 0, pingcount = 3;
static int myid = 0, options = 0;
static unsigned long tmin = -1, tmax = 0, tsum = 0;
static char rcvd_tbl[MAX_DUP_CHK / 8];
static int ping_exit = 0;

static void sendping(int);
static void pingstats(int);
static void unpack(char *, int, struct sockaddr_in *);

static void *pingtimer;

/**************************************************************************/
static int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return (ans);
}


static void pingstats(int ign)
{
	//signal(SIGINT, SIG_IGN);
	
	cli_printf("\n--- %s ping statistics ---\n", hostname);
	cli_printf("%ld packets transmitted, ", ntransmitted);
	cli_printf("%ld packets received, ", nreceived);
	if (nrepeats)
		cli_printf("%ld duplicates, ", nrepeats);
	if (ntransmitted)
		cli_printf("%ld%% packet loss\n",
			   (ntransmitted - nreceived) * 100 / ntransmitted);
	if (nreceived)
		cli_printf("round-trip min/avg/max = %lu.%lu/%lu.%lu/%lu.%lu ms\n",
			   tmin / 10, tmin % 10,
			   (tsum / (nreceived + nrepeats)) / 10,
			   (tsum / (nreceived + nrepeats)) % 10, tmax / 10, tmax % 10);
}

int set_pingexit (void)
{
	ping_exit = 1;
}

static void sendping(int ign)
{
	struct icmp *pkt;
	int i;
	char packet[datalen + 8];

	pkt = (struct icmp *) packet;

	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_code = 0;
	pkt->icmp_cksum = 0;
	pkt->icmp_seq = ntransmitted++;
	pkt->icmp_id = myid;
	CLR(pkt->icmp_seq % MAX_DUP_CHK);

	gettimeofday((struct timeval *) &packet[8], NULL);
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	i = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));

#if 0
	if (i < 0)
		printf ("ping: sendto: %s\n", strerror(errno));
	else if (i != sizeof(packet))
		printf ("ping wrote %d chars; %d expected\n", i,
			   (int)sizeof(packet));
#endif

	if (ntransmitted < pingcount) {	/* schedule next in 1s */
		start_timer (PINGINTERVAL * tm_get_ticks_per_second (), NULL, (void (*) (void *))sendping, 0);
	} else {					/* done, wait for the last ping to come back */
		pingtimer = start_timer (5 * tm_get_ticks_per_second (), NULL, 
					(void (*) (void *))set_pingexit, 0);
	}
}

static const char *icmp_type_name (int id)
{
	switch (id) {
	case ICMP_ECHOREPLY: 		return "Echo Reply";
	case ICMP_DEST_UNREACH: 	return "Destination Unreachable";
	case ICMP_SOURCE_QUENCH: 	return "Source Quench";
	case ICMP_REDIRECT: 		return "Redirect (change route)";
	case ICMP_ECHO: 			return "Echo Request";
	case ICMP_TIME_EXCEEDED: 	return "Time Exceeded";
	case ICMP_PARAMETERPROB: 	return "Parameter Problem";
	case ICMP_TIMESTAMP: 		return "Timestamp Request";
	case ICMP_TIMESTAMPREPLY: 	return "Timestamp Reply";
	case ICMP_INFO_REQUEST: 	return "Information Request";
	case ICMP_INFO_REPLY: 		return "Information Reply";
	case ICMP_ADDRESS: 			return "Address Mask Request";
	case ICMP_ADDRESSREPLY: 	return "Address Mask Reply";
	default: 					return "unknown ICMP type";
	}
}

static void unpack(char *buf, int sz, struct sockaddr_in *from)
{
	struct icmp *icmppkt;
	struct ip_hdr *iphdr;
	struct timeval tv, *tp;
	int hlen, dupflag;
	unsigned long triptime;

	gettimeofday(&tv, NULL);

	/* check IP header */
	iphdr = (struct ip_hdr *) buf;
    	hlen = IPH_HL (iphdr) * 4;
	/* discard if too short */
	if (sz < (datalen + ICMP_MINLEN))
		return;

	sz -= hlen;
	icmppkt = (struct icmp *) (buf + hlen);

	if (icmppkt->icmp_id != myid)
	    return;				/* not our ping */

	if (icmppkt->icmp_type == ICMP_ECHOREPLY) {
	    ++nreceived;
		tp = (struct timeval *) icmppkt->icmp_data;

		if ((tv.tv_usec -= tp->tv_usec) < 0) {
			--tv.tv_sec;
			tv.tv_usec += 1000000;
		}
		tv.tv_sec -= tp->tv_sec;

		triptime = tv.tv_sec * 10000 + (tv.tv_usec / 100);
		tsum += triptime;
		if (triptime < tmin)
			tmin = triptime;
		if (triptime > tmax)
			tmax = triptime;

		if (TST(icmppkt->icmp_seq % MAX_DUP_CHK)) {
			++nrepeats;
			--nreceived;
			dupflag = 1;
		} else {
			SET(icmppkt->icmp_seq % MAX_DUP_CHK);
			dupflag = 0;
		}

		if (options & O_QUIET)
			return;

		cli_printf("%d bytes from %s: icmp_seq=%u", sz,
			   inet_ntoa(*(struct in_addr *) &from->sin_addr.s_addr),
			   icmppkt->icmp_seq);
		cli_printf(" ttl=%d", iphdr->_ttl);
		cli_printf(" time=%lu.%lu ms", triptime / 10, triptime % 10);
		if (dupflag)
			cli_printf(" (DUP!)");
		printf("\n");
	} else 
		if (icmppkt->icmp_type != ICMP_ECHO)
			cli_printf("Warning: Got ICMP %d (%s)\n",
					icmppkt->icmp_type, icmp_type_name (icmppkt->icmp_type));
}

static void ping(const char *host)
{
	struct hostent *h;
	char buf[MAXHOSTNAMELEN];
	char packet[datalen + MAXIPLEN + MAXICMPLEN];
	int sockopt;

	memset(&pingaddr, 0, sizeof(struct sockaddr_in));

	if ((pingsock = socket (AF_INET, SOCK_RAW,IP_PROTO_ICMP)) < 0) {	/* 1 == ICMP */
		return;
	}

	/* enable broadcast pings */
	sockopt = 1;
	setsockopt(pingsock, SOL_SOCKET, SO_BROADCAST, (char *) &sockopt,
			sizeof(sockopt));

	/* set recv buf for broadcast pings */
	sockopt = 48 * 1024;
	setsockopt(pingsock, SOL_SOCKET, SO_RCVBUF, (char *) &sockopt,
			sizeof(sockopt));

	pingaddr.sin_family = AF_INET;
	if (!(h = gethostbyname(host))) {
		cli_printf("ping: unknown host %s\n", host);
		close (pingsock);
		return;
	}

	if (h->h_addrtype != AF_INET) {
		cli_printf("ping: unknown address type; only AF_INET is currently supported.\n");
		close (pingsock);
		return;
	}

	pingaddr.sin_family = AF_INET;	/* h->h_addrtype */
	memcpy(&pingaddr.sin_addr, h->h_addr, sizeof(pingaddr.sin_addr));
	strncpy(buf, h->h_name, sizeof(buf) - 1);
	hostname = buf;

	cli_printf("PING %s (%s): %d data bytes\n",
		   hostname,
		   inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr),
		   datalen);

	ping_exit = 0;
	pingtimer = NULL;
	ntransmitted = 0;
	nreceived = 0; 
	nrepeats = 0; 
	myid = 0;
	options = 0;
	tmin = -1;
	tmax = 0;
	tsum = 0;

	/* start the ping's going ... */
	sendping(0);

	/* listen for replies */
	while (1) {

		struct sockaddr_in from;
		socklen_t fromlen = (socklen_t) sizeof(from);
		int c;
		fd_set rfds;
		struct timeval tv;

		FD_ZERO (&rfds);
	
		FD_SET (pingsock, &rfds);

		tv.tv_sec = PINGINTERVAL;
		tv.tv_usec = 0;

		if (ping_exit) {
			break;
		}	

		if (select (pingsock + 1, &rfds, NULL, NULL, &tv) < 0) {
			continue;
		}
		if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
				(struct sockaddr *) &from, &fromlen)) < 0) {
			if (errno == EINTR)
				continue;
		}
		unpack(packet, c, &from);

		if (pingcount > 0 && nreceived >= pingcount) {
			break;
		}
	}

	pingstats (0);
	close (pingsock);
}
int ping_me (char *host)
{
	myid = get_tsk_pid() & 0xFFFF;
	ping(host);
	if (pingtimer) 
		del_timer (pingtimer);
	return 0;
}
