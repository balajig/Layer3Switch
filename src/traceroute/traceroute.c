/*
 * Copyright (c) 1988, 1989, 1991, 1994, 1995, 1996, 1997, 1998, 1999, 2000
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static const char copyright[] =
    "@(#) Copyright (c) 1988, 1989, 1991, 1994, 1995, 1996, 1997, 1998, 1999, 2000\n\
The Regents of the University of California.  All rights reserved.\n";
#if 0
static const char rcsid[] =
    "@(#)$Id: traceroute.c,v 1.4 2006/02/07 06:22:57 lindak Exp $ (LBL)";
#endif
static const char rcsid[] =
    "$FreeBSD: src/contrib/traceroute/traceroute.c,v 1.26 2004/04/17 18:44:23 pb Exp $";
#endif

/*
 * traceroute host  - trace the route ip packets follow going to "host".
 *
 * Attempt to trace the route an ip packet would follow to some
 * internet host.  We find out intermediate hops by launching probe
 * packets with a small ttl (time to live) then listening for an
 * icmp "time exceeded" reply from a gateway.  We start our probes
 * with a ttl of one and increase by one until we get an icmp "port
 * unreachable" (which means we got to "host") or hit a max (which
 * defaults to net.inet.ip.ttl hops & can be changed with the -m flag).
 * Three probes (change with -q flag) are sent at each ttl setting and
 * a line is printed showing the ttl, address of the gateway and
 * round trip time of each probe.  If the probe answers come from
 * different gateways, the address of each responding system will
 * be printed.  If there is no response within a 5 sec. timeout
 * interval (changed with the -w flag), a "*" is printed for that
 * probe.
 *
 * Probe packets are UDP format.  We don't want the destination
 * host to process them so the destination port is set to an
 * unlikely value (if some clod on the destination is using that
 * value, it can be changed with the -p flag).
 *
 * A sample use might be:
 *
 *     [yak 71]% traceroute nis.nsf.net.
 *     traceroute to nis.nsf.net (35.1.1.48), 64 hops max, 56 byte packet
 *      1  helios.ee.lbl.gov (128.3.112.1)  19 ms  19 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  39 ms  19 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  39 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  39 ms  40 ms  39 ms
 *      5  ccn-nerif22.Berkeley.EDU (128.32.168.22)  39 ms  39 ms  39 ms
 *      6  128.32.197.4 (128.32.197.4)  40 ms  59 ms  59 ms
 *      7  131.119.2.5 (131.119.2.5)  59 ms  59 ms  59 ms
 *      8  129.140.70.13 (129.140.70.13)  99 ms  99 ms  80 ms
 *      9  129.140.71.6 (129.140.71.6)  139 ms  239 ms  319 ms
 *     10  129.140.81.7 (129.140.81.7)  220 ms  199 ms  199 ms
 *     11  nic.merit.edu (35.1.1.48)  239 ms  239 ms  239 ms
 *
 * Note that lines 2 & 3 are the same.  This is due to a buggy
 * kernel on the 2nd hop system -- lbl-csam.arpa -- that forwards
 * packets with a zero ttl.
 *
 * A more interesting example is:
 *
 *     [yak 72]% traceroute allspice.lcs.mit.edu.
 *     traceroute to allspice.lcs.mit.edu (18.26.0.115), 64 hops max
 *      1  helios.ee.lbl.gov (128.3.112.1)  0 ms  0 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  19 ms  19 ms  19 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  19 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  19 ms  39 ms  39 ms
 *      5  ccn-nerif22.Berkeley.EDU (128.32.168.22)  20 ms  39 ms  39 ms
 *      6  128.32.197.4 (128.32.197.4)  59 ms  119 ms  39 ms
 *      7  131.119.2.5 (131.119.2.5)  59 ms  59 ms  39 ms
 *      8  129.140.70.13 (129.140.70.13)  80 ms  79 ms  99 ms
 *      9  129.140.71.6 (129.140.71.6)  139 ms  139 ms  159 ms
 *     10  129.140.81.7 (129.140.81.7)  199 ms  180 ms  300 ms
 *     11  129.140.72.17 (129.140.72.17)  300 ms  239 ms  239 ms
 *     12  * * *
 *     13  128.121.54.72 (128.121.54.72)  259 ms  499 ms  279 ms
 *     14  * * *
 *     15  * * *
 *     16  * * *
 *     17  * * *
 *     18  ALLSPICE.LCS.MIT.EDU (18.26.0.115)  339 ms  279 ms  279 ms
 *
 * (I start to see why I'm having so much trouble with mail to
 * MIT.)  Note that the gateways 12, 14, 15, 16 & 17 hops away
 * either don't send ICMP "time exceeded" messages or send them
 * with a ttl too small to reach us.  14 - 17 are running the
 * MIT C Gateway code that doesn't send "time exceeded"s.  God
 * only knows what's going on with 12.
 *
 * The silent gateway 12 in the above may be the result of a bug in
 * the 4.[23]BSD network code (and its derivatives):  4.x (x <= 3)
 * sends an unreachable message using whatever ttl remains in the
 * original datagram.  Since, for gateways, the remaining ttl is
 * zero, the icmp "time exceeded" is guaranteed to not make it back
 * to us.  The behavior of this bug is slightly more interesting
 * when it appears on the destination system:
 *
 *      1  helios.ee.lbl.gov (128.3.112.1)  0 ms  0 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  19 ms  39 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  19 ms  39 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  39 ms  40 ms  19 ms
 *      5  ccn-nerif35.Berkeley.EDU (128.32.168.35)  39 ms  39 ms  39 ms
 *      6  csgw.Berkeley.EDU (128.32.133.254)  39 ms  59 ms  39 ms
 *      7  * * *
 *      8  * * *
 *      9  * * *
 *     10  * * *
 *     11  * * *
 *     12  * * *
 *     13  rip.Berkeley.EDU (128.32.131.22)  59 ms !  39 ms !  39 ms !
 *
 * Notice that there are 12 "gateways" (13 is the final
 * destination) and exactly the last half of them are "missing".
 * What's really happening is that rip (a Sun-3 running Sun OS3.5)
 * is using the ttl from our arriving datagram as the ttl in its
 * icmp reply.  So, the reply will time out on the return path
 * (with no notice sent to anyone since icmp's aren't sent for
 * icmp's) until we probe with a ttl that's at least twice the path
 * length.  I.e., rip is really only 7 hops away.  A reply that
 * returns with a ttl of 1 is a clue this problem exists.
 * Traceroute prints a "!" after the time if the ttl is <= 1.
 * Since vendors ship a lot of obsolete (DEC's Ultrix, Sun 3.x) or
 * non-standard (HPUX) software, expect to see this problem
 * frequently and/or take care picking the target host of your
 * probes.
 *
 * Other possible annotations after the time are !H, !N, !P (got a host,
 * network or protocol unreachable, respectively), !S or !F (source
 * route failed or fragmentation needed -- neither of these should
 * ever occur and the associated gateway is busted if you see one).  If
 * almost all the probes result in some kind of unreachable, traceroute
 * will give up and exit.
 *
 * Notes
 * -----
 * This program must be run by root or be setuid.  (I suggest that
 * you *don't* make it setuid -- casual use could result in a lot
 * of unnecessary traffic on our poor, congested nets.)
 *
 * This program requires a kernel mod that does not appear in any
 * system available from Berkeley:  A raw ip socket using proto
 * IPPROTO_RAW must interpret the data sent as an ip datagram (as
 * opposed to data to be wrapped in a ip datagram).  See the README
 * file that came with the source to this program for a description
 * of the mods I made to /sys/netinet/raw_ip.c.  Your mileage may
 * vary.  But, again, ANY 4.x (x < 4) BSD KERNEL WILL HAVE TO BE
 * MODIFIED TO RUN THIS PROGRAM.
 *
 * The udp port usage may appear bizarre (well, ok, it is bizarre).
 * The problem is that an icmp message only contains 8 bytes of
 * data from the original datagram.  8 bytes is the size of a udp
 * header so, if we want to associate replies with the original
 * datagram, the necessary information must be encoded into the
 * udp header (the ip id could be used but there's no way to
 * interlock with the kernel's assignment of ip id's and, anyway,
 * it would have taken a lot more kernel hacking to allow this
 * code to set the ip id).  So, to allow two or more users to
 * use traceroute simultaneously, we use this task's pid as the
 * source port (the high bit is set to move the port number out
 * of the "likely" range).  To keep track of which probe is being
 * replied to (so times and/or hop counts don't get confused by a
 * reply that was delayed in transit), we increment the destination
 * port number before each probe.
 *
 * Don't use this as a coding example.  I was trying to find a
 * routing problem and this code sort-of popped out after 48 hours
 * without sleep.  I was amazed it ever compiled, much less ran.
 *
 * I stole the idea for this program from Steve Deering.  Since
 * the first release, I've learned that had I attended the right
 * IETF working group meetings, I also could have stolen it from Guy
 * Almes or Matt Mathis.  I don't know (or care) who came up with
 * the idea first.  I envy the originators' perspicacity and I'm
 * glad they didn't keep the idea a secret.
 *
 * Tim Seaver, Ken Adelman and C. Philip Wood provided bug fixes and/or
 * enhancements to the original distribution.
 *
 * I've hacked up a round-trip-route version of this that works by
 * sending a loose-source-routed udp datagram through the destination
 * back to yourself.  Unfortunately, SO many gateways botch source
 * routing, the thing is almost worthless.  Maybe one day...
 *
 *  -- Van Jacobson (van@ee.lbl.gov)
 *     Tue Dec 20 03:50:13 PST 1988
 */

