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
#include "ip.h"

int ip_init (void);
int init_ip_interface_table (void);

struct ip_addr_entry ip_port[MAX_PORTS];

int init_ip_interface_table (void)
{
	int i = 0;

	memset (ip_port, 0, sizeof (ip_port));
	
	while (i < get_max_ports ()) 
	{
		ip_port[i].IfIndex = i + 1;
		i++;
	}

	return 0;
}


int ip_init (void)
{
	init_ip_interface_table ();
}
#if 0
/*
 * Forward an IP packet. It finds an appropriate route for the packet,
 * decrements the TTL value of the packet, adjusts the checksum and outputs
 * the packet on the appropriate interface.
 */
static void
ip_forward (ip_t *ip, buf_t *p, unsigned char *gateway, if_t *netif,
	unsigned char *if_ipaddr)
{
	ip_hdr_t *iphdr = (ip_hdr_t*) p->payload;

	/* Decrement TTL and send ICMP if ttl == 0. */
	if (iphdr->ttl <= 1) {
		/* Don't send ICMP messages in response to ICMP messages */
		if (iphdr->proto != IP_PROTO_ICMP) {
			icmp_time_exceeded (ip, p);
		}
		return;
	}
	iphdr->ttl--;

	/* Incremental update of the IP checksum. */
	if (iphdr->chksum_h != 0xff)
		iphdr->chksum_h += 1;
	else if (iphdr->chksum_l != 0xff) {
		iphdr->chksum_h = 0;
		iphdr->chksum_l += 1;
	} else {
		iphdr->chksum_h = 1;
		iphdr->chksum_l = 0;
	}

	/* Forwarding packet to netif. */
	if (! gateway)
		gateway = iphdr->dest;
	if_output (netif, p, gateway, if_ipaddr);
	++ip->forw_datagrams;
}

/*
 * This function is called by the network interface device driver
 * when an IP packet is received. The function does the basic checks
 * of the IP header such as packet size being at least larger than
 * the header size etc. If the packet was not destined for us, the packet
 * is forwarded (using ip_forward). The IP checksum is always checked.
 * Finally, the packet is sent to the upper layer protocol input function.
 */
void
ip_input (ip_t *ip, buf_t *p, if_t *inp)
{
	ip_hdr_t *iphdr = (ip_hdr_t*) p->payload;
	unsigned char hlen, broadcast;

	++ip->in_receives;

	/* Identify the IP header */
	if ((iphdr->version & 0xf0) != 0x40) {
		/* Bad version number. */
		/*debug_printf ("ip_input: bad version number %d\n",
			iphdr->version >> 4);*/
		buf_free (p);
		++ip->in_hdr_errors;
		return;
	}

	hlen = (iphdr->version & 0x0f) * 4;
	if (hlen > p->len) {
		/* Too short packet. */
		/*debug_printf ("ip_input: too short packet (hlen=%d bytes)\n",
			hlen);*/
		buf_free (p);
		++ip->in_hdr_errors;
		return;
	}

	/* Verify checksum */
	if (crc16_inet (0, p->payload, hlen) != CRC16_INET_GOOD) {
		/* Failing checksum. */
		/*debug_printf ("ip_input: bad checksum\n", hlen);*/
		buf_free (p);
		++ip->in_hdr_errors;
		return;
	}

	/* Trim buf. This should have been done at the netif layer,
	 * but we'll do it anyway just to be sure that its done. */
	buf_truncate (p, iphdr->len_h << 8 | iphdr->len_l);

	/* Is this packet for us? */
	broadcast = IS_BROADCAST (iphdr->dest);
	if (! broadcast) {
		if_t *netif;

		netif = route_lookup_self (ip, iphdr->dest, &broadcast);
		if (! netif) {
			/* Packet not for us, route or discard */
			if (ip->forwarding && ! broadcast) {
				unsigned char *if_ipaddr, *gateway;

				netif = route_lookup (ip, iphdr->dest,
					&gateway, &if_ipaddr);
				if (! gateway)
					gateway = iphdr->dest;

				/* Don't forward packets onto the same
				 * network interface on which they arrived. */
				if (netif && netif != inp)
					ip_forward (ip, p, gateway, netif,
						if_ipaddr);
				else
					buf_free (p);
			} else {
				++ip->in_addr_errors;
				/*debug_printf ("ip_input: no route to host %d.%d.%d.%d\n",
					iphdr->dest[0], iphdr->dest[1], iphdr->dest[2], iphdr->dest[3]);*/
				buf_free (p);
			}
			return;
		}
	}
	if (broadcast)
		++inp->in_mcast_pkts;

	if (iphdr->offset_l || (iphdr->offset_h & (IP_OFFMASK | IP_MF)) != 0) {
		/* Drop fragmented packets. */
		/*debug_printf ("ip_input: fragmentation not implemented\n");*/
		buf_free (p);
		++ip->in_hdr_errors;
		return;
	}

	if (hlen > IP_HLEN) {
		/* Drop packets with IP options. */
		/*debug_printf ("ip_input: options not implemented\n");*/
		buf_free (p);
		++ip->in_hdr_errors;
		return;
	}

	/* Send to upper layers */
	++ip->in_delivers;
	switch (iphdr->proto) {
	case IP_PROTO_UDP:
		buf_add_header (p, -IP_HLEN);
		p = udp_input (ip, p, inp, iphdr);
		if (p) {
			/* No match was found, send ICMP destination
			 * port unreachable unless destination address
			 * was broadcast/multicast. */
			if (! broadcast && ! IS_MULTICAST (iphdr->dest)) {
				buf_add_header (p, IP_HLEN);
				icmp_dest_unreach (ip, p, ICMP_DUR_PORT);
			} else
				buf_free (p);
		}
		break;

	case IP_PROTO_TCP:
		if (! tcp_input)
			goto proto_unreach;
		buf_add_header (p, -IP_HLEN);
		tcp_input (ip, p, inp, iphdr);
		break;

	case IP_PROTO_ICMP:
		++ip->icmp_in_msgs;
		switch (p->payload [IP_HLEN]) {
		case ICMP_ECHO:
			/* Ignore ICMP messages on broadcasts */
			++ip->icmp_in_echos;
			if (! broadcast && ! IS_MULTICAST (iphdr->dest))
				icmp_echo_request (ip, p, inp);
			else
				buf_free (p);
			break;
		default:
			/* ICMP type not supported. */
			/*debug_printf ("ip_input: unsupported ICMP op\n");*/
			++ip->icmp_in_errors;
			buf_free (p);
			break;
		}
		break;

	default:
proto_unreach:
		/*debug_printf ("ip_input: protocol not implemented\n");*/

		/* Send ICMP destination protocol unreachable
		 * unless is was a broadcast */
		if (! broadcast && ! IS_MULTICAST (iphdr->dest)) {
			icmp_dest_unreach (ip, p, ICMP_DUR_PROTO);
		} else
			buf_free (p);

		/* Unsupported transport protocol. */
		--ip->in_delivers;
		++ip->in_unknown_protos;
		++inp->in_unknown_protos;
		break;
	}
}

