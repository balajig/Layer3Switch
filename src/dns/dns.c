/** @file dns.c 

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

#include "sdpos.h"

#ifndef NULL
# define NULL ((void *) 0)
#endif

#if (DNS_DEBUG_TRACE == 1)
# define dns_trace(...) {eprintf("[dns] "); eprintf(__VA_ARGS__); eprintf("\r\n");}
#else
# define dns_trace(...) 
#endif

#if(DNS_DEBUG_ANOMALIES == 1)
# define dns_anomaly(...) {eprintf("%v[31m[dns] "); eprintf(__VA_ARGS__); eprintf("%v[0m\r\n");}
#else
# define dns_anomaly(...)
#endif


typedef struct dns_header_struct
{
	u16 transaction_id;
	u16 flags;
	u16 questions;
	u16 answers;
	u16 authoritative_records;
	u16 additional_records;
} dns_header_t;

typedef struct dns_answer_struct
{
	u16	name;
	u16	type;
	u16	class;
	u32	ttl;
	u16	len;
} dns_answer_t;

static udp_socket_t dns_socket;
static u16 transaction_id = 0x1543;

static retcode dns_parse_answer(ip_t *host_ip, dns_answer_t *answer);
static void    dns_puts(char *string);
static void    dns_gets(char *string);
static void    dns_putw(u16 value);
static u16     dns_getw(void);


static void dns_puts(char *s)
{
  u16 i, offset = 0, len;
  char c;

	do
	{
	  c = 'a';
		for(len = 0; ((c != 0) && (c != '.') && (c != '/')); len++)
			c = s[offset + len];
	
		if(len > 255)
		{
		  dns_anomaly("String to long.");
		  len = 255;
		}
		
		/* Segment length */
		if(len > 0)
		{
		  dns_trace("puts: segment len = %2d.", len - 1)
		  gputc(dns_socket, len - 1);
		  for(i = 0; i < len - 1; i++)
		    gputc(dns_socket, s[offset + i]);
		}
		offset += len;// + 1;
		dns_trace("puts: offset = %2d.", offset);
	} while((c != 0) && (c != '/'));
	
	dns_trace("puts: end.");
	/* Terminal zero */
	gputc(dns_socket, 0);
}


static void dns_gets(char *string)
{
	u8 i, c, n;

	n = ggetc(dns_socket, 0);
	while(n > 0)
	{
		for(i = 0; i < n; i++)
		{
			c = ggetc(dns_socket, 0);
			if(string != NULL)
			{
			  *string = c;
			  string++;
			}
		}
		n = ggetc(dns_socket, 0);
	}
}

static void dns_putw(u16 value)
{
  gputc(dns_socket, value >> 8);
  gputc(dns_socket, value & 0xff);
}

static u16 dns_getw(void)
{
  u16 res;
  res = (ggetc(dns_socket, 1) << 8) & 0xff00;
  res |= (ggetc(dns_socket, 1) & 0xff);
  return res;
}

static retcode dns_parse_answer(ip_t *remote_ip, dns_answer_t *answer)
{
  dns_trace("parse answer...");
  answer->name  = dns_getw();
  answer->type  = dns_getw();
  answer->class = dns_getw();
  answer->ttl   = (dns_getw() << 16) | dns_getw();
  answer->len = dns_getw();

  if((answer->type  == 1) &&
     (answer->class == 1) &&
     (answer->len   == 4))
  {
    remote_ip->v[0]= ggetc(dns_socket, 1);
    remote_ip->v[1]= ggetc(dns_socket, 1);
    remote_ip->v[2]= ggetc(dns_socket, 1);
    remote_ip->v[3]= ggetc(dns_socket, 1);
    return ok;
  }
  while(answer->len-- != 0)
    ggetc(dns_socket, 1);
  return fail; 
}

