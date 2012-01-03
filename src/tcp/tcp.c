/** @file tcp.c
    @brief Transport Control Protocol
    Reference: RFC 793

    Copyright 2007-2008 j. Arzi. (julien.arzi@orange.fr)


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

#include "tcp.h"


/*******************************************
 ******      DEFINES                ********
 *******************************************/

#ifndef NULL
# define NULL ((void *) 0)
#endif

#if (TCP_DEBUG_TRACE == 1)
# define tcp_trace(...) {eprintf("[tcp] "); eprintf(__VA_ARGS__); eprintf("\r\n");}
#else
# define tcp_trace(...) 
#endif

#if(TCP_DEBUG_ANOMALIES == 1)
# define tcp_anomaly(...) {eprintf("%v[31m[tcp] "); eprintf(__VA_ARGS__); eprintf("%v[0m\r\n");}
#else
# define tcp_anomaly(...)
#endif

/** timeouts in ms */
#define TCP_START_TIMEOUT   	         500
#define TCP_TIME_WAIT_TIMEOUT           50
#define MAX_RETRY_COUNTS                 2
/** Time period for tx fifo transmission */
#define TCP_AUTO_TRANSMIT_TIMEOUT        2
/** Time to wait before acking a single paquet received */
#define TCP_DELAYED_ACK_TIMEOUT        100

/** TCP flags */
#define TCP_FLAG_FIN     0x01
#define TCP_FLAG_SYN     0x02
#define TCP_FLAG_RST     0x04
#define TCP_FLAG_PSH     0x08
#define TCP_FLAG_ACK     0x10
#define TCP_FLAG_URG     0x20

/** Local ports affected for client sockets. */
#define FIRST_LOCAL_PORT 3000

#define modulo_sub(A,B,M) (((A) >= (B)) ? ((A) - (B)) : ((M)+(A)-(B)))
#define min(A,B)          (((A) > (B)) ? (B) : (A))


/* 576 */
#define TCP_MAX_MTU_SIZE 1400
//576
//1576

#define TCP_TICK_PERIOD    8
//2

//#define CHECK_INT_SOCKET(S) if(S >= MAX_TCP_SOCKETS) {tcp_anomaly("invalid socket."); thread_exit();}

/*******************************************
 ******      TYPEDEFS               ********
 *******************************************/

typedef struct tcp_header_struct
{
  u16 source_port;
  u16 target_port;
  u32 sequence_number;
  u32 ack_number;
  u8  data_offset;
  u8  flags;
  u16 window;
  u16 checksum;
  u16 urgent_pointer;
} __attribute__ ((__packed__)) tcp_header_t;


typedef struct tcp_options_struct
{
# define OPTION_MAX_SEGMENT_SIZE 0x02
  u8  kind;
  u8  length;
  /* First word of options */
  u16 w1;
} __attribute__ ((__packed__)) tcp_options_t;

/* Pseudo header as defined by rfc 793 */
typedef struct pseudo_header_struct
{
  ip_t source;
  ip_t target;
  u8   zero;
  u8   protocol;
  u16  length;
} __attribute__ ((__packed__)) pseudo_header_t;

/* List of possible tcp states */
typedef enum tcp_state_enum
{
  TCP_LISTEN = 0,
      TCP_SYN_SENT,
      TCP_SYN_RECEIVED,
      TCP_ESTABLISHED,
      TCP_FIN_WAIT_1,
      TCP_FIN_WAIT_2,
      TCP_CLOSING,
      TCP_TIME_WAIT,
      TCP_CLOSE_WAIT,
      TCP_LAST_ACK,
      TCP_CLOSED,
} tcp_state_t;

#if defined(USE_SHELL) || (TCP_DEBUG_TRACE == 1)
const char *tcp_state_name[11] =
{
    "LISTEN",
    "SYN_SENT",
    "SYN_RECEIVED",
    "ESTABLISHED",
    "FIN_WAIT_1",
    "FIN_WAIT_2",
    "CLOSING",
    "TIME_WAIT",
    "CLOSE_WAIT",
    "LAST_ACK",
    "CLOSED"
};
#endif


typedef struct tcb_stub_struct
{
  u32 tx_start;
  u32 rx_start;
  u32 rx_end;
  u32 tx_head;
  u32 tx_tail;
  u32 rx_head;
  u32 rx_tail;
  u32 tx_unacked_tail;
  s32 hole_size;
  u32 future_data_size;

  /** Timeout event */
  u32 timeout_tick;

  /** Window update event */
  u32 update_tick;

  /** Delayed acquittement */
  u32 delayed_ack_time;

  /** Current state */
  tcp_state_t state;

  /** Server socket ? (listening) */
  bool is_server;

  /** Already received one segment and not acked. */
  bool do_ack_next_segment;

  /** Half-full flush already done ? */
  bool has_flushed_half_full;

  /** Buffer full: transmit data as soon as possible */
  bool do_transmit_asap;

  /** Remote IP */
  ip_t remote;

  /** Remote port */
  u16 remote_port;

  /** Local port */
  u16 local_port;

  /** Remote window length */
  u16 remote_window;

  /** Local sequence number */
  u32 local_sn;

  /** Remote sequence number */
  u32 remote_sn;

  u8  retry_count;
  u32 retry_period;
} tcb_stub_t;




/*******************************************
 ******      LOCAL FUNCTIONS        ********
 *******************************************/

static u32      tcp_write(u16 port, const void *buffer, u32 len);
static void     tcp_putc(u16 port, char c);
static void     tcp_flush(u16 port);
static void     tcp_flush_int(u16 port);
static s16      tcp_getc(u16 port, u32 timeout);

static void         tcp_handle_segment(tcp_socket_t s, tcp_header_t *h, u16 len);
static void         tcp_send_packet(tcp_socket_t s, u8 flags);
static tcp_socket_t tcp_find_matching_socket(tcp_header_t *h, ip_t remote);
static void         tcp_swap_header(tcp_header_t* header);
static u8           tcp_process_rx(tcp_socket_t s, tcp_header_t *h, u16 len);
static u32          tcp_count_tx_space(tcp_socket_t s);
static u32          tcp_count_rx_space(tcp_socket_t s);
static u32          tcp_nb_bytes_available(tcp_socket_t s);
static u32          tcp_rand(void);
static void         tcp_tick_task(void);
static void         tcp_setup_timeout(tcp_socket_t s, u16 timeout);
static void         tcp_enable_window_update_timer(tcp_socket_t s);
static void         tcp_enable_delayed_ack_timer(tcp_socket_t s);
static void         tcp_reset_stub(tcp_socket_t s);
static void         tcp_set_state(tcp_socket_t s, tcp_state_t state);



/*******************************************
 ******      LOCAL DATA             ********
 *******************************************/

/** Index of the first tcp iostream */
static iostream_t tcp_iostreams;
static iodevice_t tcp_iodevice = {.putc = tcp_putc, .flush = tcp_flush, .getc = tcp_getc, .seek = NULL, .write = tcp_write, .read = NULL};

