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
#include "lwip/netdb.h"
#include <termios.h>


void * telnet2_task (void *arg);
int telnet_to (char *host); 

static telnet_t *telnet;
static int do_echo;

static const telnet_telopt_t telopts[] = {
	{ TELNET_TELOPT_ECHO, TELNET_WONT, TELNET_DO },
	{ TELNET_TELOPT_TTYPE, TELNET_WILL, TELNET_DONT },
	{ TELNET_TELOPT_COMPRESS2, TELNET_WONT, TELNET_DO },
	{ TELNET_TELOPT_MSSP, TELNET_WONT, TELNET_DO },
	{ -1, 0, 0 }
};

#if 0
static struct termios orig_tios;
static void _cleanup(void) {
	tcsetattr(STDOUT_FILENO, TCSADRAIN, &orig_tios);
}
#endif

static void _input(char *buffer, int size) {
	static char crlf[] = { '\r', '\n' };
	int i;

	for (i = 0; i != size; ++i) {
		/* if we got a CR or LF, replace with CRLF
		 * NOTE that usually you'd get a CR in UNIX, but in raw
		 * mode we get LF instead (not sure why)
		 */
		if (buffer[i] == '\r' || buffer[i] == '\n') {
			telnet_send(telnet, crlf, 2);
		} else {
			telnet_send(telnet, buffer + i, 1);
		}
	}
	fflush(stdout);
}

static void _send(int sock, const char *buffer, size_t size) {
	int rs;

	/* send data */
	while (size > 0) {
		if ((rs = send(sock, buffer, size, 0)) == -1) {
		//	fprintf(stderr, "send() failed: %s\n", strerror(errno));
			//exit(1);
		} else if (rs == 0) {
			fprintf(stderr, "send() unexpectedly returned 0\n");
			//exit(1);
		}

		/* update pointer and size to see if we've got more to send */
		buffer += rs;
		size -= rs;
	}
}

static void _event_handler(telnet_t *tnet, telnet_event_t *ev,
		void *user_data) {
	int sock = *(int*)user_data;

	switch (ev->type) {
		/* data received */
		case TELNET_EV_DATA:
			printf("%.*s", (int)ev->data.size, ev->data.buffer);
			fflush(stdout);
			break;
			/* data must be sent */
		case TELNET_EV_SEND:
			_send(sock, ev->data.buffer, ev->data.size);
			break;
			/* request to enable remote feature (or receipt) */
		case TELNET_EV_WILL:
			/* we'll agree to turn off our echo if server wants us to stop */
			if (ev->neg.telopt == TELNET_TELOPT_ECHO)
				do_echo = 0;
			break;
			/* notification of disabling remote feature (or receipt) */
		case TELNET_EV_WONT:
			if (ev->neg.telopt == TELNET_TELOPT_ECHO)
				do_echo = 1;
			break;
			/* request to enable local feature (or receipt) */
		case TELNET_EV_DO:
			break;
			/* demand to disable local feature (or receipt) */
		case TELNET_EV_DONT:
			break;
			/* respond to TTYPE commands */
		case TELNET_EV_TTYPE:
			/* respond with our terminal type, if requested */
			if (ev->ttype.cmd == TELNET_TTYPE_SEND) {
				telnet_ttype_is(tnet, getenv("TERM"));
			}
			break;
			/* respond to particular subnegotiations */
		case TELNET_EV_SUBNEGOTIATION:
			break;
			/* error */
		case TELNET_EV_ERROR:
			fprintf(stderr, "ERROR: %s\n", ev->error.msg);
			exit(1);
		default:
			/* ignore */
			break;
	}
}

void * telnet2_task (void *arg)
{
	int sock = (int)arg;
	int rs;
	char buffer[512];
	while (1) {
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);

		if (select(sock + 1, &rfds, NULL, NULL, NULL) < 0)
			continue;
		if (FD_ISSET (sock, &rfds)) {
			if ((rs = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
				telnet_recv(telnet, buffer, rs);
			}
		}
	}
}

int telnet_to (char *host) 
{
	int rs;
	int sock;
	struct addrinfo *ai;
	struct addrinfo hints;
#if 0
	struct termios tios;
#endif
	tmtaskid_t hthread = -1;
	char buffer[512];

	/* look up server host */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((rs = getaddrinfo(host, "23", &hints, &ai)) != 0) {
		fprintf(stderr, "getaddrinfo() failed for %s: %s\n", host,
				strerror(rs));
		return 1;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "socket() failed: %s\n", strerror(errno));
		return 1;
	}

	if (connect(sock, ai->ai_addr, ai->ai_addrlen) == -1) {
		fprintf(stderr, "server() failed: %s\n", strerror(errno));
		return 1;
	}

	/* free address lookup info */
	freeaddrinfo(ai);

	/* get current terminal settings, set raw mode, make sure we
	 * register atexit handler to restore terminal settings
	 */
#if 0
	tcgetattr(STDOUT_FILENO, &orig_tios);
	atexit(_cleanup);
	tios = orig_tios;
	cfmakeraw(&tios);
	tcsetattr(STDOUT_FILENO, TCSADRAIN, &tios);
#endif
	/* set input echoing on by default */
	do_echo = 0;

	/* initialize telnet box */
	telnet = telnet_client_init (telopts, _event_handler, 0, &sock);

	if (!task_create ("telnet2", 30, 3, 32 * 1024, telnet2_task, NULL, (void *)sock, 
				&hthread)) {
		printf ("Task creation failed : %s\n", "telnet");
		return -1;
	}


	/* loop while both connections are open */
	while (1) {
#undef read
		ssize_t read(int fd, void *buf, size_t count);
		if ((rs = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) 
			_input(buffer, rs);
	}

	/* clean up */
	tsk_cancel (hthread);
	telnet_free(telnet);
	close(sock);

	return 0;
}