#include "common_types.h"
#include "lwip/ip.h"
#include "lwip/tcp_impl.h"
#include "lwip/udp.h"
#include "lwip/netdb.h"
#include "lwip/icmp.h"
#include "lwip/sockets.h"
#include "ip_icmp.h"
#include "socks.h"

/* rfc1716 */


/* Maximum number of gateways (include room for one noop) */
#define NGATEWAYS ((int)((MAX_IPOPTLEN - IPOPT_MINOFF - 1) / sizeof(u_int32_t)))

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	64
#endif

#define Fprintf (void)fprintf
#define Printf (void)printf


/* What a GRE packet header looks like */
struct grehdr {
	u_int16_t   flags;
	u_int16_t   proto;
	u_int16_t   length;	/* PPTP version of these fields */
	u_int16_t   callId;
};
#ifndef IPPROTO_GRE
#define IPPROTO_GRE	47
#endif

/* For GRE, we prepare what looks like a PPTP packet */
#define GRE_PPTP_PROTO	0x880b

/* Host name and address list */
struct hostinfo {
	char *name;
	int n;
	u_int32_t *addrs;
};

/* Data section of the probe packet */
struct outdata {
	u_char seq;		/* sequence number of this packet */
	u_char ttl;		/* ttl packet left with */
	struct timeval tv;	/* time packet left */
};

