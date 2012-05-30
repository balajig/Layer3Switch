/***********************************************************
 * Author : TechMinds
 *
 * GPLv2 licen
 ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <termio.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "list.h"
#include "cmd.h"

extern char read_input ();
int add_input_to_the_cmd(char c,char *tmp);
int write_input_on_screen(char c);
void write_string (char *str);

static char prmpt[MAX_PMP_LEN + 1]; 
static char hostname[MAX_PMP_LEN];
static struct termios oldt, newt;
int is_help = 0;
static int gfd = 1;
static int current_priv_level = PRIV_LEVEL5;
int clitskid = 0;
static void init_signals (void);
static void spawn_cli_thread (void);
static void *cmdinterface(void *unused);
static void init_tty_prompt (void);
void handle_segfault (int );

int create_cmdline_interface (char *prmt)
{
	/* init the tty promt properties*/
	init_tty_prompt ();
	/*signal handlers for SIGINT, SIGSEGV, SIGUSR1, SIGUSR2*/
	init_signals();
	/*XXX:set the tty prompt - we take control from shell 
	  don't forget to call reset_tty () once your job is done*/
	set_tty();

	set_prompt(prmt, strlen(prmt));

	/*finally kick-start the shell thread*/
	spawn_cli_thread ();
	return 0;
}

static void install_signal_handler (int signo, void (*handler)(int))
{
	signal (signo, handler);
}

static void init_signals (void)
{
	install_signal_handler (SIGSEGV, handle_segfault);
}

static void spawn_cli_thread (void)
{
	pthread_create (&clitskid, NULL, cmdinterface, NULL);
}

static void *cmdinterface(void *unused)
{
	char *cmd = NULL;
	int n = 0;

	print_prompt ();

	while(1) {
		int len = getline (&cmd,  &n, stdin);
		if (len > 0) {
			char c;
			write_string (cmd);
			fflush (stdout);
			sscanf (cmd, "%c", &c);

			if (c == 'v') {
			
			}
			else if (c == 'n') {

			}
			else if (c == 's') {
				show_vlinks ();
			} else {
				write_string ("\nInvaild Command\n");
				print_prompt ();
			}
		}
		if (cmd) {
			free(cmd);
			cmd = NULL;
		}
	}
	return;
}

static int init_tty_noncan_echo_disable (int fd, struct termios *terminal)
{
	/* following TTY settings for SERIAL-MODE COMMUNICATIONS*/

	if (ioctl (fd, TCGETA, terminal) == -1) {
		return -1;
	}
	terminal->c_lflag &= ~(ECHO);    /* disable echo */
	if (ioctl (fd, TCSETA, terminal) == -1) {
		return -1;
	}
	if (tcgetattr (fd, terminal) == -1) {
		return -1;
	}
	terminal->c_lflag &= ~(ICANON | ECHO);    /* Non-Canonical mode and 
						     Disable Echo */
	terminal->c_oflag |= OFDEL;    /* Fill Delete character */
	terminal->c_cc[VMIN] = 1;
	return 0;
}

static void init_tty_prompt (void)
{
	save_tty (&oldt);
	init_tty_noncan_echo_disable (gfd, &newt);
}


int set_tty ()
{
	if (tcsetattr (gfd, TCSANOW, &newt) == -1){
		return -1;
	}
	return 0;
}

int save_tty (struct termios *tty)
{
	if (tcgetattr (STDIN_FILENO, tty) == -1) {
		return -1;
	}
	return 0;   
}

int reset_tty(void)
{
	if (tcsetattr (STDIN_FILENO, TCSANOW, &oldt) == -1){
		return -1;
	}
	return 0;   
}

print_prompt ()
{
	write(gfd, prmpt, strlen(prmpt));
	fflush (stdout);
}

int write_input_on_screen(char c)
{
	write(gfd,&c,1);
}

void write_string (char *str)
{
        write (gfd, str, strlen(str));
	fflush (stdout);
}


void handle_segfault (int signo)
{
	write_string ("ooppppssssssss ....! System crashed ..."
		      "Am going to sleep for 100 secs debug the image\n");
	sleep (100);
	exit (0);
}

int set_prompt (char *pmt, int len)
{
       if (len > MAX_PMP_LEN || !pmt) {
               return -1;
       }
       sprintf (prmpt, "%s%s ", pmt, "#");
       return 0;
}

int  get_prompt (char *pmt)
{
       if (!pmt)
               return -1;
       memcpy (pmt, prmpt, MAX_PMP_LEN);
       return 0;
}
