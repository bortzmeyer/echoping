/*
 * Definitions for TCP and UDP client/server programs. 
 */

#define VERSION "1.3.0-beta-T/TCP"

#include	<stdio.h>
#include	<sys/types.h>
#include	<netdb.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include        <varargs.h>
#include        <sys/time.h>
#include        <errno.h>
#include        <unistd.h>

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef SIGALRM  /* Linux Slackware... */
#define SIGALRM   14    /* alarm clock timeout */
#endif
#ifndef SIGINT  /* Linux Slackware... */
#define SIGINT   2    /* interrupt, generated from terminal special char */
#endif

#ifndef INADDR_NONE
#define       INADDR_NONE (-1)
#endif

struct timeval  null_timeval;
struct timeval  max_timeval;

#define	ECHO_TCP_PORT	"echo"
#define	DISCARD_TCP_PORT	"discard"
#define	CHARACTER_GENERATOR_TCP_PORT	"chargen"

#define	USE_ECHO	1
#define	USE_DISCARD	2
#define	USE_CHARGEN	3

#define CHARGENERATED " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefg";

char           *server;

char           *sys_err_str();

char           *random_string();

double          tv2double();

#define DEFLINE 256
#define MAXLINE 1500
#define MAXNUMBER 20

extern char    *progname;
