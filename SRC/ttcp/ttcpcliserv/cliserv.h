/* Common includes and defines for UDP, TCP, and T/TCP
 * clients and servers */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>

#define	REQUEST	400				/* max size of request, in bytes */
#define	REPLY	400				/* max size of reply, in bytes */

#define	UDP_SERV_PORT	7777	/* UDP server's well-known port */
#define	TCP_SERV_PORT	8888	/* TCP server's well-known port */
#define	TTCP_SERV_PORT	9999	/* T/TCP server's well-known port */

/* Following shortens all the type casts of pointer arguments */
#define	SA	struct sockaddr *

void	err_quit(const char *, ...);
void	err_sys(const char *, ...);
int		read_stream(int, char *, int);

/* following for timing versions of client-server */
void	start_timer(void);
double	print_timer(void);
void	sleep_us(unsigned int);

#define	min(a,b)	((a) < (b) ? (a) : (b))
