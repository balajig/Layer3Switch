/** @file udp.c 

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

#include "udp.h"
#include <string.h>


#if (UDP_DEBUG_TRACE == 1)
# define udp_trace(...) {eprintf("[udp] "); eprintf(__VA_ARGS__); eprintf("\r\n");}
#else
# define udp_trace(...) 
#endif

#if(UDP_DEBUG_ANOMALIES == 1)
# define udp_anomaly(...) {eprintf("%v[31m[udp] "); eprintf(__VA_ARGS__); eprintf("%v[0m\r\n");}
#else
# define udp_anomaly(...)
#endif


#define swap_pseudo_header(h)  (h.length = swaps(h.length))

typedef struct udp_socket_info_struct
{
  ip_t        remote;
  udp_port_t  remote_port;
  udp_port_t  local_port;
} udp_socket_info_t;

typedef struct udp_header_struct
{
  udp_port_t source_port;
  udp_port_t destination_port;
  u16        length;
  u16        checksum;
} __attribute__ ((__packed__)) udp_header_t;

/* Pseudo header as defined by rfc 793. */
typedef struct pseudo_header_struct
{
  ip_t source_address;
  ip_t dest_address;
  u8   zero;
  u8   protocol;
  u16  length;
} __attribute__ ((__packed__)) pseudo_header_t;




/*********************************************/
/****     LOCAL FUNCTION DECLARATION    ******/
/*********************************************/

static void     udp_putc(u16 port, char c);
static void     udp_flush(u16 port);
static s16      udp_getc(u16 port, u32 timeout);
static udp_socket_t find_matching_socket(udp_header_t *h, ip_t remote, ip_t local_ip);

/******************************************/
/****     LOCAL DATA  DECLARATION    ******/
/******************************************/

/** Index of the first udp iostream */
static iostream_t udp_iostreams;
static iodevice_t udp_iodevice = {.putc = udp_putc, .flush = udp_flush, .getc = udp_getc, .seek = NULL, .write = NULL, .read = NULL};



static char               udp_rx_fifo_buffers[MAX_UDP_SOCKETS*UDP_RX_FIFO_SIZE];
static udp_socket_info_t  udp_data[MAX_UDP_SOCKETS];
static udp_socket_t       current_udp_tx_socket;
static u16                udp_tx_buffer_size;
static pseudo_header_t    pseudo_header;
static udp_header_t       up_h;



static s16 udp_getc(u16 s, u32 timeout)
{
  if(timeout == 0)
    return fifo_getc(udp_rx_fifo + s);
  return fifo_getc_with_timeout(udp_rx_fifo + s, timeout);
}

static void udp_putc(u16 s, char c)
{
  if(current_udp_tx_socket != s) 
  {
    udp_socket_info_t *p;
    p = &udp_data[s];
    ip_begin_packet(p->remote, IP_PROTOCOL_UDP);
    current_udp_tx_socket = s;
    ip_set_write_ptr(sizeof(udp_header_t));
    udp_tx_buffer_size = 0;
  }
  gputc(ip_ios, c);
  udp_tx_buffer_size++;
}

static udp_header_t       h;

static void udp_flush(u16 s)
{
  udp_socket_info_t *p;
  
  if(s != current_udp_tx_socket)
  {
    udp_anomaly("nothing to flush.")
    return;
  }

  p = &udp_data[s];

  h.source_port       = swaps(p->local_port);
  h.destination_port  = swaps(p->remote_port);
  h.length            = udp_tx_buffer_size + sizeof(udp_header_t);
  h.checksum          = 0x0000;
  
  udp_trace("Flushing buffer (dl = %2d).", udp_tx_buffer_size);
  
  /* Put UDP header */
  ip_set_write_ptr(0);
  h.length = swaps(h.length);
  gwrite(ip_ios, &h, sizeof(udp_header_t));
  current_udp_tx_socket = INVALID_UDP_SOCKET;
  /* Flush IP packet */
  gflush(ip_ios);
}




void udp_init(void)
{
  udp_socket_t s;

  udp_trace("UDP layer initialization...");

  udp_iostreams = register_iodevice(&udp_iodevice, MAX_UDP_SOCKETS);

  udp_tx_buffer_size    = 0;
  current_udp_tx_socket = 0xff;
  for(s = 0; s < MAX_UDP_SOCKETS; s++)
  {
    udp_close(s + udp_iostreams);
    fifo_init(udp_rx_fifo + s, &(udp_rx_fifo_buffers[UDP_RX_FIFO_SIZE*s]), UDP_RX_FIFO_SIZE);
  }
}



