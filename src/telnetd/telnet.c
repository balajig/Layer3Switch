//
// telnetd.c
//
// Telnet daemon
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#if 0
#include "common_types.h"
#include "sockets.h"
#include "socks.h"
#include "api.h"

//
// Events
//

#define APPDON  0  // Application terminated
#define USRATT  10 // User needs attention for reading input
#define USRRDY  11 // User is ready to receive output
#define APPATT  12 // Application needs attention for writing output
#define APPRDY  13 // Application is ready to receive input

//
// States
//

#define STATE_NORMAL 0
#define STATE_IAC    1
#define STATE_OPT    2
#define STATE_SB     3
#define STATE_OPTDAT 4
#define STATE_SE     5

//
// Special telnet characters
//

#define TELNET_SE    240   // End of subnegotiation parameters
#define TELNET_NOP   241   // No operation
#define TELNET_MARK  242   // Data mark
#define TELNET_BRK   243   // Break
#define TELNET_IP    244   // Interrupt process
#define TELNET_AO    245   // Abort output
#define TELNET_AYT   246   // Are you there
#define TELNET_EC    247   // Erase character
#define TELNET_EL    248   // Erase line
#define TELNET_GA    249   // Go ahead
#define TELNET_SB    250   // Start of subnegotiation parameters
#define TELNET_WILL  251   // Will option code
#define TELNET_WONT  252   // Won't option code
#define TELNET_DO    253   // Do option code
#define TELNET_DONT  254   // Don't option code
#define TELNET_IAC   255   // Interpret as command

//
// Telnet options
//

#define TELOPT_TRANSMIT_BINARY      0  // Binary Transmission (RFC856)
#define TELOPT_ECHO                 1  // Echo (RFC857)
#define TELOPT_SUPPRESS_GO_AHEAD    3  // Suppress Go Ahead (RFC858)
#define TELOPT_STATUS               5  // Status (RFC859)
#define TELOPT_TIMING_MARK          6  // Timing Mark (RFC860)
#define TELOPT_NAOCRD              10  // Output Carriage-Return Disposition (RFC652)
#define TELOPT_NAOHTS              11  // Output Horizontal Tab Stops (RFC653)
#define TELOPT_NAOHTD              12  // Output Horizontal Tab Stop Disposition (RFC654)
#define TELOPT_NAOFFD              13  // Output Formfeed Disposition (RFC655)
#define TELOPT_NAOVTS              14  // Output Vertical Tabstops (RFC656)
#define TELOPT_NAOVTD              15  // Output Vertical Tab Disposition (RFC657)
#define TELOPT_NAOLFD              16  // Output Linefeed Disposition (RFC658)
#define TELOPT_EXTEND_ASCII        17  // Extended ASCII (RFC698)
#define TELOPT_TERMINAL_TYPE       24  // Terminal Type (RFC1091)
#define TELOPT_NAWS                31  // Negotiate About Window Size (RFC1073)
#define TELOPT_TERMINAL_SPEED      32  // Terminal Speed (RFC1079)
#define TELOPT_TOGGLE_FLOW_CONTROL 33  // Remote Flow Control (RFC1372)
#define TELOPT_LINEMODE            34  // Linemode (RFC1184)
#define TELOPT_AUTHENTICATION      37  // Authentication (RFC1416)

//
// Globals
//

#define STOPPED 0
#define RUNNING 1

#define TERM_UNKNOWN   0
#define TERM_CONSOLE   1
#define TERM_VT100     2

#define CRTBASESIZE    (16 + 3 * 32 + 512 + 5 * 4 + 4)

struct term
{
  int type;
  int cols;
  int lines;
};

int port;
int sock = -1;
int state = STOPPED;

int off = 0;
int on = 1;

struct buffer
{
	unsigned char data[4096];
	unsigned char *start;
	unsigned char *end;
};

struct termstate
{
	int sock;
	int state;
	int code;
	unsigned char optdata[256];
	int optlen;
	struct term term;
	struct buffer bi;
	struct buffer bo;
};

void sendopt(struct termstate *ts, int code, int option)
{
	unsigned char buf[3];

	return 0;
	buf[0] = TELNET_IAC;
	buf[1] = (unsigned char) code;
	buf[2] = (unsigned char) option;
	write(ts->sock, buf, 3);
}