#ifndef HAVE_ICMP_NEXTMTU
/* Path MTU Discovery (RFC1191) */
struct my_pmtu {
	u_short ipm_void;
	u_short ipm_nextmtu;
};
#endif

static u_char	packet[512];		/* last inbound (icmp) packet */

static struct ip_hdr *outip;		/* last output ip packet */
static u_char *outp;		/* last output inner protocol packet */

static int s;				/* receive (icmp) socket file descriptor */
static int sndsock;			/* send (udp) socket file descriptor */

static char *hostname;
static struct sockaddr whereto;	/* Who to try to reach */
static struct sockaddr wherefrom;	/* Who we are */
static int packlen;			/* total length of packet */
static int protlen;			/* length of protocol part of packet */
//static int minpacket;			/* min ip packet size */
//static int maxpacket = 32 * 1024;	/* max ip packet size */
static int pmtu;			/* Path MTU Discovery (RFC1191) */
static u_int pausemsecs;

static char *prog;
static char *source;
static  const char devnull[] = "/dev/null";

static int nprobes = 3;
static int max_ttl;
static int first_ttl = 1;
static u_short ident;

static int options;			/* socket options */
static int verbose;
static int waittime = 5;		/* time to wait for response (in seconds) */
static int nflag;			/* print addresses numerically */
static int disable_seq = 0;
static int optlen;			/* length of ip options */

extern int optind;
extern int opterr;
extern char *optarg;

/* Forwards */
double	deltaT(struct timeval *, struct timeval *);
void	freehostinfo(struct hostinfo *);
void	getaddr(u_int32_t *, char *);
struct	hostinfo *gethostinfo(char *);
u_short	in_cksum(u_short *, int);
char	*inetname(struct in_addr);
int	main(int, char **);
u_short p_cksum(struct ip_hdr *, u_short *, int);
int	packet_ok(u_char *, int, struct sockaddr_in *, int);
const char	*pr_type(u_char);
void	print(u_char *, int, struct sockaddr_in *);
#ifdef	IPSEC
int	setpolicy __P((int so, char *policy));
#endif
void	send_probe(int, int);
int	str2val(const char *, const char *, int, int);
void	tvsub(struct timeval *, struct timeval *);
void usage(void);
int	wait_for_reply(int, struct sockaddr_in *, const struct timeval *);
#ifndef HAVE_USLEEP
int	usleep(u_int);
#endif

void	udp_prep(struct outdata *);
int	udp_check(const u_char *, int);
void	tcp_prep(struct outdata *);
int	tcp_check(const u_char *, int);
void	gre_prep(struct outdata *);
int	gre_check(const u_char *, int);
void	gen_prep(struct outdata *);
int	gen_check(const u_char *, int);
void	icmp_prep(struct outdata *);
int	icmp_check(const u_char *, int);
void    setsin(register struct sockaddr_in *sin, register u_int32_t addr);
int     do_traceroute (char *trcdest);
if_t *  route_lookup (uint32_t ip);
/*FIXME: Fixme Don't use system call :(*/
int gethostname(char *name, size_t len);
struct hostent *gethostbyaddr(const void *addr,
		socklen_t len, int type);

/* Descriptor structure for each outgoing protocol we support */
struct outproto {
	const char  *name;		/* name of protocol */
	u_char	num;		/* IP protocol number */
	u_short	hdrlen;		/* max size of protocol header */
	u_short	port;		/* default base protocol-specific "port" */
	void	(*prepare)(struct outdata *);
				/* finish preparing an outgoing packet */
	int	(*check)(const u_char *, int);
				/* check an incoming packet */
};

/* List of supported protocols. The first one is the default. The last
   one is the handler for generic protocols not explicitly listed. */
struct	outproto protos[] = {
#if 0
	{
		"udp",
		IPPROTO_UDP,
		sizeof(struct udp_hdr),
		32768 + 666,
		udp_prep,
		udp_check
	},
	{
		"tcp",
		IPPROTO_TCP,
		sizeof(struct tcp_hdr),
		32768 + 666,
		tcp_prep,
		tcp_check
	},
	{
		"gre",
		IPPROTO_GRE,
		sizeof(struct grehdr),
		GRE_PPTP_PROTO,
		gre_prep,
		gre_check
	},
#endif
	{
		"icmp",
		IPPROTO_ICMP,
		sizeof(struct icmp),
		0,
		icmp_prep,
		icmp_check
	},
#if 0
	{
		NULL,
		0,
		2 * sizeof(u_short),
		0,
		gen_prep,
		gen_check
	},
#endif
};
struct	outproto *proto = &protos[0];