static char tcp_buffer[(TCP_TX_FIFO_SIZE+TCP_RX_FIFO_SIZE+2) * MAX_TCP_SOCKETS];
static tcp_header_t      tcp_header;
static pseudo_header_t   pseudo_header;
tcb_stub_t tcb_stubs[MAX_TCP_SOCKETS];

/** Seeds for random generator */
static u32 seed1 = 0x3F67, seed2 = 0x267E;

static u32 tcp_current_tick;

/*******************************************
 ******      IMPLEMENTATION         ********
 *******************************************/
 
 
static void CHECK_INT_SOCKET(tcp_socket_t s)
{
	if(s >= MAX_TCP_SOCKETS) 
	{
		tcp_anomaly("invalid socket."); 
		thread_exit();
	}
}

static u32 tcp_rand(void)
{
  seed1 = 36969 * (seed1 & 0xffff) + (seed1 >> 16);
  seed2 = 36969 * (seed2 & 0xffff) + (seed2 >> 16);
  return (seed1 >> 16) | (seed2 & 0xffff);
}

static void tcp_set_state(tcp_socket_t s, tcp_state_t state)
{
  tcb_stub_t  *ps = &tcb_stubs[s];
  tcp_trace("%d: %s -> %s.", s,
      ps->state < 11 ? tcp_state_name[ps->state] : "?", 
      state < 11 ? tcp_state_name[state] : "?");
  ps->state = state;
}

void tcp_init(void)
{
  tcp_socket_t s;
  tcb_stub_t  *ps;

  tcp_trace("Initialization...");

  tcp_current_tick = 0;  

  tcp_iostreams = register_iodevice(&tcp_iodevice, MAX_TCP_SOCKETS);

  /* Initialize all sockets */
  for(s = 0; s < MAX_TCP_SOCKETS; s++)
  {
    ps = &tcb_stubs[s];
    ps->is_server = false;
    ps->tx_start = ((u32) s) * (TCP_RX_FIFO_SIZE + TCP_TX_FIFO_SIZE + 2);
    ps->rx_start = ps->tx_start + TCP_TX_FIFO_SIZE + 1;
    ps->rx_end   = ps->rx_start + TCP_RX_FIFO_SIZE;
    tcp_reset_stub(s);
  }

  thread_start_ex(&tcp_tick_task, 0, "tcp-tick");
  tcp_trace("init done.");
}



tcp_socket_t tcp_open(u16 port)
{
  tcp_socket_t s;
  tcb_stub_t *ps;

  tcp_trace("open(%2d)...", port);

  for(s = 0; s < MAX_TCP_SOCKETS; s++)
  {
    ps = &tcb_stubs[s];
    if(ps->state == TCP_CLOSED)
    {
      tcp_trace("open: stub[%d] available.", s);
      ps->is_server        = true;
      ps->local_port       = port;
      tcp_set_state(s, TCP_LISTEN);
      return s + tcp_iostreams;
    }
  }
  tcp_anomaly("open: all sockets in use.");
  return INVALID_TCP_SOCKET;
}


tcp_socket_t tcp_connect(ip_t remote_ip, u16 remote_port)
{
  tcp_socket_t s;
  tcb_stub_t *ps;

  for(s = 0; s < MAX_TCP_SOCKETS; s++)
  {
    ps = &tcb_stubs[s];

    if(ps->state != TCP_CLOSED)
      continue;

    ps->local_port  = FIRST_LOCAL_PORT + s;
    ps->remote_port = remote_port;
    ps->remote      = remote_ip;

    tcp_set_state(s, TCP_SYN_SENT);
    tcp_setup_timeout(s, TCP_START_TIMEOUT);
    tcp_send_packet(s, TCP_FLAG_SYN);
    wait(tcp_connection_established + s);
    return s + tcp_iostreams;
  }
  /* No socket available */
  return INVALID_TCP_SOCKET;
}


bool tcp_is_connected(tcp_socket_t s)
{
  return tcb_stubs[s-tcp_iostreams].state == TCP_ESTABLISHED;
}

void tcp_close(tcp_socket_t s)
{
  tcb_stub_t *ps;

  s -= tcp_iostreams;

  ps = &tcb_stubs[s];



  while((ps->state == TCP_ESTABLISHED) && (ps->tx_head != ps->tx_tail))
    wait_with_timeout(tcp_tx_space_available + s, 20 * TICK_MS);
  signal(tcp_tx_space_available + s);

  tcp_trace("Closing %d...", s);

  signal_clear(tcp_closed + s);
  switch(ps->state)
  {
  case TCP_CLOSED:
    tcp_anomaly("Already closed.");
    return;

  case TCP_LISTEN:
    tcp_anomaly("Already closed.");
    //tcp_reset_stub(s);
    break;

  case TCP_LAST_ACK:
  case TCP_FIN_WAIT_1:
  case TCP_FIN_WAIT_2:
  case TCP_CLOSING:
  case TCP_TIME_WAIT:
    /* Already closing */
    wait(tcp_closed + s);
    return;

  case TCP_SYN_SENT:
    tcp_reset_stub(s);
    break;

  case TCP_SYN_RECEIVED:
  case TCP_ESTABLISHED:
    ps->local_sn++;
    tcp_send_packet(s, TCP_FLAG_FIN | TCP_FLAG_ACK);
    tcp_set_state(s, TCP_FIN_WAIT_1);
    tcp_setup_timeout(s, TCP_START_TIMEOUT);
    wait(tcp_closed + s);
    break;

    /* Already received close from other side */
  case TCP_CLOSE_WAIT:
    ps->local_sn++;
    tcp_send_packet(s, TCP_FLAG_FIN | TCP_FLAG_ACK);
    tcp_set_state(s, TCP_LAST_ACK);
    tcp_setup_timeout(s, TCP_START_TIMEOUT);
    wait(tcp_closed + s);
    break;
  }
  tcp_trace("Socket %d now closed.", s);
}

void tcp_flush(u16 s)
{
	tcb_stub_t *ps;
	
	CHECK_INT_SOCKET(s);
	
	ps = &tcb_stubs[s];
	
	while(ps->tx_head != ps->tx_unacked_tail)
	{
		tcp_flush_int(s);
		pause(TICK_MS);
	}
}

void tcp_flush_int(u16 s)
{
  tcb_stub_t *ps;
  
  CHECK_INT_SOCKET(s);
  
  ps = &tcb_stubs[s];
  
  if(ps->state != TCP_ESTABLISHED)
  {
  	return;
  }


  require(tcp_mutex);
  
  if(!mac_is_tx_ready())
  {
  	release(tcp_mutex);
  	return;
  }
  
  tcp_trace("flush socket %d...", s);
  if(ps->tx_head != ps->tx_unacked_tail)
  {
    tcp_send_packet(s, TCP_FLAG_PSH | TCP_FLAG_ACK);
    tcp_setup_timeout(s, TCP_START_TIMEOUT);
  }
  release(tcp_mutex);
}




