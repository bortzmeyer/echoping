/*
 * Copyright (c) 1996 W. Richard Stevens.  All rights reserved.
 * Permission to use or modify this software and its documentation only for
 * educational purposes and without fee is hereby granted, provided that
 * the above copyright notice appear in all copies.  The author makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 */

#include	"cliserv.h"

int
main(int argc, char *argv[])		/* T/TCP client */
{
	struct sockaddr_in	serv;
	char				reply[REPLY];
	static char			request[] = "GET / HTTP/1.0\n\r\n\r";
	int					sockfd, n;

	if (argc != 2)
		err_quit("usage: ttcpcli <IP address of server>");

	if ( (sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("socket error");

	memset(&serv, sizeof(serv), 0);
	serv.sin_family		 = AF_INET;
	serv.sin_addr.s_addr = inet_addr(argv[1]);
	serv.sin_port		 = htons(80);

	/* form request[] ... */

	if (sendto(sockfd, request, sizeof(request)-1, MSG_EOF,
			   (SA) &serv, sizeof(serv)) != sizeof(request)-1)
		err_sys("sendto error");

	while ( (n = read_stream(sockfd, reply, REPLY)) > 0) {
		/* process "n" bytes of reply[] ... */
		write(1, reply, n);
	}
	if (n < 0)
		err_sys("read error");

	exit(0);
}