int do_traceroute (char *trcdest)
{
	register int code;
	register struct sockaddr_in *from = (struct sockaddr_in *)&wherefrom;
	register struct sockaddr_in *to = (struct sockaddr_in *)&whereto;
	int on = 1;
	register int ttl, probe, i;
	register int seq = 0;
	int tos = 0, settos = 0;
#if defined(IP_OPTIONS) && !defined(HAVE_RAW_OPTIONS)
	register int lsrr = 0;
#endif
	register u_short off = 0;
	int sump = 0;
	int sockerrno = 0;
	if_t * netif = NULL;


	/*
	 * Do the setuid-required stuff first, then lose priveleges ASAP.
	 * Do error checking for these two calls where they appeared in
	 * the original code.
	 */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
		sockerrno = errno;

	hostname = trcdest;

	max_ttl = 30;

	protlen = packlen - sizeof(*outip) - optlen;

	outip = (struct ip_hdr *)malloc((unsigned)packlen);
	if (outip == NULL) {
		Fprintf(stderr, "%s: malloc: %s\n", prog, strerror(errno));
		return 1;
	}
	memset((char *)outip, 0, packlen);

#if 0
	outip->ip_v = IPVERSION;
	if (settos)
		outip->ip_tos = tos;
#endif
#ifdef BYTESWAP_IP_HDR
	outip->_len = htons(packlen);
	outip->_offset = htons(off);
#else
	outip->_len = packlen;
	outip->_offset = off;
#endif
	outip->_proto = proto->num;
	outp = (u_char *)(outip + 1);

	getaddr (&to->sin_addr.s_addr , trcdest);
	outip->dest.addr = to->sin_addr.s_addr;

	IPH_VHL_SET(outip, IPVERSION, (outp - (u_char *)outip) >> 2);
	IPH_TOS_SET(outip, 0);
	ident = (get_tsk_pid() & 0xffff) | 0x8000;

	if (s < 0) {
		errno = sockerrno;
		Fprintf(stderr, "%s: icmp socket: %s\n", prog, strerror(errno));
		return (1);
	}
	if (options & SO_DONTROUTE)
		(void)setsockopt(s, SOL_SOCKET, SO_DONTROUTE, (char *)&on,
		    sizeof(on));

#if	defined(IPSEC) && defined(IPSEC_POLICY_IPSEC)
	if (setpolicy(s, "in bypass") < 0)
		errx(1, "%s", ipsec_strerror());

	if (setpolicy(s, "out bypass") < 0)
		errx(1, "%s", ipsec_strerror());
#endif	/* defined(IPSEC) && defined(IPSEC_POLICY_IPSEC) */

	if (sndsock < 0) {
		errno = sockerrno;
		Fprintf(stderr, "%s: raw socket: %s\n", prog, strerror(errno));
		return (1);
	}

#if defined(IP_OPTIONS) && !defined(HAVE_RAW_OPTIONS)
	if (lsrr > 0) {
		u_char optlist[MAX_IPOPTLEN];

		cp = "ip";
		if ((pe = getprotobyname(cp)) == NULL) {
			Fprintf(stderr, "%s: unknown protocol %s\n", prog, cp);
			return (1);
		}

		/* final hop */
		gwlist[lsrr] = to->sin_addr.s_addr;
		++lsrr;

		/* force 4 byte alignment */
		optlist[0] = IPOPT_NOP;
		/* loose source route option */
		optlist[1] = IPOPT_LSRR;
		i = lsrr * sizeof(gwlist[0]);
		optlist[2] = i + 3;
		/* Pointer to LSRR addresses */
		optlist[3] = IPOPT_MINOFF;
		memcpy(optlist + 4, gwlist, i);

		if ((setsockopt(sndsock, pe->p_proto, IP_OPTIONS,
		    (char *)optlist, i + sizeof(gwlist[0]))) < 0) {
			Fprintf(stderr, "%s: IP_OPTIONS: %s\n",
			    prog, strerror(errno));
			return (1);
		    }
	}
#endif

#ifndef SO_SNDBUF
	if (setsockopt(sndsock, SOL_SOCKET, SO_SNDBUF, (char *)&packlen,
	    sizeof(packlen)) < 0) {
		Fprintf(stderr, "%s: SO_SNDBUF: %s\n", prog, strerror(errno));
		return (1);
	}
#endif
#ifndef IP_HDRINCL
	if (setsockopt(sndsock, IPPROTO_IP, IP_HDRINCL, (char *)&on,
	    sizeof(on)) < 0) {
		Fprintf(stderr, "%s: IP_HDRINCL: %s\n", prog, strerror(errno));
		return (1);
	}
#else
#ifdef IP_TOS
	if (settos && setsockopt(sndsock, IPPROTO_IP, IP_TOS,
	    (char *)&tos, sizeof(tos)) < 0) {
		Fprintf(stderr, "%s: setsockopt tos %d: %s\n",
		    prog, tos, strerror(errno));
		return (1);
	}
#endif
#endif
	if (options & SO_DEBUG)
		(void)setsockopt(sndsock, SOL_SOCKET, SO_DEBUG, (char *)&on,
		    sizeof(on));
	if (options & SO_DONTROUTE)
		(void)setsockopt(sndsock, SOL_SOCKET, SO_DONTROUTE, (char *)&on,
		    sizeof(on));

	/* Determine our source address */

	netif = route_lookup (outip->dest.addr);

	if (!netif) {
		return -1;
	}

	setsin(from, netif->ip_addr.addr);

	outip->src.addr = from->sin_addr.s_addr;

	/* Check the source address (-s), if any, is valid */
	if (bind(sndsock, (struct sockaddr *)from, sizeof(*from)) < 0) {
		Fprintf(stderr, "%s: bind: %s\n",
		    prog, strerror(errno));
		return  (1);
	}

#if	defined(IPSEC) && defined(IPSEC_POLICY_IPSEC)
	if (setpolicy(sndsock, "in bypass") < 0)
		errx(1, "%s", ipsec_strerror());

	if (setpolicy(sndsock, "out bypass") < 0)
		errx(1, "%s", ipsec_strerror());
#endif	/* defined(IPSEC) && defined(IPSEC_POLICY_IPSEC) */

	Fprintf(stderr, "%s to %s (%s)",
	    prog, hostname, inet_ntoa(to->sin_addr));
	if (source)
		Fprintf(stderr, " from %s", source);
	Fprintf(stderr, ", %d hops max, %d byte packets\n", max_ttl, packlen);
	(void)fflush(stderr);

	for (ttl = first_ttl; ttl <= max_ttl; ++ttl) {
		u_int32_t lastaddr = 0;
		int gotlastaddr = 0;
		int got_there = 0;
		int unreachable = 0;
		int sentfirst = 0;
		int loss;

		Printf("%2d ", ttl);
		for (probe = 0, loss = 0; probe < nprobes; ++probe) {
			register int cc;
			struct timeval t1, t2;
			struct timezone tz;
			register struct ip_hdr *ip;
			struct outdata outdata;

			if (sentfirst && pausemsecs > 0)
				usleep(pausemsecs * 1000);
			/* Prepare outgoing data */
			if (disable_seq) {
				outdata.seq = seq;
			} else {
				outdata.seq = ++seq;
			}
			outdata.ttl = ttl;

			/* Avoid alignment problems by copying bytewise: */
			(void)gettimeofday(&t1, &tz);
			memcpy(&outdata.tv, &t1, sizeof(outdata.tv));

			/* Finalize and send packet */
			(*proto->prepare)(&outdata);
			send_probe(seq, ttl);
			++sentfirst;

			/* Wait for a reply */
			while ((cc = wait_for_reply(s, from, &t1)) != 0) {
				double T;
				int precis;

				(void)gettimeofday(&t2, &tz);
				i = packet_ok(packet, cc, from, seq);
				/* Skip short packet */
				if (i == 0)
					continue;
				if (!gotlastaddr ||
				    from->sin_addr.s_addr != lastaddr) {
					print(packet, cc, from);
					lastaddr = from->sin_addr.s_addr;
					++gotlastaddr;
				}
				T = deltaT(&t1, &t2);
#ifdef SANE_PRECISION
				if (T >= 1000.0)
					precis = 0;
				else if (T >= 100.0)
					precis = 1;
				else if (T >= 10.0)
					precis = 2;
				else
#endif
					precis = 3;
				Printf("  %.*f ms", precis, T);
				if (i == -2) {
#ifndef ARCHAIC
					ip = (struct ip_hdr *)packet;
					if (ip->_ttl <= 1)
						Printf(" !");
#endif
					++got_there;
					break;
				}
				/* time exceeded in transit */
				if (i == -1)
					break;
				code = i - 1;
				switch (code) {

				case ICMP_PORT_UNREACH:
#ifndef ARCHAIC
					ip = (struct ip_hdr *)packet;
					if (ip->_ttl <= 1)
						Printf(" !");
#endif
					++got_there;
					break;

				case ICMP_NET_UNREACH:
					++unreachable;
					Printf(" !N");
					break;

				case ICMP_HOST_UNREACH:
					++unreachable;
					Printf(" !H");
					break;

				case ICMP_PROT_UNREACH:
					++got_there;
					Printf(" !P");
					break;

				case ICMP_FRAG_NEEDED:
					++unreachable;
					Printf(" !F-%d", pmtu);
					break;

				case ICMP_SR_FAILED:
					++unreachable;
					Printf(" !S");
					break;

				case ICMP_PKT_FILTERED:
					++unreachable;
					Printf(" !X");
					break;

				case ICMP_PREC_VIOLATION:
					++unreachable;
					Printf(" !V");
					break;

				case ICMP_PREC_CUTOFF:
					++unreachable;
					Printf(" !C");
					break;

				default:
					++unreachable;
					Printf(" !<%d>", code);
					break;
				}
				break;
			}
			if (cc == 0) {
				loss++;
				Printf(" *");
			}
			(void)fflush(stdout);
		}
		if (sump) {
			Printf(" (%d%% loss)", (loss * 100) / nprobes);
		}
		putchar('\n');
		if (got_there ||
		    (unreachable > 0 && unreachable >= nprobes - 1))
			break;
	}
	close (s);
	return (0);
}

