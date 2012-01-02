/** @file tcp.h
    @brief Transport Control Protocol
    Reference: RFC 793

    Copyright 2007-2009 j. Arzi.

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

#ifndef TCP_H
#define TCP_H

#include "sdpos.h"
#include "network.h"

typedef iostream_t tcp_socket_t;

#define INVALID_TCP_SOCKET INVALID_IOSTREAM

/** @brief Open a client port and connect to a remote server. 
 *  Returns when connection is established.
 *  If connection to the remote server is not possible, this function returns 
 *  INVALID_TCP_SOCKET. */
extern tcp_socket_t tcp_connect(ip_t remote_ip, u16 remote_port);

/** @brief Open a server port (ready for a remote client to connect)
 *  and returns immediatly. This function returns INVALID_TCP_SOCKET
 *  if no more socket is available. */
extern tcp_socket_t tcp_open(u16 port);

/** @brief Returns when a remote host has connected to a local server 
 *  port (opened with tcp_open above).
 *  This function is blocking, and returns to the caller the remote client 
 *  configuration (IP address and remote port). */
extern void tcp_listen(tcp_socket_t s, ip_t *remote_ip, u16 *remote_port);

/** This function is alternate and asynchronous way to know if 
 *  a remote client has connected to a local server port. 
 *  It returns immediatly (non blocking). */
extern bool tcp_is_connected(tcp_socket_t s);

/** This function close and if needed disconnect the specified 
 *  socket (either client or server port). */
extern void tcp_close(tcp_socket_t s);


/** For stack internal usage - not to be called by the applicative layer. */
extern void tcp_process(ip_t remote, ip_t local_ip, u16 len);

/** For stack internal usage - not to be called by the applicative layer. */
extern void tcp_init(void);

#endif
