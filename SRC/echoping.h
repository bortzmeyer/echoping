/* $Id$ */

/* Settings you should not change -- see below for changeable ones */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Settings you can change */

#define DEFLINE 256
#define MAXLINE 65535
#define UDPMAX 65535
#ifdef HTTP
#define MAXTOREAD 150000
#endif
#ifdef SMTP
#define MAXSMTP 1024
#define MAXSMTPLINES 30
#endif
#define MAXNUMBER 20

/* Probably too many inclusions but this is to keep 'gcc -Wall' happy... */
#include	<stdio.h>
#include        <stdlib.h>
#include	<sys/types.h>
#include	<netdb.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include        <stdarg.h>
#include        <sys/time.h>
#include        <errno.h>
#include        <unistd.h>
#include        <string.h>
#include        <signal.h>

#ifdef OPENSSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#endif /* OpenSSL */

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef SIGALRM			/* Linux... */
#define SIGALRM   14		/* alarm clock timeout */
#endif
#ifndef SIGINT			/* Linux... */
#define SIGINT   2		/* interrupt, generated from terminal special char */
#endif

#ifndef INADDR_NONE		/* SunOS */
#define       INADDR_NONE (-1)
#endif

/* These entities should be in errno.h but some systems do not define
   them. */
#ifdef DECL_SYS_ERRLIST
extern char *sys_errlist[];
#endif
#ifdef DECL_SYS_NERR
extern int sys_nerr;
#endif

/* If we have it, use it */
#ifdef HAVE_SIGACTION
#define USE_SIGACTION 1
#endif
#ifdef HAVE_TTCP
#define USE_TTCP 1
#endif
#ifdef HAVE_TOS
#define USE_TOS 1
#endif


#ifndef HEADER_INCLUDED
typedef union _CHANNEL
{
  FILE *fs;
#ifdef OPENSSL
  SSL *ssl;
#endif
}
CHANNEL;
#endif


struct timeval null_timeval;
struct timeval max_timeval;

#define	ECHO_TCP_PORT	"echo"
#define	DISCARD_TCP_PORT	"discard"
#define	CHARACTER_GENERATOR_TCP_PORT	"chargen"
#define DEFAULT_HTTP_TCP_PORT "HTTP"
#define DEFAULT_HTTPS_TCP_PORT "HTTPS"
#define DEFAULT_ICP_UDP_PORT "ICP"

#define	USE_ECHO	1
#define	USE_DISCARD	2
#define	USE_CHARGEN	3
#define	USE_HTTP	4
#define USE_ICP		5
#define USE_SMTP        6

#define CHARGENERATED " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefg";

char *server;

/* My functions */

/* error.c */
void usage ();
void err_sys (char *str, ...);
void err_ret (char *str, ...);
void err_quit (char *str, ...);
char *sys_err_str ();
/* writen.c */
int writen ();
/* readline.c */
int readline ();
#ifdef OPENSSL
int SSL_readline ();
#endif
/* util.c */
char *random_string ();
void tvsub ();
void tvadd ();
void tvavg ();
void tvmin ();
void tvmax ();
double tv2double ();
/* http.c */
#ifdef HTTP
char *make_http_sendline ();
void find_server_and_port ();
/* This one has prototypes, for a very subtile compiler issue. */
int read_from_server (CHANNEL fs, short ssl);
#endif

#ifdef ICP
#include "icp.h"
void *make_icp_sendline ();
int recv_icp ();
#ifndef HTTP
void find_server_and_port ();
int read_from_server ();
#endif
#endif

#ifdef SMTP
int smtp_read_response_from_server ();
#endif

extern char *progname;

extern unsigned short timeout_flag;

#ifndef HEADER_INCLUDED
#define HEADER_INCLUDED
#endif