static u32 tcp_count_rx_space(tcp_socket_t s)
{
  tcb_stub_t  *ps;
  
  CHECK_INT_SOCKET(s);

  ps = &tcb_stubs[s];

  if((ps->state != TCP_ESTABLISHED) && (ps->state != TCP_CLOSE_WAIT))
    return 0;

  if(ps->rx_head >= ps->rx_tail)
    return (ps->rx_end - ps->rx_start) - (ps->rx_head - ps->rx_tail);
  else
    return ps->rx_tail - ps->rx_head - 1;
}

static u32 tcp_count_tx_space(tcp_socket_t s)
{
  tcb_stub_t	*ps;
  
  CHECK_INT_SOCKET(s);

  ps = &tcb_stubs[s];

  if((ps->state != TCP_ESTABLISHED) && (ps->state != TCP_CLOSE_WAIT))
    return 0;

  if(ps->tx_head >= ps->tx_tail)
    return (ps->rx_start - ps->tx_start - 1) - (ps->tx_head - ps->tx_tail);
  else
    return ps->tx_tail - ps->tx_head - 1;
}



void tcp_write_int(u16 s, const char *buffer, u32 len)
{
	tcb_stub_t *ps;
  u32 tx_space;
  u32 len0, len1, head, end;
  
  CHECK_INT_SOCKET(s);
  
  ps = &tcb_stubs[s];
  
  if(ps->state != TCP_ESTABLISHED)
  {
  	return;
  }

  /* If necessary, wait space available in tx fifo */
  while(tcp_count_tx_space(s) < len)
  {
    signal_t sig = wait_one_of(tcp_tx_space_available + s, tcp_closed + s);
    if((sig == tcp_closed + s) && (ps->state != TCP_ESTABLISHED))
    {
    	return;
    }
  }

  require(tcp_mutex);

  tx_space = tcp_count_tx_space(s);

	head = ps->tx_head;
	end  = ps->rx_start - 1;
  
	/*for(i = 0; i < len; i++)
	{
	  tcp_buffer[ps->tx_head++] = buffer[i];
	  if(ps->tx_head >= ps->rx_start)
    	ps->tx_head = ps->tx_start;
	}*/
  
  len0 = len;
  if(head + len > end)
  {
    len0 = (end + 1) - head;
    len1 = len - len0;
  }
  
  memcpy(&(tcp_buffer[head]), buffer, len0);
  if(len1 > 0)
    memcpy(&(tcp_buffer[ps->tx_start]), &(buffer[len0]), len1);
  
  
	ps->tx_head += len;
	if(ps->tx_head >= ps->rx_start)
		ps->tx_head -= (ps->rx_start - ps->tx_start);
	
	
	
	/* Flush if tx fifo is half full */
  /*if((!ps->has_flushed_half_full)
      && (tx_space <= ((ps->rx_start-ps->tx_start)>>1)))
  {
    release(tcp_mutex);
    tcp_flush_int(s);
    require(tcp_mutex);
    ps->has_flushed_half_full = true;
  }*/
  /* Flush if tx fifo is half full */
  if(tx_space <= ((ps->rx_start-ps->tx_start)>>1))
  {
    release(tcp_mutex);
    tcp_flush_int(s);
    require(tcp_mutex);
    //ps->has_flushed_half_full = true;
  }
	

  release(tcp_mutex);

  // Send the last byte as a separate packet (likely will make the remote node send back ACK faster)
  //if(tx_space == 1)
    //tcp_flush_int(s);

  /* Start timeout to flush data periodically */
  tcp_enable_window_update_timer(s);
  
  
  #if 0
  tx_space = tcp_count_tx_space(s);
  /* Flush if tx fifo is half full */
  if((!ps->has_flushed_half_full)
      && (tx_space <= ((ps->rx_start-ps->tx_start)>>1)))
  {
    tcp_flush_int(s);
    ps->has_flushed_half_full = true;
  }
  #endif
}

static u32 tcp_write(u16 s, const void *buffer_, u32 len)
{
	const char *buffer = (const char *) buffer_;
	
	while(len > (TCP_TX_FIFO_SIZE-1))
	{
		tcp_write_int(s, (char *) buffer, (TCP_TX_FIFO_SIZE-1));
		buffer += (TCP_TX_FIFO_SIZE-1);
		len    -= (TCP_TX_FIFO_SIZE-1);
	}
	if(len > 0)
		tcp_write_int(s, buffer, len);
	return len;
}

static void tcp_putc(u16 s, char c)
{
  tcb_stub_t *ps;
  u32 tx_space;
  
  CHECK_INT_SOCKET(s);
  
  ps = &tcb_stubs[s];
  
  if(ps->state != TCP_ESTABLISHED)
  {
  	return;
  }

  /* If necessary, wait space available in tx fifo */
  while(!tcp_count_tx_space(s))
  {
    signal_t sig = wait_one_of(tcp_tx_space_available + s, tcp_closed + s);
    if((sig == tcp_closed + s) && (ps->state != TCP_ESTABLISHED))
    {
    	return;
    }
  }

  require(tcp_mutex);

  tx_space = tcp_count_tx_space(s);

  /* Flush if tx fifo is half full */
  if((!ps->has_flushed_half_full)
      && (tx_space <= ((ps->rx_start-ps->tx_start)>>1)))
  {
    release(tcp_mutex);
    tcp_flush_int(s);
    require(tcp_mutex);
    ps->has_flushed_half_full = true;
  }

  tcp_buffer[ps->tx_head] = c;

  if(++ps->tx_head >= ps->rx_start)
    ps->tx_head = ps->tx_start;

  release(tcp_mutex);

  // Send the last byte as a separate packet (likely will make the remote node send back ACK faster)
  if(tx_space == 1)
    tcp_flush_int(s);

  /* Start timeout to flush data periodically */
  tcp_enable_window_update_timer(s);
}

void tcp_discard(tcp_socket_t s)
{
  tcb_stub_t *ps;
  
  CHECK_INT_SOCKET(s);

  if(tcp_nb_bytes_available(s))
  {
    ps = &tcb_stubs[s];

    /* Delete all data in the RX buffer */
    ps->rx_tail = ps->rx_head;

    // Send a Window update message to the remote node
    tcp_send_packet(s, TCP_FLAG_ACK);
  }
}

static u32 tcp_nb_bytes_available(tcp_socket_t s)
{
  tcb_stub_t	*ps;
  
  CHECK_INT_SOCKET(s);
  
  ps = &tcb_stubs[s];

  if(ps->rx_head >= ps->rx_tail)
    return ps->rx_head - ps->rx_tail;
  else
    return (ps->rx_end - ps->rx_tail + 1) + (ps->rx_head - ps->rx_start);
}