void parseopt(struct termstate *ts, int code, int option)
{
	switch (option)
	{
		case TELOPT_ECHO:
			break;

		case TELOPT_SUPPRESS_GO_AHEAD:
			if (code == TELNET_WILL || code == TELNET_WONT)
				sendopt(ts, TELNET_DO, option);
			else
				sendopt(ts, TELNET_WILL, option);
			break;

		case TELOPT_TERMINAL_TYPE:
		case TELOPT_NAWS:
		case TELOPT_TERMINAL_SPEED:
			sendopt(ts, TELNET_DO, option);
			break;

		default:
			if (code == TELNET_WILL || code == TELNET_WONT)
				sendopt(ts, TELNET_DONT, option);
			else
				sendopt(ts, TELNET_WONT, option);
	}
}

void parseoptdat(struct termstate *ts, int option, unsigned char *data, int len)
{

	switch (option)
	{
		case TELOPT_NAWS:
			if (len == 4)
			{
				int cols = ntohs(*(unsigned short *) data);
				int lines = ntohs(*(unsigned short *) (data + 2));
				if (cols != 0) ts->term.cols = cols;
				if (lines != 0) ts->term.lines = lines;
			}
			break;

		case TELOPT_TERMINAL_SPEED:
			break;

		case TELOPT_TERMINAL_TYPE:
			break;
	}
}

void parse(struct termstate *ts, int c)
{
	switch (ts->state) 
	{
		case STATE_NORMAL:
			if (c == TELNET_IAC) {
				read (ts->sock, c, 1);
				ts->state = STATE_IAC;
			}
			break;

		case STATE_IAC:
			switch (c) 
			{
				case TELNET_IAC:
					ts->state = STATE_NORMAL;
					break;

				case TELNET_WILL:
				case TELNET_WONT:
				case TELNET_DO:
				case TELNET_DONT:
					ts->code = c;
					ts->state = STATE_OPT;
					break;

				case TELNET_SB:
					ts->state = STATE_SB;
					break;

				default:
					ts->state = STATE_NORMAL;
			}
			break;

		case STATE_OPT:
			parseopt(ts, ts->code, c);
			ts->state = STATE_NORMAL;
			break;

		case STATE_SB:
			ts->code = c;
			ts->optlen = 0;
			ts->state = STATE_OPTDAT;
			break;

		case STATE_OPTDAT:
			if (c == TELNET_IAC)
				ts->state = STATE_SE;
			else if (ts->optlen < sizeof(ts->optdata))
				ts->optdata[ts->optlen++] = c;
			break;

		case STATE_SE:
			if (c == TELNET_SE) parseoptdat(ts, ts->code, ts->optdata, ts->optlen);
			ts->state = STATE_NORMAL;
			break;
	} 
}

void telnet_task (void *arg)
{
	int s = (int) arg;
	int app;
	int n;
	struct termstate ts;
	struct sockaddr_in sin;
	int sinlen = sizeof sin;
	int session = -1;
	int last_was_cr = 0;

	getpeername(s, (struct sockaddr *) &sin, &sinlen);

	// Initialize terminal state
	memset(&ts, 0, sizeof(struct termstate));
	ts.sock = s;
	ts.state = STATE_NORMAL;
	ts.term.type = TERM_VT100;
	ts.term.cols = 80;
	ts.term.lines = 25;

	// Send initial options
	sendopt(&ts, TELNET_WILL, TELOPT_ECHO);
	sendopt(&ts, TELNET_WILL, TELOPT_SUPPRESS_GO_AHEAD);
	sendopt(&ts, TELNET_WONT, TELOPT_LINEMODE);
	sendopt(&ts, TELNET_DO, TELOPT_NAWS);

	if ((session = cli_telnet_session_init ("telent@OpenSwitch",  s)) < 0)
		return -1;

	cli_start_session (session);

	while (1) {

		char ch = 0;
		n = read (s, &ch, sizeof ch);
		if (n < 0 || ch < 0) 
			continue;
		if (ch == 0 && last_was_cr)
			ch = '\n';
		last_was_cr = (ch == '\r');
		// Parse user input for telnet options
		parse(&ts, &ch);
		if (ch != '\r')
			cparser_feed (session, ch);
	}
}

