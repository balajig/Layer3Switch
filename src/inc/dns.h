/** @file  dns.h
    @brief Domain Name service

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

#ifndef DNS_H
#define DNS_H

#include "network.h"

/** @brief Type of address to resolve */
typedef enum dns_type_enum
{
  /** @brief To resolve host address */
  DNS_TYPE_A  = 1,
  /** @brief To resolve mail exchange */
  DNS_TYPE_MX = 15
} dns_t;

/** @brief Resolves the IP address of the specified host name.
 *  @param timeout expressed in ms.
 *  @param record_type either DNS_TYPE_A or DNS_TYPE_MX (mail) */
extern retcode dns_resolve(char *host_name, dns_t record_type, ip_t *remote_ip, u16 timeout);

#endif