#define howmany(x,y) (((x)+((y)-1))/(y))
int
wait_for_reply(register int sock, register struct sockaddr_in *fromp,
    register const struct timeval *tp)
{
	fd_set fdsp;
	size_t nfds;
	struct timeval now, wait;
	struct timezone tz;
	register int cc = 0;
	socklen_t fromlen = sizeof(*fromp);

	nfds = howmany(sock + 1, NFDBITS);
	memset(&fdsp, 0, nfds * sizeof(fd_mask));
	FD_SET(sock, &fdsp);

	wait.tv_sec = tp->tv_sec + waittime;
	wait.tv_usec = tp->tv_usec;
	(void)gettimeofday(&now, &tz);
	tvsub(&wait, &now);
	if (wait.tv_sec < 0) {
		wait.tv_sec = 0;
		wait.tv_usec = 1;
	}

	cc = recvfrom(sock, (char *)packet, sizeof(packet), 0,
			(struct sockaddr *)fromp, &fromlen);

	return(cc);
}

void
send_probe(int seq, int ttl)
{
	register int cc;

	outip->_ttl = ttl;
	outip->_id = htons(ident + seq);

	/* XXX undocumented debugging hack */
	if (verbose > 1) {
		register const u_short *sp;
		register int nshorts, i;

		sp = (u_short *)outip;
		nshorts = (u_int)packlen / sizeof(u_short);
		i = 0;
		Printf("[ %d bytes", packlen);
		while (--nshorts >= 0) {
			if ((i++ % 8) == 0)
				Printf("\n\t");
			Printf(" %04x", ntohs(*sp++));
		}
		if (packlen & 1) {
			if ((i % 8) == 0)
				Printf("\n\t");
			Printf(" %02x", *(u_char *)sp);
		}
		Printf("]\n");
	}

#if !defined(IP_HDRINCL) && defined(IP_TTL)
	if (setsockopt(sndsock, IPPROTO_IP, IP_TTL,
	    (char *)&ttl, sizeof(ttl)) < 0) {
		Fprintf(stderr, "%s: setsockopt ttl %d: %s\n",
		    prog, ttl, strerror(errno));
		return (1);
	}
#endif

	cc = sendto(sndsock, (char *)outip,
	    packlen, 0, &whereto, sizeof(whereto));
	if (cc < 0 || cc != packlen)  {
		if (cc < 0)
			Fprintf(stderr, "%s: sendto: %s\n",
			    prog, strerror(errno));
		Printf("%s: wrote %s %d chars, ret=%d\n",
		    prog, hostname, packlen, cc);
		(void)fflush(stdout);
	}
}