void *telnetd (void *arg)
{
	int s;
	int rc;
	struct sockaddr_in sin;
	tmtaskid_t hthread;
	port = 23;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		return -1;
	}

	memset (&sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = (INADDR_ANY);
	sin.sin_port = htons (port);
	sin.sin_len = sizeof (sin);

	rc = bind(sock, (struct sockaddr *) &sin, sizeof sin);

	if (rc < 0)
	{
		close (sock);
		return 1;
	}

	rc = listen(sock, 5);
	if (rc < 0)
	{
		close (sock);
		return 1;
	}

	state = RUNNING;

	while (1)
	{
		struct sockaddr_in sin;
		int socklen = 0;

		s = lwip_accept (sock, (struct sockaddr *) &sin, &socklen);

		if (s < 0)
		{
			continue;
		}

		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *) &off, sizeof(off));

		if (task_create ("telent", 30, 3, 48 * 1024, telnet_task, NULL, (void *)s, 
					&hthread) == TSK_FAILURE) {
			printf ("Task creation failed : %s\n", "telnet");
		}
	}
}

#else 
/*
 * Sean Middleditch
 * sean@sourcemud.org
 *
 * The author or authors of this code dedicate any and all copyright interest
 * in this code to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and successors. We
 * intend this dedication to be an overt act of relinquishment in perpetuity of
 * all present and future rights to this code under copyright law. 
 */

#if !defined(_WIN32)
#	if !defined(_POSIX_SOURCE)
#		define _POSIX_SOURCE
#	endif
#	if !defined(_BSD_SOURCE)
#		define _BSD_SOURCE
#	endif

#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <poll.h>
#	include <unistd.h>
#else
#	include <winsock2.h>
#	include <ws2tcpip.h>

#	define snprintf _snprintf
#	define poll WSAPoll
#	define close closesocket
#	define strdup _strdup
#	define ECONNRESET WSAECONNRESET
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "libtelnet.h"

#define MAX_USERS 64
#define LINEBUFFER_SIZE 256

static const telnet_telopt_t telopts[] = {
	{ TELNET_TELOPT_COMPRESS2,	TELNET_WILL, TELNET_DONT },
	{ -1, 0, 0 }
};

struct telnet_session_t {
	int sock;
	telnet_t *telnet;
	char linebuf[256];
	int linepos;
};


static void linebuffer_push(char *buffer, size_t size, int *linepos,
		char ch, void (*cb)(const char *line, int overflow, void *ud),
		void *ud) {

	/* CRLF -- line terminator */
	if (ch == '\n' && *linepos > 0 && buffer[*linepos - 1] == '\r') {
		/* NUL terminate (replaces \r in buffer), notify app, clear */
		buffer[*linepos - 1] = 0;
		cb(buffer, 0, ud);
		*linepos = 0;

	/* CRNUL -- just a CR */
	} else if (ch == 0 && *linepos > 0 && buffer[*linepos - 1] == '\r') {
		/* do nothing, the CR is already in the buffer */

	/* anything else (including technically invalid CR followed by
	 * anything besides LF or NUL -- just buffer if we have room
	 * \r
	 */
	} else if (*linepos != size) {
		buffer[(*linepos)++] = ch;

	/* buffer overflow */
	} else {
		/* terminate (NOTE: eats a byte), notify app, clear buffer */
		buffer[size - 1] = 0;
		cb(buffer, size - 1, ud);
		*linepos = 0;
	}
}

static void _send(int sock, const char *buffer, unsigned int size) {
	int rs;

	/* ignore on invalid socket */
	if (sock == -1)
		return;

	/* send data */
	while (size > 0) {
		if ((rs = send(sock, buffer, size, 0)) == -1) {
			if (errno != EINTR && errno != ECONNRESET) {
				fprintf(stderr, "send() failed: %s\n", strerror(errno));
				exit(1);
			} else {
				return;
			}
		} else if (rs == 0) {
			fprintf(stderr, "send() unexpectedly returned 0\n");
			exit(1);
		}

		/* update pointer and size to see if we've got more to send */
		buffer += rs;
		size -= rs;
	}
}