static s16 tcp_getc(u16 s, u32 timeout)
{
  tcb_stub_t	*ps;
  u32 bytes_available;
  s16 res;
  
  CHECK_INT_SOCKET(s);

  bytes_available = tcp_nb_bytes_available(s);
  while(!bytes_available)
  {
  	if(timeout != 0)
  	{
    	if(wait_with_timeout(tcp_rx_data_available+s, timeout) == fail)
      	return -1;
  	}
  	else
  	{
  		wait(tcp_rx_data_available+s);
  	}
    bytes_available = tcp_nb_bytes_available(s);
  }

  ps = &tcb_stubs[s];
  res = ((s16) tcp_buffer[ps->rx_tail]) & 0xff;
  if(++ps->rx_tail > ps->rx_end)
    ps->rx_tail = ps->rx_start;

  if(bytes_available == 1)
    tcp_send_packet(s, TCP_FLAG_ACK);
  else
    tcp_enable_window_update_timer(s);
  return res;
}

static void tcp_setup_timeout(tcp_socket_t s, u16 timeout)
{
  tcb_stub_t *ps;
  
  ps   = &tcb_stubs[s];
  ps->retry_count  = 0;
  ps->retry_period = timeout;
  ps->timeout_tick = tcp_current_tick + ps->retry_period;
}

static void tcp_enable_window_update_timer(tcp_socket_t s)
{
  tcb_stub_t *ps       = &tcb_stubs[s];
  if(ps->update_tick == 0)
    ps->update_tick = tcp_current_tick + TCP_AUTO_TRANSMIT_TIMEOUT;
}

static void tcp_enable_delayed_ack_timer(tcp_socket_t s)
{
  tcb_stub_t *ps       = &tcb_stubs[s];
  if(ps->delayed_ack_time == 0)
    ps->delayed_ack_time = tcp_current_tick + TCP_DELAYED_ACK_TIMEOUT;
}

static void tcp_disable_timeout(tcp_socket_t s)
{
  tcb_stub_t *ps   = &tcb_stubs[s];
  ps->timeout_tick = 0;
}

static void tcp_tick_task(void)
{
  tcp_trace("tick thread started.");

  start_alarm(tcp_tick, TCP_TICK_PERIOD * TICK_MS);
  require(tcp_mutex);
  for(;;)
  {
    tcp_socket_t s;

    release(tcp_mutex);
    wait(tcp_tick);
    require(tcp_mutex);

    tcp_current_tick += TCP_TICK_PERIOD;


    for(s = 0; s < MAX_TCP_SOCKETS; s++)
    {
      tcb_stub_t *ps;
      bool ack_sent;

      ps = &tcb_stubs[s];

      ack_sent = false;

      /* Transmit data as soon as the medium is available */
      if(ps->do_transmit_asap)
      {
        if(mac_is_tx_ready())
        {
          tcp_send_packet(s, TCP_FLAG_ACK);
          ack_sent = true;
        }
      }

      /* Window updates */
      if(!ack_sent && ps->update_tick && (tcp_current_tick > ps->update_tick))
      {
        tcp_send_packet(s, TCP_FLAG_ACK);
        ack_sent = true;
      }

      /* Delayed ACKnowledge */
      if(!ack_sent && ps->delayed_ack_time && (tcp_current_tick > ps->delayed_ack_time))
      {
        tcp_send_packet(s, TCP_FLAG_ACK);
        ack_sent = true;
      }

      /* Timeout occured ? */
      if((ps->timeout_tick == 0) || (tcp_current_tick <= ps->timeout_tick))
        continue;

      /* Double period for next try */
      ps->retry_period *= 2;
      ps->retry_count++;

      /* Update timeout */
      ps->timeout_tick = tcp_current_tick + ps->retry_period;

      if(ps->state != TCP_TIME_WAIT)
      {
        tcp_anomaly("timeout on socket %d: state = %s.", s, tcp_state_name[ps->state]);
      }

      switch(ps->state)
      {
      case TCP_LISTEN:
      case TCP_CLOSED:
        break;
      case TCP_SYN_SENT:
        tcp_setup_timeout(s, TCP_START_TIMEOUT);
        tcp_send_packet(s, TCP_FLAG_SYN);
        break;

      case TCP_SYN_RECEIVED:
        if(ps->retry_count <= MAX_RETRY_COUNTS)
        {
          tcp_trace("Replying SYN+ACK.");
          tcp_send_packet(s, TCP_FLAG_SYN | TCP_FLAG_ACK);
        }
        else
        {
          if(ps->is_server)
          {
            tcp_anomaly("Aborting server connexion.");
            tcp_send_packet(s, TCP_FLAG_RST);
            tcp_reset_stub(s);
          }
          else
          {
            tcp_set_state(s, TCP_SYN_SENT);
            tcp_setup_timeout(s, TCP_START_TIMEOUT);
            tcp_send_packet(s, TCP_FLAG_SYN);
          }
        }
        break;

      case TCP_ESTABLISHED:
        if(ps->tx_unacked_tail != ps->tx_tail)
        {
          /* Retransmit unacknowledged data */
          if(ps->retry_count <= MAX_RETRY_COUNTS)
          {
            tcp_trace("retransmit unacknowledged data.");
            /* Update sequence number */
            ps->local_sn -= (s32) (ps->tx_unacked_tail - ps->tx_tail);
            if(ps->tx_unacked_tail < ps->tx_tail)
              ps->local_sn -= (s32) (ps->rx_start - ps->tx_start);
            ps->tx_unacked_tail = ps->tx_tail;

            tcp_send_packet(s, TCP_FLAG_PSH | TCP_FLAG_ACK);
          }
          else
          {
            tcp_anomaly("Max retry count: aborting.");
            ps->local_sn++;
            // Request to close
            tcp_set_state(s, TCP_FIN_WAIT_1);
            tcp_setup_timeout(s, TCP_START_TIMEOUT);
            tcp_send_packet(s, TCP_FLAG_FIN | TCP_FLAG_ACK);
          }
        }
        break;

      case TCP_FIN_WAIT_1:
        if(ps->retry_count <= MAX_RETRY_COUNTS)
        {
          ps->local_sn -= (signed long)(signed short)(ps->tx_unacked_tail - ps->tx_tail);
          if(ps->tx_unacked_tail < ps->tx_tail)
            ps->local_sn -= (signed long)(signed short)(ps->rx_start - ps->tx_start);
          ps->tx_unacked_tail = ps->tx_tail;
          tcp_send_packet(s, TCP_FLAG_FIN | TCP_FLAG_ACK);
        }
        else
        {
          tcp_send_packet(s, TCP_FLAG_RST | TCP_FLAG_ACK);
          tcp_reset_stub(s);
        }
        break;

      case TCP_FIN_WAIT_2:
        tcp_send_packet(s, TCP_FLAG_RST | TCP_FLAG_ACK);
        tcp_reset_stub(s);
        break;

      case TCP_CLOSING:
        if(ps->retry_count <= MAX_RETRY_COUNTS)
        {
          ps->local_sn -= (signed long)(signed short)(ps->tx_unacked_tail - ps->tx_tail);
          if(ps->tx_unacked_tail < ps->tx_tail)
            ps->local_sn -= (signed long)(signed short)(ps->rx_start - ps->tx_start);
          ps->tx_unacked_tail = ps->tx_tail;

          tcp_send_packet(s, TCP_FLAG_ACK);
        }
        else
        {
          tcp_send_packet(s, TCP_FLAG_RST | TCP_FLAG_ACK);
          tcp_reset_stub(s);
        }
        break;

      case TCP_TIME_WAIT:
        tcp_reset_stub(s);
        break;

      case TCP_CLOSE_WAIT:
        if(ps->retry_count <= MAX_RETRY_COUNTS)
        {
          ps->local_sn -= (signed long)(signed short)(ps->tx_unacked_tail - ps->tx_tail);
          if(ps->tx_unacked_tail < ps->tx_tail)
            ps->local_sn -= (signed long)(signed short)(ps->rx_start - ps->tx_start);
          ps->tx_unacked_tail = ps->tx_tail;

          tcp_send_packet(s, TCP_FLAG_ACK);
        }
        else
        {
          tcp_send_packet(s, TCP_FLAG_RST | TCP_FLAG_ACK);
          tcp_reset_stub(s);
        }
        break;

      case TCP_LAST_ACK:
        if(ps->retry_count <= MAX_RETRY_COUNTS)
        {
          tcp_send_packet(s, TCP_FLAG_FIN | TCP_FLAG_ACK);
        }
        else
        {
          tcp_send_packet(s, TCP_FLAG_RST | TCP_FLAG_ACK);
          tcp_reset_stub(s);
        }
        break;
      }
    }
  }
}