#if	defined(IPSEC) && defined(IPSEC_POLICY_IPSEC)
int
setpolicy(so, policy)
	int so;
	char *policy;
{
	char *buf;

	buf = ipsec_set_policy(policy, strlen(policy));
	if (buf == NULL) {
		warnx("%s", ipsec_strerror());
		return -1;
	}
	(void)setsockopt(so, IPPROTO_IP, IP_IPSEC_POLICY,
		buf, ipsec_get_policylen(buf));

	free(buf);

	return 0;
}
#endif

double
deltaT(struct timeval *t1p, struct timeval *t2p)
{
	register double dt;

	dt = (double)(t2p->tv_sec - t1p->tv_sec) * 1000.0 +
	     (double)(t2p->tv_usec - t1p->tv_usec) / 1000.0;
	return (dt);
}

/*
 * Convert an ICMP "type" field to a printable string.
 */
const char *
pr_type(register u_char t)
{
	static const char *ttab[] = {
	"Echo Reply",	"ICMP 1",	"ICMP 2",	"Dest Unreachable",
	"Source Quench", "Redirect",	"ICMP 6",	"ICMP 7",
	"Echo",		"ICMP 9",	"ICMP 10",	"Time Exceeded",
	"Param Problem", "Timestamp",	"Timestamp Reply", "Info Request",
	"Info Reply"
	};

	if (t > 16)
		return("OUT-OF-RANGE");

	return(ttab[t]);
}

int
packet_ok(register u_char *buf, int cc, register struct sockaddr_in *from,
    register int seq)
{
	register struct icmp *icp;
	register u_char type, code;
	register int hlen;
#ifndef ARCHAIC
	register struct ip_hdr *ip;

	ip = (struct ip_hdr *) buf;
	hlen = IPH_HL(ip);
	if (cc < hlen + ICMP_MINLEN) {
		if (verbose)
			Printf("packet too short (%d bytes) from %s\n", cc,
				inet_ntoa(from->sin_addr));
		return (0);
	}
	cc -= hlen;
	icp = (struct icmp *)(buf + hlen);
#else
	icp = (struct icmp *)buf;
#endif
	type = icp->icmp_type;
	code = icp->icmp_code;
	/* Path MTU Discovery (RFC1191) */
	if (code != ICMP_FRAG_NEEDED)
		pmtu = 0;
	else {
#ifdef HAVE_ICMP_NEXTMTU
		pmtu = ntohs(icp->icmp_nextmtu);
#else
		pmtu = ntohs(((struct my_pmtu *)&icp->icmp_void)->ipm_nextmtu);
#endif
	}
	if (type == ICMP_ECHOREPLY
	    && proto->num == IPPROTO_ICMP
	    && (*proto->check)((u_char *)icp, (u_char)seq))
		return -2;
	if ((type == ICMP_TIME_EXCEEDED && code == ICMP_TIMXCEED_INTRANS) ||
	    type == ICMP_UNREACH) {
		struct ip_hdr *hip;
		u_char *inner;

		hip = &icp->icmp_ip;
		hlen = IPH_HL(hip);
		inner = (u_char *)((u_char *)hip + hlen);
		if (hlen + 12 <= cc
		    && hip->_proto == proto->num
		    && (*proto->check)(inner, (u_char)seq))
			return (type == ICMP_TIME_EXCEEDED ? -1 : code + 1);
	}
#ifndef ARCHAIC
	if (verbose) {
		register int i;
		u_int32_t *lp = (u_int32_t *)&icp->icmp_ip;

		Printf("\n%d bytes from %s to ", cc, inet_ntoa(from->sin_addr));
		Printf("%s: icmp type %d (%s) code %d\n",
		    inet_ntoa(ip->dest), type, pr_type(type), icp->icmp_code);
		for (i = 4; i < cc ; i += sizeof(*lp))
			Printf("%2d: x%8.8x\n", i, *lp++);
	}
#endif
	return(0);
}

