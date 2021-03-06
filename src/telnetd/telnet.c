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

#include "common_types.h"
#include "libtelnet.h"
#include "lwip/sockets.h"
#include "socks.h"

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
void cparser_feed (int session, int ch);
static void  * telnet_task (void *arg);
int cli_telnet_session_init (const char *prmt, int fd, void *data);
int cli_start_session (int session);
int telnet_init (void);
static void * telnetd (void *arg) ;

static void _send(int sock, const char *buffer, unsigned int size) {
	int rs;

	/* ignore on invalid socket */
	if (sock == -1)
		return;

	/* send data */
	while (size > 0) {
		if ((rs = send(sock, buffer, size, 0)) == -1) {
			if (errno != EINTR && errno != ECONNRESET) {
			//	fprintf(stderr, "send() failed: %s\n", strerror(errno));
				//exit(1);
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
#if 0
static void _online(const char *line, int overflow, void *ud) {
	struct telnet_session_t *user = (struct telnet_session_t*)ud;
	/* if line is "quit" then, well, quit */
	if (strcmp(line, "quit") == 0) {
		close(user->sock);
		free(user);
		return;
	}
}
#endif
static void _input(struct telnet_session_t *user, const char *buffer,
		unsigned int size) {
	unsigned int i;
	user = user;
	for (i = 0; i != size; ++i) {
		if (buffer[i] != '\r' && (int)buffer[i] != 0)
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

static void  * telnet_task (void *arg)
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
		return NULL;

	telnet_negotiate(telnet, TELNET_WILL, TELNET_TELOPT_COMPRESS2);

//	write(s,"\377\375\042\377\373\001",6);

	cli_start_session (session);


	//telnet_printf(telnet, "telnet@OpenSwitch# ");

	while (1) {
		memset (buffer, 0, sizeof(buffer));
		if ((rs = recv(s, buffer, 1, 0)) > 0) {
			telnet_recv(telnet, buffer, rs);
		} else if (rs == 0) {
#if 0
			close(s);
			telnet_free(telnet);
			break;
#endif
		}
	}
	return NULL;
}


static void * telnetd (void *arg UNUSED_PARAM) 
{
	short listen_port;
	int listen_sock;
	int rs;
	struct sockaddr_in addr;
	socklen_t addrlen;
	tmtaskid_t  hthread = -1;

	listen_port = 23;

	/* create listening socket */
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "socket() failed\n");
		return NULL;;
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
		fprintf(stderr, "bind() failed\n");
		return NULL;
	}

	/* listen for clients */
	if (listen(listen_sock, 5) == -1) {
		fprintf(stderr, "listen() failed\n");
		return NULL;
	}

	/* loop for ever */
	for (;;) {

		addrlen = sizeof(addr);

		if ((rs = accept(listen_sock, (struct sockaddr *)&addr,
						&addrlen)) == -1) {
			fprintf(stderr, "accept() failed\n");
			return NULL;
		}

		if (task_create ("telnet", 30, 3, 48 * 1024, telnet_task, NULL, (void *)rs, 
					&hthread) == 0) {
			printf ("Task creation failed : %s\n", "telnet");
		}
	}
	/* not that we can reach this, but GCC will cry if it's not here */
	return NULL;
}

int telnet_init (void)
{
	tmtaskid_t  hthread = -1;
	if (task_create ("telnetd", 30, 3, 16 * 1024, telnetd, NULL, NULL, 
				&hthread) == 0) {
		printf ("Task creation failed : %s\n", "telnetd");
		return -1;
	}
	return 0;
}