void tcp_process(ip_t remote, ip_t local_ip, u16 len)
{
  tcp_socket_t socket;
  volatile u16  checksum_1;
  volatile u16  checksum_2;
  u8   options_length;

  tcp_trace("process %2d bytes from %i...", len, &remote);
  
  require(tcp_mutex);

  /* Compute IP pseudo-header checksum */
  pseudo_header.source   = remote;
  pseudo_header.target   = local_ip;
  pseudo_header.zero     = 0x0;
  pseudo_header.protocol = IP_PROTOCOL_TCP;
  pseudo_header.length   = len;

  /* Endianness conversion */
  pseudo_header.length = swaps(pseudo_header.length);

  checksum_1 = ~ip_compute_checksum(0xffff, (char*)&pseudo_header, sizeof(pseudo_header_t));
  checksum_2 = ip_compute_rx_checksum(len);

  if(checksum_1 != checksum_2)
  {
  	ip_discard_rx();
  	release(tcp_mutex);
    tcp_anomaly("tcp process: bad checksum (%2x VS %2x), len = %2d.", checksum_1, checksum_2, len);
    return;
  }

  ip_set_read_ptr(0);
  gread(ip_ios, &tcp_header, sizeof(tcp_header_t));
  tcp_swap_header(&tcp_header);


  /* Skip options */
  options_length = (u8)(((tcp_header.data_offset >> 4) << 2) - sizeof(tcp_header_t));
  len = len - options_length - sizeof(tcp_header_t);

  socket = tcp_find_matching_socket(&tcp_header, remote);
  if(socket != INVALID_TCP_SOCKET)
    tcp_handle_segment(socket, &tcp_header, len);
  else
  {
    tcp_anomaly("tcp process: unknown socket.");
    ip_discard_rx();
  }
  release(tcp_mutex);
}

struct profile_struct
{
  u64 cumulative_send_packet;
  u64   cumulative_send_packet_checksum;
  u64   cumulative_send_packet_gwrite;
  u64 cumulative_gflush;
} profile = {0, 0, 0, 0};

counter_t sp_cnt;
counter_t sp_cnt_2;