void
icmp_prep(struct outdata *outdata)
{
	struct icmp *const icmpheader = (struct icmp *) outp;

	icmpheader->icmp_type = ICMP_ECHO;
	icmpheader->icmp_id = htons(ident);
	icmpheader->icmp_seq = htons(outdata->seq);
	icmpheader->icmp_cksum = 0;
	icmpheader->icmp_cksum = in_cksum((u_short *)icmpheader, protlen);
	if (icmpheader->icmp_cksum == 0)
		icmpheader->icmp_cksum = 0xffff;
}

int
icmp_check(const u_char *data, int seq)
{
	struct icmp *const icmpheader = (struct icmp *) data;

	return (icmpheader->icmp_id == htons(ident)
	    && icmpheader->icmp_seq == htons(seq));
}

#if 0
void
udp_prep(struct outdata *outdata)
{
	struct udp_hdr *const outudp = (struct udp_hdr *) outp;

	outudp->uh_sport = htons(ident);
	outudp->uh_dport = htons(port + outdata->seq);
	outudp->uh_ulen = htons((u_short)protlen);
	outudp->uh_sum = 0;
	if (doipcksum) {
	    u_short sum = p_cksum(outip, (u_short*)outudp, protlen);
	    outudp->uh_sum = (sum) ? sum : 0xffff;
	}

	return;
}

int
udp_check(const u_char *data, int seq)
{
	struct udp_hdr *const udp = (struct udp_hdr *) data;

	return (ntohs(udp->uh_sport) == ident
	    && ntohs(udp->uh_dport) == port + seq);
}

void
tcp_prep(struct outdata *outdata)
{
	struct tcp_hdr *const tcp = (struct tcp_hdr *) outp;

	tcp->th_sport = htons(ident);
	tcp->th_dport = htons(port + outdata->seq);
	tcp->th_seq = (tcp->th_sport << 16) | tcp->th_dport;
	tcp->th_ack = 0;
	tcp->th_off = 5;
	tcp->th_flags = TH_SYN;
	tcp->th_sum = 0;

	if (doipcksum) {
	    u_short sum = p_cksum(outip, (u_short*)tcp, protlen);
	    tcp->th_sum = (sum) ? sum : 0xffff;
	}
}

int
tcp_check(const u_char *data, int seq)
{
	struct tcp_hdr *const tcp = (struct tcp_hdr *) data;

	return (ntohs(tcp->th_sport) == ident
	    && ntohs(tcp->th_dport) == port + seq);
}

void
gre_prep(struct outdata *outdata)
{
	struct grehdr *const gre = (struct grehdr *) outp;

	gre->flags = htons(0x2001);
	gre->proto = htons(port);
	gre->length = 0;
	gre->callId = htons(ident + outdata->seq);
}

int
gre_check(const u_char *data, int seq)
{
	struct grehdr *const gre = (struct grehdr *) data;

	return(ntohs(gre->proto) == port
	    && ntohs(gre->callId) == ident + seq);
}

void
gen_prep(struct outdata *outdata)
{
	u_int16_t *const ptr = (u_int16_t *) outp;

	ptr[0] = htons(ident);
	ptr[1] = htons(port + outdata->seq);
}

int
gen_check(const u_char *data, int seq)
{
	u_int16_t *const ptr = (u_int16_t *) data;

	return(ntohs(ptr[0]) == ident
	    && ntohs(ptr[1]) == port + seq);
}
#endif
void
print(register u_char *buf, register int cc, register struct sockaddr_in *from)
{
	register struct ip_hdr *ip;
	register int hlen;

	ip = (struct ip_hdr *) buf;
	hlen = IPH_HL(ip);
	cc -= hlen;

	if (nflag)
		Printf(" %s", inet_ntoa(from->sin_addr));
	else
		Printf(" %s (%s)", inetname(from->sin_addr),
		    inet_ntoa(from->sin_addr));

	if (verbose)
		Printf(" %d bytes to %s", cc, inet_ntoa (ip->dest));
}
#if 0
/*
 * Checksum routine for UDP and TCP headers.
 */
u_short 
p_cksum(struct ip_hdr *ip, u_short *data, int len)
{
	static struct ipovly ipo;
	u_short sumh, sumd;
	u_long sumt;

	ipo.ih_pr = ip->ip_p;
	ipo.ih_len = htons(len);
	ipo.ih_src = ip->ip_src;
	ipo.ih_dst = ip->ip_dst;

	sumh = in_cksum((u_short*)&ipo, sizeof(ipo)); /* pseudo ip hdr cksum */
	sumd = in_cksum((u_short*)data, len);	      /* payload data cksum */
	sumt = (sumh << 16) | (sumd);

	return ~in_cksum((u_short*)&sumt, sizeof(sumt));
}
#endif
/*
 * Checksum routine for Internet Protocol family headers (C Version)
 */
u_short
in_cksum(register u_short *addr, register int len)
{
	register int nleft = len;
	register u_short *w = addr;
	register u_short answer;
	register int sum = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1)
		sum += *(u_char *)w;

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}

