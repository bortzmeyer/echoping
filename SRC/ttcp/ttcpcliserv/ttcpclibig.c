/*
 * Copyright (c) 1996 W. Richard Stevens.  All rights reserved.
 * Permission to use or modify this software and its documentation only for
 * educational purposes and without fee is hereby granted, provided that
 * the above copyright notice appear in all copies.  The author makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 */

#include	"cliserv.h"
#include	<netinet/tcp.h>

#undef	REQUEST
#undef	REPLY

#define	REQUEST	3300				/* max size of request, in bytes */
#define	REPLY	3400				/* max size of reply, in bytes */

int
main(int argc, char *argv[])
{
	struct sockaddr_in	serv;
	char				request[REQUEST], reply[REPLY];
	int					sockfd, n;

	if (argc != 2)
		err_quit("usage: ttcpcli <IP address of server>");

	if ( (sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("socket error");

	n = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_DEBUG,
									(char *) &n, sizeof(n)) < 0)
		err_sys("SO_DEBUG error");

#ifdef	notdef
	n = 1;
	if (setsockopt(sockfd, IPPROTO_TCP, TCP_NOPUSH,
									(char *) &n, sizeof(n)) < 0)
		err_sys("TCP_NOPUSH error");
#endif

	memset(&serv, sizeof(serv), 0);
	serv.sin_family		 = AF_INET;
	serv.sin_addr.s_addr = inet_addr(argv[1]);
	serv.sin_port		 = htons(TCP_SERV_PORT);

	/* form request[] ... */

	if (sendto(sockfd, request, REQUEST, MSG_EOF,
			   (SA) &serv, sizeof(serv)) != REQUEST)
		err_sys("sendto error");

	if ( (n = read_stream(sockfd, reply, REPLY)) < 0)
		err_sys("read error");

	/* process "n" bytes of reply[] ... */

	exit(0);
}