u32 tcp_nb_redo = 0;
u32 tcp_nb_ocas = 0;
static void tcp_send_packet(tcp_socket_t s, u8 flags)
{
  tcb_stub_t		*ps;
  tcp_header_t      header;
  tcp_options_t     options;
  u32 			len;
  u16 checksum;
  u32 nb_trans = 0;
  
  redo:
  
  counter_start(&sp_cnt);

# if (TCP_DEBUG_TRACE == 1)
  eprintf("send(%d): flags = ", s);
  if(flags & TCP_FLAG_SYN)
    eprintf("SYN ");
  if(flags & TCP_FLAG_FIN)
    eprintf("FIN ");
  if(flags & TCP_FLAG_RST)
    eprintf("RST ");
  if(flags & TCP_FLAG_PSH)
    eprintf("PSH ");
  if(flags & TCP_FLAG_ACK)
    eprintf("ACK ");
  if(flags & TCP_FLAG_URG)
    eprintf("URG ");
  eprintf("\r\n");
# endif

	checksum = 0xffff;

  ps = &tcb_stubs[s];

  /* Write IP header */
  ip_begin_packet(ps->remote, IP_PROTOCOL_TCP);
  /* Skip tcp header */
  if(flags & TCP_FLAG_SYN)
    ip_set_write_ptr(sizeof(tcp_header_t) + sizeof(tcp_options_t));
  else
    ip_set_write_ptr(sizeof(tcp_header_t));


  /* Disable retransmission timers */
  ps->update_tick         = 0;
  ps->delayed_ack_time    = 0;
  ps->do_transmit_asap    = false;
  ps->do_ack_next_segment = false;

  if(flags & TCP_FLAG_SYN)
  {
    /* No data for SYN packets */
    len = 0;
  }
  else
  {
    if(ps->tx_head == ps->tx_unacked_tail)
    {
      len = 0;
    }
    else
    {
      u32 effective_wnd, lens[3];
      char *packets[3];
      char temp[2];
      u8 i;

      len = modulo_sub(ps->tx_head, ps->tx_unacked_tail, TCP_TX_FIFO_SIZE + 1);
      effective_wnd = ps->remote_window - modulo_sub(ps->tx_unacked_tail, ps->tx_tail, TCP_TX_FIFO_SIZE + 1);
      if(len > effective_wnd)
        len = effective_wnd;

      if(len > TCP_MAX_MTU_SIZE)
      {
        len = TCP_MAX_MTU_SIZE;
        ps->do_transmit_asap = true;
      }

      packets[0] = &(tcp_buffer[ps->tx_unacked_tail]);
      packets[1] = temp;

      /* Data is contiguous in the buffer, no problem */
      if(ps->tx_head > ps->tx_unacked_tail)
      {
      	/* Ensure length is even */
      	if(len & 1)
      		len--;
      		
        lens[0] = len;
        lens[1] = 0;
        lens[2] = 0;
        
      }
      /* Data is not contiguous in the buffer */
      else
      {
      	/* end of buffer */
      	lens[0] = min(len, ps->rx_start - ps->tx_unacked_tail);
        lens[1] = 0;
        /* begin of buffer */
        packets[2] = &(tcp_buffer[ps->tx_start]);
        lens[2]    = len - lens[0];
        
        if((lens[0] & 1) && (lens[2] > 0))
        {
        	u32 index = ps->tx_unacked_tail + lens[0];
          if(index > 0)
          	index--;
          	
        	lens[0]--;
        	lens[1]--;
        	lens[1] = 2;
        	temp[0] = tcp_buffer[index];
        	temp[1] = tcp_buffer[ps->tx_start];
        	packets[2] = &(tcp_buffer[ps->tx_start+1]);
        	
        	if(lens[2] & 1)
        		lens[2]--;
        }
        else if(lens[0] & 1)
        {
        	lens[0]--;
        }
      }

			len = 0;
      for(i = 0; i < 3; i++)
      {	
      	if(lens[i] > 0)
      	{
      		len += lens[i];
	        
          counter_start(&sp_cnt_2);
          
	        gwrite(ip_ios, packets[i], lens[i]);
          profile.cumulative_send_packet_gwrite += counter_get(&sp_cnt_2);
	        checksum = ip_compute_checksum(checksum, packets[i], lens[i]);
          profile.cumulative_send_packet_checksum += counter_get(&sp_cnt_2);
      	}
      }
      ps->tx_unacked_tail += len;
      if(ps->tx_unacked_tail >= ps->rx_start)
        ps->tx_unacked_tail -= (TCP_TX_FIFO_SIZE + 1);

      tcp_trace("Sent packet, len = %2d, len_0 = %2d, len_1 = %2d, len_2 = %2d. hd=%2d,tl=%2d,unatl=%2d, sn = %4d.", len, lens[0], lens[1], lens[2], ps->tx_head, ps->tx_tail, ps->tx_unacked_tail, ps->local_sn);
    }
  }

  header.source_port			  = ps->local_port;
  header.target_port				= ps->remote_port;
  if(flags & (TCP_FLAG_SYN | TCP_FLAG_FIN))
    header.sequence_number  = ps->local_sn-1;
  else
    header.sequence_number		= ps->local_sn;
  header.ack_number			    = ps->remote_sn;
  header.flags			        = flags;
  header.urgent_pointer     = 0;

  /* Update local sequence number */
  ps->local_sn += len;

  header.window = tcp_count_rx_space(s);

  if(header.window == 0)
  {
    tcp_trace("Tx zero window...");
  }

  tcp_swap_header(&header);

  len += sizeof(header);

  header.data_offset = (sizeof(tcp_header_t) >> 2) << 4;
  if(flags & TCP_FLAG_SYN)
  {
    len += sizeof(options);
    options.kind   = OPTION_MAX_SEGMENT_SIZE;
    options.length = 0x04;
    options.w1     = 0x6400;
    header.data_offset += (sizeof(tcp_options_t) >> 2) << 4;
  }

  /* Compute pseudo-header checksum */
  pseudo_header.source	 = current_ip_address;
  pseudo_header.target   = ps->remote;
  pseudo_header.zero     = 0x0;
  pseudo_header.protocol = IP_PROTOCOL_TCP;
  pseudo_header.length	 = len;

  /* Endianness conversion */
  pseudo_header.length = swaps(pseudo_header.length);

  header.checksum = 0x0000;
  checksum = ip_compute_checksum(checksum, (char*)&pseudo_header, sizeof(pseudo_header_t));

  ip_set_write_ptr(0);
  gwrite(ip_ios, &header, sizeof(tcp_header_t));
  checksum = ip_compute_checksum(checksum, (char*)&header, sizeof(tcp_header_t));

  if(flags & TCP_FLAG_SYN)
  {
    gwrite(ip_ios, (char*)&options, sizeof(options));
    checksum = ip_compute_checksum(checksum, (char*)&options, sizeof(tcp_options_t));
  }

  /* Update tcp packet checksum */
  ip_set_write_ptr(16);
  gwrite(ip_ios, (char*)&checksum, 2);
  
  profile.cumulative_send_packet += counter_get(&sp_cnt);
  
  gflush(ip_ios);
  profile.cumulative_gflush += counter_get(&sp_cnt);
  
  
  nb_trans++;
  tcp_nb_ocas++;
  if(ps->do_transmit_asap && mac_is_tx_ready())
  {
  	//tprintf("T%4d.\r\n", nb_trans);
  	//pause(15 * TICK_MS);
  	tcp_nb_redo++;
    goto redo;
  }
}






static tcp_socket_t tcp_find_matching_socket(tcp_header_t *h, ip_t remote)
{
  tcb_stub_t *ps;
  tcp_socket_t s;
  tcp_socket_t partial_match;

  partial_match = INVALID_TCP_SOCKET;

  for(s = 0; s < MAX_TCP_SOCKETS; s++ )
  {
    ps = &tcb_stubs[s];

    if(ps->state == TCP_CLOSED)
      continue;
    else if(ps->state == TCP_LISTEN)
    {
      if(ps->local_port == h->target_port)
        partial_match = s;
      continue;
    }
    else if((h->target_port == ps->local_port) &&
            (h->source_port == ps->remote_port) &&
            (remote.val == ps->remote.val))
      return s;
  }

  if(partial_match == INVALID_TCP_SOCKET)
    return INVALID_TCP_SOCKET;

  ps = &tcb_stubs[partial_match];

  ps->tx_head          = ps->tx_start;
  ps->tx_tail          = ps->tx_start;
  ps->rx_head          = ps->rx_start;
  ps->rx_tail          = ps->rx_start;
  ps->tx_unacked_tail  = ps->tx_start;

  ps->remote          = remote;
  ps->remote_port     = h->source_port;
  ps->local_port      = h->target_port;
  ps->local_sn        = tcp_rand();


  tcp_trace("Found partial match. Local port = %2d, remote port = %2d.", ps->local_port, ps->remote_port);
  return partial_match;
}

static void tcp_swap_header(tcp_header_t* header)
{
  header->source_port     = swaps(header->source_port);
  header->target_port     = swaps(header->target_port);
  header->sequence_number = swapl(header->sequence_number);
  header->ack_number      = swapl(header->ack_number);
  header->window          = swaps(header->window);
  header->checksum        = swaps(header->checksum);
  header->urgent_pointer  = swaps(header->urgent_pointer);
}

static void tcp_reset_stub(tcp_socket_t s)
{
  tcb_stub_t *ps = &tcb_stubs[s];

  tcp_set_state(s, ps->is_server ? TCP_LISTEN : TCP_CLOSED);
  ps->timeout_tick         = 0;
  ps->update_tick          = 0;
  ps->delayed_ack_time     = 0;
  ps->do_ack_next_segment  = false;
  ps->has_flushed_half_full= false;
  ps->do_transmit_asap     = false;
  ps->tx_tail              = ps->tx_start;
  ps->tx_head              = ps->tx_start;
  ps->rx_tail              = ps->rx_start;
  ps->rx_head              = ps->rx_start;
  ps->tx_unacked_tail      = ps->tx_start;
  ps->local_sn             = tcp_rand();
  ps->hole_size            = -1;
  tcp_disable_timeout(s);
  signal(tcp_closed + s);
}