/*
 * Subtract 2 timeval structs:  out = out - in.
 * Out is assumed to be within about LONG_MAX seconds of in.
 */
void
tvsub(register struct timeval *out, register struct timeval *in)
{

	if ((out->tv_usec -= in->tv_usec) < 0)   {
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

/*
 * Construct an Internet address representation.
 * If the nflag has been supplied, give
 * numeric value, otherwise try for symbolic name.
 */
char *
inetname(struct in_addr in)
{
	register char *cp;
	register struct hostent *hp;
	static int first = 1;
	static char domain[MAXHOSTNAMELEN + 1], line[MAXHOSTNAMELEN + 1];

	if (first && !nflag) {
		first = 0;
		if (gethostname(domain, sizeof(domain) - 1) < 0)
			domain[0] = '\0';
		else {
			cp = strchr(domain, '.');
			if (cp == NULL) {
				hp = gethostbyname(domain);
				if (hp != NULL)
					cp = strchr(hp->h_name, '.');
			}
			if (cp == NULL)
				domain[0] = '\0';
			else {
				++cp;
				(void)strncpy(domain, cp, sizeof(domain) - 1);
				domain[sizeof(domain) - 1] = '\0';
			}
		}
	}
	if (!nflag && in.s_addr != INADDR_ANY) {
		hp = gethostbyaddr((char *)&in, sizeof(in), AF_INET);
		if (hp != NULL) {
			if ((cp = strchr(hp->h_name, '.')) != NULL &&
			    strcmp(cp + 1, domain) == 0)
				*cp = '\0';
			(void)strncpy(line, hp->h_name, sizeof(line) - 1);
			line[sizeof(line) - 1] = '\0';
			return (line);
		}
	}
	return (inet_ntoa(in));
}

struct hostinfo *
gethostinfo(register char *hname)
{
	register int n;
	register struct hostent *hp;
	register struct hostinfo *hi;
	register char **p;
	register u_int32_t addr, *ap;

	if (strlen(hname) > 64) {
		Fprintf(stderr, "%s: hostname \"%.32s...\" is too long\n",
		    prog, hname);
		return NULL;
	}
	hi = calloc(1, sizeof(*hi));
	if (hi == NULL) {
		Fprintf(stderr, "%s: calloc %s\n", prog, strerror(errno));
		return NULL;
	}
	addr = inet_addr(hname);
	if ((int32_t)addr != -1) {
		hi->name = strdup(hname);
		hi->n = 1;
		hi->addrs = calloc(1, sizeof(hi->addrs[0]));
		if (hi->addrs == NULL) {
			Fprintf(stderr, "%s: calloc %s\n",
			    prog, strerror(errno));
			return NULL;
		}
		hi->addrs[0] = addr;
		return (hi);
	}

	hp = gethostbyname(hname);
	if (hp == NULL) {
		Fprintf(stderr, "%s: unknown host %s\n", prog, hname);
		return NULL;
	}
	if (hp->h_addrtype != AF_INET || hp->h_length != 4) {
		Fprintf(stderr, "%s: bad host %s\n", prog, hname);
		return NULL;
	}
	hi->name = strdup(hp->h_name);
	for (n = 0, p = hp->h_addr_list; *p != NULL; ++n, ++p)
		continue;
	hi->n = n;
	hi->addrs = calloc(n, sizeof(hi->addrs[0]));
	if (hi->addrs == NULL) {
		Fprintf(stderr, "%s: calloc %s\n", prog, strerror(errno));
		return NULL;
	}
	for (ap = hi->addrs, p = hp->h_addr_list; *p != NULL; ++ap, ++p)
		memcpy(ap, *p, sizeof(*ap));
	return (hi);
}

void
freehostinfo(register struct hostinfo *hi)
{
	if (hi->name != NULL) {
		free(hi->name);
		hi->name = NULL;
	}
	free((char *)hi->addrs);
	free((char *)hi);
}

void
getaddr(register u_int32_t *ap, register char *hname)
{
	register struct hostinfo *hi;

	hi = gethostinfo(hname);
	*ap = hi->addrs[0];
	freehostinfo(hi);
}

void
setsin(register struct sockaddr_in *sin, register u_int32_t addr)
{

	memset(sin, 0, sizeof(*sin));
#ifdef HAVE_SOCKADDR_SA_LEN
	sin->sin_len = sizeof(*sin);
#endif
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = addr;
}

/* String to value with optional min and max. Handles decimal and hex. */
int
str2val(register const char *str, register const char *what,
    register int mi, register int ma)
{
	register const char *cp;
	register int val;
	char *ep;

	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
		cp = str + 2;
		val = (int)strtol(cp, &ep, 16);
	} else
		val = (int)strtol(str, &ep, 10);
	if (*ep != '\0') {
		Fprintf(stderr, "%s: \"%s\" bad value for %s \n",
		    prog, str, what);
		return (1);
	}
	if (val < mi && mi >= 0) {
		if (mi == 0)
			Fprintf(stderr, "%s: %s must be >= %d\n",
			    prog, what, mi);
		else
			Fprintf(stderr, "%s: %s must be > %d\n",
			    prog, what, mi - 1);
		return (1);
	}
	if (val > ma && ma >= 0) {
		Fprintf(stderr, "%s: %s must be <= %d\n", prog, what, ma);
		return (1);
	}
	return (val);
}
