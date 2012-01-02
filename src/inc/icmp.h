/** @file icmp.h
    @brief ICMP layer (only ping request/answer managed). 
    ICMP layer provides commands to test IP connexion to a remote host.
    This implementation supports only ping requests / replies.

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

#ifndef ICMP_H
#define ICMP_H

#include "defines.h"
#include "ip.h"


/** @brief Ping remote host
 *  @param remote_ip Remote host ip address
 *  @returns -1 if no answer, otherwise answer time in ms. */
extern s16 icmp_ping(ip_t remote_ip, u16 timeout);

/** @brief Process ICMP frame from lower layer.
 *  For stack internal usage - not to be called by the applicative layer. */
extern void icmp_process(ip_t remote, u16 len);

#endif
