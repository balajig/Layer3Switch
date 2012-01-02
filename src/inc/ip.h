#ifndef IP_H
#define IP_H

/** @file ip.h
 *  @brief IP network layer 

    Copyright 2007-2008 j. Arzi.

    This file is part of SDPOS.

    SDPOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SDPOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with SDPOS.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "network.h"



/*******************************************
 ****** LIST OF HIGHER LEVEL PROTOCOLS *****
 *******************************************/
#define IP_PROTOCOL_ICMP    1
#define IP_PROTOCOL_TCP     6
#define IP_PROTOCOL_UDP     17



/*******************************************
 ****** IP transmission API            *****
 *******************************************/

extern void ip_begin_packet(ip_t target, u8 protocol);
extern void ip_set_write_ptr(u16 offset);

extern iostream_t ip_ios;



/*******************************************
 ****** IP reception API               *****
 *******************************************/

/** @brief Extract IP header from current MAC rx packet.
    Header data is put into parameters: remote node informations, protocol,
    and packet length. */
extern retcode ip_get_header(ip_t *local_ip, ip_t *remote,
                             u8 *protocol, u16 *len);


/** @brief Current ip rx packet is discarded and buffer is freed-up **/
#define ip_discard_rx()         mac_discard_rx()

/** @brief Modify current packet rx pointer offset. */
extern void ip_set_read_ptr(u16 offset);

extern u16 ip_compute_checksum(u16 initial_checksum, char *buffer, u32 count);

/** Compute checksum of the current rx buffer, beginning
 *  from the current rx pointer, up to len bytes */
extern u16 ip_compute_rx_checksum(u16 len);

#endif



