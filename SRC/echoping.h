/* $Id$ */

/* Settings you should not change -- see below for changeable ones */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Settings you can change */

#define DEFLINE 256
#define UDPMAX 65535
/* Mostly for HTTP */
#define MAXTOREAD 150000
#ifdef SMTP
#define MAXSMTP 1024
#define MAXSMTPLINES 30
#endif

/* Probably too many inclusions but this is to keep 'gcc -Wall' happy... */
#include	<stdio.h>
#include        <stdlib.h>
#include	<sys/types.h>
#include	<netdb.h>
#include	<sys/socket.h>
#include        <netinet/tcp.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include        <stdarg.h>
#include        <sys/time.h>
#include        <errno.h>
#include        <unistd.h>
#include        <string.h>
#include        <signal.h>
#include        <math.h>
#include        <dlfcn.h>

/* popt library */
#include        <popt.h>

#ifdef OPENSSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#endif /* OpenSSL */

#ifdef GNUTLS
#include <gnutls/gnutls.h>
#endif

#ifdef LIBIDN
#include <stringprep.h>		/* stringprep_locale_to_utf8() */
#include <idna.h>		/* idna_to_ascii_from_utf8() */
#endif

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

#ifndef SOL_IP
#define SOL_IP (getprotobyname("ip")->p_proto)
#endif

/* These entities should be in errno.h but some systems do not define
   them. */
#ifdef DECL_SYS_NERR
extern int sys_nerr;
#endif

/* If we have it, use it */
#ifdef HAVE_SIGACTION
#define USE_SIGACTION 1
#endif
#ifdef HAVE_TOS
#define USE_TOS 1
#endif
#ifdef HAVE_SOCKET_PRIORITY
#define USE_PRIORITY 1
#endif


#ifndef HEADER_INCLUDED
typedef union _CHANNEL
{
  FILE *fs;
#ifdef OPENSSL
  SSL *ssl;
#endif
#ifdef GNUTLS
  gnutls_session tls;
#endif
}
CHANNEL;

/* Do not use "short" for "boolean" because popt does not know this
   type. On a little-endian machine without alignment issues, it may
   work but not, for instance, on UltraSparc platforms. See for
   instance Debian bug #254322. */
typedef unsigned int boolean;

struct result
{
  boolean valid;
  struct timeval timevalue;
};

boolean timeout_flag;
struct echoping_struct
{
  boolean udp;			/* Use the UDP protocol (TCP is the default) */
  boolean ttcp;
  boolean only_ipv4;
  boolean only_ipv6;
  boolean verbose;
};
typedef struct echoping_struct echoping_options;
#ifndef IN_PLUGIN
/* The functions we will find in the plugin */
/* Initializes the plugin with its arguments. Returns the port name or number or NULL if the plugin wants to use the raw interface. */
typedef char *(*init_f) (const int argc, const char **argv,
			 const echoping_options global_options);
init_f plugin_init;
typedef void (*start_f) (struct addrinfo *);
start_f plugin_start;
typedef void (*start_raw_f) ();
start_raw_f plugin_raw_start;
typedef int (*execute_f) ();
execute_f plugin_execute;
typedef void (*terminate_f) ();
terminate_f plugin_terminate;
#endif

#endif

struct timeval null_timeval;
struct timeval max_timeval;

#define	ECHO_TCP_PORT	"echo"
#define	DISCARD_TCP_PORT	"discard"
#define	CHARACTER_GENERATOR_TCP_PORT	"chargen"
#define DEFAULT_HTTP_TCP_PORT "http"
#define DEFAULT_HTTPS_TCP_PORT "https"
#define DEFAULT_ICP_UDP_PORT "icp"

#ifdef HTTP
/* Use the old HTTP 1.0 protocol? If yes, set HTTP10 to 1*/
#undef HTTP10
#endif

#define	USE_ECHO	1
#define	USE_DISCARD	2
#define	USE_CHARGEN	3
#define	USE_HTTP	4
#define USE_ICP		5
#define USE_SMTP        6

#define CHARGENERATED " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefg";

char *server;
#ifdef LIBIDN
char *locale_server, *ace_server, *utf8_server;
#endif

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
#ifdef GNUTLS
int TLS_readline ();
#endif
/* util.c */
char *random_string ();
void tvsub ();
void tvadd ();
void tvavg ();
void tvmin ();
void tvmax ();
int tvcmp ();
void tvstddev ();
void tvstddevavg ();
double tv2double ();
struct timeval double2tv ();
/* http.c */
#ifdef HTTP
char *make_http_sendline ();
void find_server_and_port ();
/* This one has prototypes, for a very subtile compiler issue. */
int read_from_server (CHANNEL fs, short ssl, boolean accept_redirects);
#endif

#ifdef ICP
#include "icp.h"
void *make_icp_sendline ();
int recv_icp ();
#ifndef HTTP
int read_from_server ();
#endif
#endif

#ifdef SMTP
int smtp_read_response_from_server ();
#endif

extern char *progname;

extern boolean timeout_flag;

#include "compilation.h"

#ifndef HEADER_INCLUDED
#define HEADER_INCLUDED
#endif
