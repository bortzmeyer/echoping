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
	char				request[REQUEST], reply[REPLY];
	int					sockfd, n;

	if (argc != 2)
		err_quit("usage: ttcpcli <IP address of server>");

	if ( (sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("socket error");

	memset(&serv, 0, sizeof(serv));
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