static void tcp_handle_segment(tcp_socket_t s, tcp_header_t *h, u16 len)
{
  tcb_stub_t *ps;
  u8 flags;

  ps = &tcb_stubs[s];
  flags = 0x00;

  //tcp_trace("handle seg. s %d, len %2d.", s, len);
# if (TCP_DEBUG_TRACE == 1)
  eprintf("handle(%d): flags = ", s);
  if(h->flags & TCP_FLAG_SYN)
    eprintf("SYN ");
  if(h->flags & TCP_FLAG_FIN)
    eprintf("FIN ");
  if(h->flags & TCP_FLAG_RST)
    eprintf("RST ");
  if(h->flags & TCP_FLAG_PSH)
    eprintf("PSH ");
  if(h->flags & TCP_FLAG_ACK)
    eprintf("ACK ");
  if(h->flags & TCP_FLAG_URG)
    eprintf("URG ");
  eprintf("\r\n");
# endif

  /* Reset */
  if(h->flags & TCP_FLAG_RST)
  {
    tcp_send_packet(s, TCP_FLAG_RST);
    tcp_anomaly("Got RST from pair.");
    tcp_reset_stub(s);
    ip_discard_rx();
    return;
  }


  if((ps->remote_window == 0) && h->window)
    ps->do_transmit_asap = true;
  ps->remote_window = h->window;

  switch(ps->state)
  {
  case TCP_LISTEN:
    if(!(h->flags & TCP_FLAG_SYN))
    {
      tcp_send_packet(s, TCP_FLAG_RST);
      tcp_reset_stub(s);
      break;
    }

    flags = TCP_FLAG_SYN | TCP_FLAG_ACK;
    tcp_set_state(s, TCP_SYN_RECEIVED);
    tcp_setup_timeout(s, TCP_START_TIMEOUT);
    ps->remote_sn = h->sequence_number + 1;
    break;

  case TCP_SYN_SENT:
    if(!(h->flags & TCP_FLAG_SYN))
      break;

    if(h->flags & TCP_FLAG_ACK)
    {
      if(h->ack_number == ps->local_sn)
      {
        flags = TCP_FLAG_ACK;
        tcp_set_state(s, TCP_ESTABLISHED);
        signal(tcp_connection_established + s);
        tcp_disable_timeout(s);
        flags |= tcp_process_rx(s, h, len);
      }
      else
      {
        flags = TCP_FLAG_RST;
        break;
      }
    }
    else
    {
      flags = TCP_FLAG_SYN | TCP_FLAG_ACK;
      tcp_set_state(s, TCP_SYN_RECEIVED);
      tcp_setup_timeout(s, TCP_START_TIMEOUT);
    }

    ps->remote_sn = h->sequence_number + 1;

    break;

  case TCP_SYN_RECEIVED:
    if((h->flags & TCP_FLAG_ACK) && (h->ack_number == ps->local_sn))
    {
      tcp_set_state(s, TCP_ESTABLISHED);
      signal(tcp_connection_established + s);
      tcp_disable_timeout(s);
      flags |= tcp_process_rx(s, h, len);
    }
    break;

  case TCP_ESTABLISHED:
    flags |= tcp_process_rx(s, h, len);

    if(h->flags & TCP_FLAG_FIN)
    {
      ps->remote_sn++;

      if(len == 0)
      {
        flags = TCP_FLAG_FIN | TCP_FLAG_ACK;
        ps->local_sn++;
        tcp_set_state(s, TCP_LAST_ACK);
      }
      else
      {
        /* If rx data remaining to be processed,
         * wait for local application to close the socket. */
        flags = TCP_FLAG_ACK;
        tcp_set_state(s, TCP_CLOSE_WAIT);
      }

      tcp_setup_timeout(s, TCP_START_TIMEOUT);
    }

    break;

  case TCP_FIN_WAIT_1:
    if((h->flags & TCP_FLAG_FIN) && (h->flags & TCP_FLAG_ACK) && (h->ack_number == ps->local_sn))
    {
      ps->remote_sn++;
      flags = TCP_FLAG_ACK;
      tcp_set_state(s, TCP_TIME_WAIT);
      tcp_setup_timeout(s, TCP_TIME_WAIT_TIMEOUT);
    }
    else if((h->flags & TCP_FLAG_ACK) && (h->ack_number == ps->local_sn))
    {
      tcp_set_state(s, TCP_FIN_WAIT_2);
    }
    else if(h->flags & TCP_FLAG_FIN)
    {
      ps->remote_sn++;
      flags = TCP_FLAG_ACK;
      tcp_set_state(s, TCP_CLOSING);
      tcp_setup_timeout(s, TCP_START_TIMEOUT);
    }

    flags |= tcp_process_rx(s, h, len);

    break;

  case TCP_FIN_WAIT_2:
    if(h->flags & TCP_FLAG_FIN)
    {
      ps->remote_sn++;
      flags = TCP_FLAG_ACK;
      tcp_set_state(s, TCP_TIME_WAIT);
      tcp_setup_timeout(s, TCP_TIME_WAIT_TIMEOUT);
    }

    flags |= tcp_process_rx(s, h, len);
    break;

  case TCP_CLOSING:
    if((h->flags & TCP_FLAG_ACK) && (h->ack_number == ps->local_sn))
    {
      tcp_set_state(s, TCP_TIME_WAIT);
      tcp_setup_timeout(s, TCP_TIME_WAIT_TIMEOUT);
    }
    break;

  case TCP_CLOSE_WAIT:
    flags = TCP_FLAG_ACK;
    break;

  case TCP_LAST_ACK:
    if((h->flags & TCP_FLAG_ACK) && (h->ack_number == ps->local_sn))
      tcp_reset_stub(s);
    break;

  case TCP_TIME_WAIT:
    flags = TCP_FLAG_RST;
    break;

  case TCP_CLOSED:
    break;
  }


  /* Discard acknowledged data from the transmit fifo */
  if((h->flags & TCP_FLAG_ACK) && (ps->state != TCP_CLOSED) && (ps->state != TCP_LISTEN))
  {
    s32 nb_bytes_sent, nb_bytes_acked;

    nb_bytes_sent  = ps->local_sn - modulo_sub(ps->tx_unacked_tail, ps->tx_tail, TCP_TX_FIFO_SIZE + 1);
    nb_bytes_acked = h->ack_number - nb_bytes_sent;

    tcp_trace("handle ack: sent = %2d, acked = %2d, local sn = %4d, ack number = %4d.", nb_bytes_sent, nb_bytes_acked, ps->local_sn, h->ack_number);

    if((nb_bytes_acked > 0) && (nb_bytes_acked <= ps->rx_start - ps->tx_start))
    {
      ps->has_flushed_half_full = false;

      ps->tx_tail += nb_bytes_acked;
      if(ps->tx_tail >= ps->rx_start)
        ps->tx_tail -= (TCP_TX_FIFO_SIZE + 1);

      signal(tcp_tx_space_available + s);

      /* Disable timeout if all tx data acked */
      if((ps->state == TCP_ESTABLISHED) && (ps->tx_tail == ps->tx_unacked_tail))
        tcp_disable_timeout(s);
      /* Reset timeout if only some tx data acked */
      else if(ps->state == TCP_ESTABLISHED)
      {
        // TODO.
      }
    }
  }

  ip_discard_rx();

  if(flags)
    tcp_send_packet(s, flags);
}