/*
 * Send an IP packet on a network interface. This function constructs
 * the IP header and calculates the IP header checksum.
 * dest		- destination IP address
 * src		- source IP adress. If NULL, the IP address of outgoing
 *		  interface is used instead
 * proto	- protocol field value
 * gateway	- IP address of gateway
 * netif	- outgoing interface. If NULL, the routing table is searched
 *		  for an interface, using destination address as a key
 * if_ipaddr	- IP address of outgoing interface (when netif is not NULL)
 */
bool_t
ip_output_if (ip_t *ip, buf_t *p, unsigned char *dest, unsigned char *src,
	small_uint_t proto, unsigned char *gateway, if_t *netif,
	unsigned char *if_ipaddr)
{
	ip_hdr_t *iphdr;
	unsigned short chksum;

	++ip->out_requests;
	if (! buf_add_header (p, IP_HLEN)) {
		/* Not enough room for IP header. */
		/*debug_printf ("ip_output_if: no space for header\n");*/
		++ip->out_discards;
		buf_free (p);
		return 0;
	}

	/*
	 * Build an IP header.
	 */
	iphdr = (ip_hdr_t*) p->payload;
	iphdr->ttl = ip->default_ttl;
	iphdr->proto = proto;
	iphdr->version = 0x40 + IP_HLEN / 4;
	iphdr->len_h = p->tot_len >> 8;
	iphdr->len_l = p->tot_len;
	iphdr->tos = ip->tos;
	iphdr->offset_h = 0;
	iphdr->offset_l = 0;
	++ip->id;
	iphdr->id_h = ip->id >> 8;
	iphdr->id_l = ip->id;

	memcpy (iphdr->dest, dest, 4);
	memcpy (iphdr->src, src ? src : if_ipaddr, 4);

	iphdr->chksum_h = 0;
	iphdr->chksum_l = 0;
	chksum = ~crc16_inet (0, p->payload, IP_HLEN);
#if HTONS(1) == 1
	iphdr->chksum_h = chksum >> 8;
	iphdr->chksum_l = chksum;
#else
	iphdr->chksum_h = chksum;
	iphdr->chksum_l = chksum >> 8;
#endif
	/*debug_printf ("ip: netif %S output %d bytes\n",
		netif->name, p->tot_len);*/
	/*buf_print_ip (p);*/

	if (! gateway)
		gateway = dest;
	return if_output (netif, p, dest, if_ipaddr);
}

/*
 * Send an IP packet.
 */
bool_t
ip_output (ip_t *ip, buf_t *p, unsigned char *dest, unsigned char *src,
	small_uint_t proto)
{
	if_t *netif;
	unsigned char *gateway, *if_ipaddr;

	/* Find the outgoing network interface. */
	netif = route_lookup (ip, dest, &gateway, &if_ipaddr);
	if (! netif) {
		/* No route to host. */
		/*debug_printf ("ip_output: no route to host %d.%d.%d.%d\n",
			dest[0], dest[1], dest[2], dest[3]);*/
		++ip->out_requests;
		++ip->out_no_routes;
		buf_free (p);
		return 0;
	}
	return ip_output_if (ip, p, dest, src, proto, gateway, netif,
		if_ipaddr);
}


#endif