udp_socket_t udp_open(udp_port_t local_port, ip_t *remote_node, udp_port_t remote_port)
{
  udp_socket_t s;
  udp_socket_info_t *p;
  
  udp_trace("open local port = %2d, remote port = %2d.", local_port, remote_port);

  if(remote_node)
  {
    udp_trace("remote node specified: %i.", remote_node);
  }
  else
  {
    udp_trace("remote node unspecified.");
  }

  for(s = 0; s < MAX_UDP_SOCKETS; s++)
  {
    p = &(udp_data[s]);
    if(p->local_port == INVALID_UDP_PORT)
    {
      p->local_port = local_port;

      /* If remote_node is supplied, remember it. */
      if(remote_node)
        p->remote.val = remote_node->val;
      /* else set broadcast address */
      else
        p->remote.val = 0xffffffff;
      
      p->remote_port   = remote_port;
      
      udp_trace("Socket %d is opened.", s);
      return s + udp_iostreams;
    }
  }
  udp_anomaly("No more socket available.");
  return INVALID_UDP_SOCKET;
}



void udp_close(udp_socket_t s)
{
  s -= udp_iostreams;
  udp_trace("Close socket %d.", s);
  udp_data[s].local_port = INVALID_UDP_PORT;
  udp_data[s].remote.val = 0xffffffff;
}

void udp_process(ip_t remote, ip_t local_ip, u16 len)
{
  udp_socket_t		s;
  u16 checksum_1, checksum_2;

  udp_trace("udp process...");

  /* Retrieve UDP header. */
  mac_get_array((char*)&up_h, sizeof(up_h));

  up_h.source_port       = swaps(up_h.source_port);
  up_h.destination_port  = swaps(up_h.destination_port);
  up_h.length            = swaps(up_h.length) - sizeof(udp_header_t);

  /* See if we need to validate the checksum field (0x0000 is disabled) */
  if(up_h.checksum)
  {
    /* Calculate IP pseudoheader checksum. */
    pseudo_header.source_address = remote;
    pseudo_header.dest_address   = local_ip;
    pseudo_header.zero				   = 0x0;
    pseudo_header.protocol			 = IP_PROTOCOL_UDP;
    pseudo_header.length				 = len;

    swap_pseudo_header(pseudo_header);

    checksum_1 = ~ip_compute_checksum(0xffff, (char*)&pseudo_header, sizeof(pseudo_header_t));

    ip_set_read_ptr(0);
    checksum_2 = ip_compute_rx_checksum(len);

    if(checksum_1 != checksum_2)
    {
      udp_anomaly("Bad checksum: %2x VS %2x.", checksum_1, checksum_2);
      mac_discard_rx();
      return;
    }
  }

  s = find_matching_socket(&up_h, remote, local_ip);
  if(s != INVALID_UDP_SOCKET)
  {
    u16 i;
    ip_set_read_ptr(sizeof(udp_header_t));
    for(i = 0; i < up_h.length; i++)
      fifo_putc(udp_rx_fifo + s, ggetc(mac_ios, 0));
  }
  else
  {
  	udp_anomaly("no matching socket (from %i).", &remote);
  }
  mac_discard_rx();
}


static udp_socket_t find_matching_socket(udp_header_t *h,
                                         ip_t remote,
                                         ip_t local_ip)
{
  udp_socket_t       s;
  udp_socket_t       partial_match;
  udp_socket_info_t *p;

  partial_match = INVALID_UDP_SOCKET;

  for (s = 0; s < MAX_UDP_SOCKETS; s++ )
  {
    p = &(udp_data[s]);

    if(h->destination_port == p->local_port)
    {
      /* Here, at least partial match (for open without target specification). */
      partial_match = s;
      
      /* Full match? */
      if((p->remote_port == h->source_port))
      {
        if ((p->remote.val == remote.val) ||
            (local_ip.val == 0xffffffff))
          return s;
      }
    }
  }
  /* No full match, but partial match on a socket:
   * must save remote node addresses (IP & MAC), and also remote port. */
  if(partial_match != INVALID_UDP_SOCKET)
  {
    p = &udp_data[partial_match];
    p->remote.val = remote.val;
    p->remote_port = h->source_port;
  }
  return partial_match;
}
