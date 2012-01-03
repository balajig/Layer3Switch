#ifndef UDP_H
#define UDP_H

/** @file UDP.h
 *  @brief User Datagram Protocol layer

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



#include "common_types.h"
#include "ip.h"

typedef iostream_t udp_socket_t;

#define INVALID_UDP_SOCKET      INVALID_IOSTREAM
#define INVALID_UDP_PORT        (0ul)


/** @brief UDP layer initialization.
    This function should be called once before udp layer usage. */
extern void udp_init(void);

/** @brief Allocate a new UDP socket with specified port parameters.
 *         Remote node parameter is optional and should be
 *         specified only in case the caller wants to send data immediatly.
 *         Otherwise, this field (ip address of remote node) will be completed
 *         by udp layer on first packet reception on this socket.
 *  @param[in] remote_node Remote node info such as MAC and IP
 *  address. If NULL, broadcast node address is set.
 *  @param[in] remote_port Remote port to which to talk to.
 *  If INVALID_UDP_SOCKET, local port is opened for Listen.
 *  @param[in] local_port  A valid port number.
 *  @returns On success: A UDP socket handle that can be used 
 *			 for subsequent UDP API calls.
 *			 The handle can be uses as an iostream (gputc, ggetc, gprintf, etc.).
 *	     On failure: INVALID_UDP_SOCKET */
extern udp_socket_t udp_open(udp_port_t local_port,
                             ip_t      *remote_node,
                             udp_port_t remote_port);

/** @brief Close previously opened UDP socket.
    Given socket is marked as available for future communications. */
extern void udp_close(udp_socket_t s);

/** @brief Handler called by IP stack */
extern void udp_process(ip_t remote, ip_t local_ip, u16 len);

#endif