/* process input line */
static void _online(const char *line, int overflow, void *ud) {
	struct telnet_session_t *user = (struct telnet_session_t*)ud;
	int i;
	/* if line is "quit" then, well, quit */
	if (strcmp(line, "quit") == 0) {
		close(user->sock);
		free(user);
		return;
	}
}

static void _input(struct telnet_session_t *user, const char *buffer,
		unsigned int size) {
	unsigned int i;
	for (i = 0; i != size; ++i) {
			if (buffer[i] != '\r')
			cparser_feed (1, (char)buffer[i]);
	}
}

static void _event_handler(telnet_t *telnet, telnet_event_t *ev,
		void *user_data) {
	struct telnet_session_t *user = (struct telnet_session_t*)user_data;

	switch (ev->type) {
	/* data received */
	case TELNET_EV_DATA:
		_input(user, ev->data.buffer, ev->data.size);
		break;
	/* data must be sent */
	case TELNET_EV_SEND:
		_send(user->sock, ev->data.buffer, ev->data.size);
		break;
	/* enable compress2 if accepted by client */
	case TELNET_EV_DO:
		if (ev->neg.telopt == TELNET_TELOPT_COMPRESS2)
			telnet_begin_compress2(telnet);
		break;
	/* error */
	case TELNET_EV_ERROR:
		close(user->sock);
		user->sock = -1;
		telnet_free(user->telnet);
		break;
	default:
		/* ignore */
		break;
	}
}

void telnet_task (void *arg)
{
	int s = (int) arg;
	int session = -1;
	telnet_t *telnet = NULL;
	struct telnet_session_t  *new = calloc (1, sizeof (struct telnet_session_t));
	char buffer[512];
	int rs = 0;

	telnet = telnet_client_init (telopts, _event_handler, 0, new);

	new->telnet = telnet;
	new->sock = s;
	if ((session = cli_telnet_session_init ("telent@OpenSwitch",  s, telnet)) < 0)
		return -1;

	telnet_negotiate(telnet, TELNET_WILL, TELNET_TELOPT_COMPRESS2);

	cli_start_session (session);

	telnet_printf(telnet, "telnet@OpenSwitch# ");

	while (1) {
		memset (buffer, 0, sizeof(buffer));
		if ((rs = recv(s, buffer, sizeof(buffer), 0)) > 0) {
			telnet_recv(telnet, buffer, s);
		} else if (rs == 0) {
			close(s);
			telnet_free(telnet);
			break;
		}
	}
}


void * telnetd (void *arg) 
{
	short listen_port;
	int listen_sock;
	int rs;
	int i;
	struct sockaddr_in addr;
	socklen_t addrlen;
	int  hthread = -1;

	listen_port = 23;

	/* create listening socket */
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "socket() failed: %s\n", strerror(errno));
		return 1;
	}

	/* reuse address option */
	rs = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&rs, sizeof(rs));

	/* bind to listening addr/port */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(listen_port);
	if (bind(listen_sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		fprintf(stderr, "bind() failed: %s\n", strerror(errno));
		return 1;
	}

	/* listen for clients */
	if (listen(listen_sock, 5) == -1) {
		fprintf(stderr, "listen() failed: %s\n", strerror(errno));
		return 1;
	}

	/* loop for ever */
	for (;;) {

		addrlen = sizeof(addr);

		if ((rs = accept(listen_sock, (struct sockaddr *)&addr,
						&addrlen)) == -1) {
			fprintf(stderr, "accept() failed: %s\n", strerror(errno));
			return 1;
		}

		if (task_create ("telent", 30, 3, 48 * 1024, telnet_task, NULL, (void *)rs, 
					&hthread) == 0) {
			printf ("Task creation failed : %s\n", "telnet");
		}
	}
	/* not that we can reach this, but GCC will cry if it's not here */
	return 0;
}

#endif
int telnet_init (void)
{
	int  hthread = -1;
	if (task_create ("telentd", 30, 3, 20 * 1024, telnetd, NULL, NULL, 
				&hthread) == 0) {
		printf ("Task creation failed : %s\n", "telnetd");
		return -1;
	}
}
