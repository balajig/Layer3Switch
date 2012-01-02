#ifndef MAC_H
#define MAC_H

/** @file mac.h
 *  @brief MAC abstract layer 

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


typedef struct mdio_driver_struct
{
	u16     (*probe)(u16 port, u16 *phy_addr);

	u16     (*read)(u16 port, u16 regnum);

	retcode (*write)(u16 port, u16 regnum, u16 data);
	
} mdio_driver_t;

/** @brief Ethernet header structure (14 bytes) */
typedef struct ethernet_header_struct
{
   mac_t target;
   mac_t source;
   u16   type;
} 
_packed_attribute_ ethernet_header_t;

/** MAC packet types */
#define MAC_IP      	0x00
#define MAC_ARP     	0x06

/** @brief Enable the MAC hardware module. */
extern retcode mac_init(void);

/** Is MAC linked to network, or unplugged? */
extern bool mac_is_linked(void);

/** Try to link if not linked */
extern void mac_try_link(void);

/** Dispose MAC module. */
extern retcode mac_close(void);

/******************************
 *     WRITE FUNCTIONS        *
 * ****************************/


/** @returns true if mac mutex unlocked AND previous ethernet transmission completed */
extern int mac_is_tx_ready(void);

/** @brief Lock the mac mutex and wait previous transmission finished */
extern void mac_require_tx(void);

extern void mac_set_write_ptr(unsigned short address);

/** @brief Put mac-level header (eg ethernet, ppp, slip)
 *  @param remote destination MAC address
 *  @param type = MAC_ARP | MAC_IP
 *  @param size Data size to follow */
extern void mac_put_header(mac_t *remote, char type, u16 size);

extern iostream_t mac_ios;

/******************************
 *     READ FUNCTIONS         *
 * ****************************/

extern void mac_get_header(mac_t *remote, u8 *type);

extern void mac_set_read_ptr(unsigned short offset);

extern unsigned short mac_get_array(char *val, unsigned short len);

//extern char mac_get(void);

extern void mac_discard_rx(void);


#endif