retcode dns_resolve(char *host_name, dns_t record_type, ip_t *remote_ip, u16 timeout)
{
	dns_header_t header;
	dns_answer_t answer;
	s16 temp;

  require(dns_mutex);
  
  dns_trace("resolve host = %s...", host_name);

	/** DNS port is 53 */
	dns_socket = udp_open(8053, &primary_dns_server, 53);
	
	if(dns_socket == INVALID_UDP_SOCKET)
	{
	  dns_anomaly("Failed to open udp socket.");
	  release(dns_mutex);
	  return fail;
	}

	// Put DNS query here
	transaction_id++;
  // User chosen transaction ID
	dns_putw(transaction_id);
  // Standard query with recursion
	gputc(dns_socket, 0x01);
	gputc(dns_socket, 0x00);
  // 0x0001 questions
	gputc(dns_socket, 0x00);
	gputc(dns_socket, 0x01);
  // 0x0000 answers
	gputw(dns_socket, 0x0000);
  // 0x0000 name server resource records
	gputw(dns_socket, 0x0000);
  // 0x0000 additional records
	gputw(dns_socket, 0x0000);

	// Put hostname string to resolve
	dns_puts(host_name);

	gputc(dns_socket, 0x00);
	gputc(dns_socket, record_type);
  // Class: IN (Internet)
	gputc(dns_socket, 0x00);
	gputc(dns_socket, 0x01);
	gflush(dns_socket);
	
  start_alarm(dns_timeout, timeout);
  

  while(!is_raised(dns_timeout))
  {
  
  dns_trace("Waiting answer...");

  temp = ggetc(dns_socket, timeout);
  if(temp == -1)
  {
    dns_anomaly("Not received answer in specified timeout.");
    udp_close(dns_socket);
    release(dns_mutex);
    return fail;
  }

	header.transaction_id = (temp << 8) | (ggetc(dns_socket, 1) & 0xff);

	/* Check transaction id */
	if(header.transaction_id != transaction_id)
	{
	  dns_anomaly("Bad transaction id received.");
	  continue;
	}
	
	header.flags = dns_getw();

	header.questions = dns_getw();
	header.answers = dns_getw();
	header.authoritative_records = dns_getw();
	header.additional_records = dns_getw();
	
	dns_trace("Q=%2d, A=%2d, auth=%2d, add=%2d.", header.questions, header.answers, header.authoritative_records, header.additional_records);

	/* Ignore questions */
	while(header.questions--)
	{
		dns_gets(NULL);
		/* Question type and class */
		dns_getw();
		dns_getw();
	}
			
	/* Parse answers */
	while(header.answers--)
	{
	  if(dns_parse_answer(remote_ip, &answer) == ok)
	  {
	    udp_close(dns_socket);
	    release(dns_mutex);
	    return ok;
	  }
	}

	/* Parse authoritative records */
	while(header.authoritative_records--)
	{
    if(dns_parse_answer(remote_ip, &answer) == ok)
    {
      udp_close(dns_socket);
      release(dns_mutex);
      return ok;
    }
	}

	/* Parse additional records */
	while(header.additional_records--)
	{
    if(dns_parse_answer(remote_ip, &answer) == ok)
    {
      udp_close(dns_socket);
      release(dns_mutex);
      return ok;
    }
	}
  }

  dns_anomaly("Not received answer in specified timeout.");
  udp_close(dns_socket);
  release(dns_mutex);
  return fail;
}

#ifdef USE_SHELL
void dns_resolve_cmde(void)
{
  char *hostname;
  ip_t target;
  
  /* dns www.website.org */
  hostname = &(shell_buffer[4]);
  hostname[shell_buffer_size - 4] = 0;
  
  tprintf("Resolving '%s'... ", hostname);
  
  if(dns_resolve(hostname, DNS_TYPE_A, &target, 3000) == fail)
    tprintf("failed.\r\n", hostname);
  else
    tprintf("ok: ip = %i.\r\n", &target);
}
#endif