static u8 tcp_process_rx(tcp_socket_t s, tcp_header_t *h, u16 len)
{
  tcb_stub_t *ps = &tcb_stubs[s];
  u32 temp;
  s32 missing_bytes;
  u32 wMissingBytes;
  u32 free_space;



  tcp_trace("%d: process rx bytes, len = %2d.", s, len);

  if(len == 0)
    return 0;

  // Calculate the number of bytes ahead of our head pointer this segment skips
  missing_bytes = h->sequence_number - ps->remote_sn;
  wMissingBytes = (u16)missing_bytes;

  free_space = tcp_count_rx_space(s);

  // See if any of the segment data is within our RX window
  if(((s32)free_space > missing_bytes) && ((s16)missing_bytes + (s32)len > 0))
  {
    // See if there are bytes we must skip
    if((s32)wMissingBytes <= 0)
    {
      // Position packet read pointer to start of useful data area.
      ip_set_read_ptr(((h->data_offset >> 4) << 2) - wMissingBytes);
      len += wMissingBytes;
      ps->remote_sn += len;

      // Copy the application data from the packet into the socket RX FIFO
      // See if we need a two part copy (spans rx_end->rx_start)
      if(ps->rx_head + len > ps->rx_end)
      {
        temp = ps->rx_end - ps->rx_head + 1;
        gread(ip_ios, &(tcp_buffer[ps->rx_head]), temp);
        gread(ip_ios, &(tcp_buffer[ps->rx_start]), len - temp);
        ps->rx_head = ps->rx_start + (len - temp);
      }
      else
      {
        gread(ip_ios, &(tcp_buffer[ps->rx_head]), len);
        ps->rx_head += len;
      }
      if(len > 0)
        signal(tcp_rx_data_available + s);

      // See if we have a hole and other data waiting already in the RX FIFO
      if(ps->hole_size != -1)
      {
        ps->hole_size -= len;
        temp = ps->future_data_size + ps->hole_size;

        // See if we just closed up a hole, and if so, advance head pointer
        if((s32)temp < 0)
        {
          ps->hole_size = -1;
        }
        else if(ps->hole_size <= 0)
        {
          ps->remote_sn += temp;
          ps->rx_head += temp;
          if(ps->rx_head > ps->rx_end)
            ps->rx_head -= ps->rx_end - ps->rx_start + 1;
          ps->hole_size = -1;
        }
      }
    }
    // This packet is out of order or we lost a packet, see if we can generate a hole to accomodate it
    else if((s32)wMissingBytes > 0)
    {
      // Position packet read pointer to start of useful data area.
      ip_set_read_ptr((h->data_offset >> 4) << 2);

      // See if we need a two part copy (spans rx_end->rx_start)
      if(ps->rx_head + wMissingBytes + len > ps->rx_end)
      {
        // Calculate number of data bytes to copy before wraparound
        temp = ps->rx_end - ps->rx_head + 1 - wMissingBytes;
        if((s32)temp >= 0)
        {
          gread(ip_ios, &(tcp_buffer[ps->rx_head + wMissingBytes]), temp);
          gread(ip_ios, &(tcp_buffer[ps->rx_start]), len - temp);
        }
        else
        {
          gread(ip_ios, &(tcp_buffer[ps->rx_head + wMissingBytes - (ps->rx_end - ps->rx_start + 1)]), len);
        }
      }
      else
        gread(ip_ios, &(tcp_buffer[ps->rx_head + wMissingBytes]), len);

      // Record the hole is here
      if(ps->hole_size == -1)
      {
        ps->hole_size = wMissingBytes;
        ps->future_data_size = len;
      }
      else
      {
        // We already have a hole, see if we can shrink the hole
        // or extend the future data size
        if(wMissingBytes < (u32) ps->hole_size)
        {
          if((wMissingBytes + len > (u32) ps->hole_size + ps->future_data_size) || (wMissingBytes + len < (u32) ps->hole_size))
            ps->future_data_size = len;
          else
            ps->future_data_size = (u32) ps->hole_size + ps->future_data_size - wMissingBytes;
          ps->hole_size = wMissingBytes;
        }
        else if(wMissingBytes + len > (u32) ps->hole_size + ps->future_data_size)
        {
          ps->future_data_size += wMissingBytes + len - (u32) ps->hole_size - ps->future_data_size;
        }
      }
      tcp_send_packet(s, TCP_FLAG_ACK);
    }
  }



  /* Immediatly ack if received two successive packets */
  if(ps->do_ack_next_segment)
    return TCP_FLAG_ACK;

  /* Otherwise, differed ack */
  ps->do_ack_next_segment = true;
  tcp_enable_delayed_ack_timer(s);
  return 0;
}

void tcp_listen(tcp_socket_t s, ip_t *remote_ip, u16 *remote_port)
{
	tcb_stub_t *ps;
	
	s -= tcp_iostreams;
	
	CHECK_INT_SOCKET(s);
	
	ps = &tcb_stubs[s];
	
  wait(tcp_connection_established + s);
  
  *remote_ip   = ps->remote;
  *remote_port = ps->remote_port;
}

#ifdef USE_SHELL
retcode tcp_cmde_infos(void)
{
  tcp_socket_t s;
  tprintf("TCP sockets:\r\n");
  for(s = 0; s < MAX_TCP_SOCKETS; s++)
  {
    tcb_stub_t *ps = &(tcb_stubs[s]);
    tprintf("%x: %s, ", s, (ps->state <= 11) ? tcp_state_name[ps->state] : "UNKNOWN");
    if(ps->state == TCP_ESTABLISHED)
    {
      tprintf("\r\nrx fifo = %2d\r\n", tcp_nb_bytes_available(s));
      tprintf("tx start = %2d, tx tail = %2d, tx head = %2d.\r\n", ps->tx_start, ps->tx_tail, ps->tx_head);
      tprintf("tx unacked tail = %2d.\r\n", ps->tx_unacked_tail);
      tprintf("rx start = %2d, rx tail = %2d, rx head = %2d.\r\n", ps->rx_start, ps->rx_tail, ps->rx_head);

      tprintf("rx end = %2d.\r\n", ps->rx_end);
    }
    tprintf("\r\n");
  }
  return ok;
}
#endif
