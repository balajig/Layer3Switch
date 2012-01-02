/** @file dhcp.h
 *  @brief Dynamic Host Configuration Protocol   

    This module will automatically acquire a valid IP 
    configuration from a DHCP server (address, gateway, 
    subnet mask and dns addresses).

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

#ifndef DHCP_H
#define DHCP_H

/** This asynchronous function returns true if dhcp IP configuration has been served. */
extern bool dhcp_state(void);

/** Wait until the dhcp client has received a valid IP configuration from
 *  a DHCP server. This function is blocking and returns to the caller only either
 *  when the DHCP configuration is server, either when the specified timeout has
 *  expired (and in this case, it returns fail). */
extern retcode dhcp_wait_configured(u32 timeout);


/** For stack internal usage - not to be called by the applicative layer. */
extern void dhcp_init(void);

/** For stack internal usage - not to be called by the applicative layer. */
extern void dhcp_close(void);

#endif


