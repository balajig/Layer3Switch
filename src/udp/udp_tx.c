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

/*
 * Process the packet, received from the network.
 * Called by IP layer.
 * If there is no target socket found, just return the packet.
 */
buf_t * udp_input (ip_t *ip, buf_t *p, netif_t *inp, ip_hdr_t *iph)
{
	udp_hdr_t *h;
	udp_socket_t *s;
	unsigned short dest, src, len;

	++ip->udp_in_datagrams;

	if (p->tot_len < sizeof(udp_hdr_t)) {
		/* Bad UDP packet received. */
		/*debug_printf ("udp_input: too short packet (hlen=%d bytes)\n", p->tot_len);*/
drop:
		++ip->udp_in_errors;
		buf_free (p);
		return 0;
	}
	h = (udp_hdr_t*) p->payload;

	/* LY: p->tot_len may include a eth-tail, use udp-len instead. */
	len = (h->len_h << 8) | h->len_l;
	/*debug_printf ("udp_input: driver %s received %d bytes, useful %d bytes\n",
		inp->name, p->tot_len, len);*/
	if (len > p->tot_len)
		goto drop;

	if ((h->chksum_h | h->chksum_l) != 0 &&
	    buf_chksum (p, crc16_inet_header (iph->src,
	    iph->dest, IP_PROTO_UDP, len)) != 0) {
		/* Checksum failed for received UDP packet. */
		/*debug_printf ("udp_input: bad checksum\n");*/
		goto drop;
	}

	/* Find a destination socket. */
	dest = h->dest_h << 8 | h->dest_l;
	src = h->src_h << 8 | h->src_l;
	for (s = ip->udp_sockets; s; s = s->next) {
		/*debug_printf ("<local :%d remote %d.%d.%d.%d:%d> ",
			s->local_port, s->peer_ip[0], s->peer_ip[1],
			s->peer_ip[2], s->peer_ip[3], s->peer_port);*/
		/* Compare local port number. */
		if (s->local_port != dest)
			continue;

		/* Compare remote port number. When 0, the socket
		 * will accept packets from "any" remote port. */
		if (s->peer_port && s->peer_port != src)
			continue;

		/* Compare peer IP address (or broadcast). */
		if (memcmp (s->peer_ip, IP_ADDR(0), 4) != 0 &&
		     memcmp (s->peer_ip, iph->src, 4) != 0)
			continue;

		/* Put packet to socket. */
		buf_add_header (p, -UDP_HLEN);
		mutex_lock (&s->lock);
		if (udp_queue_is_full (s)) {
			mutex_unlock (&s->lock);
			/*debug_printf ("udp_input: socket overflow\n");*/
			goto drop;
		}
		udp_queue_put (s, p, iph->src, src);
		mutex_signal (&s->lock, p);
		mutex_unlock (&s->lock);
		/*debug_printf ("udp_input: signaling socket on port %d\n",
			s->local_port);*/
		return 0;
	}

	/* No match was found. */
	/*debug_printf ("udp_input: no socket found\n");*/
	/*debug_printf ("    source %d.%d.%d.%d port %d, destination port %d\n",
		iph->src[0], iph->src[1], iph->src[2], iph->src[3],
		src, dest);*/
	++ip->udp_no_ports;
	return p;
}

