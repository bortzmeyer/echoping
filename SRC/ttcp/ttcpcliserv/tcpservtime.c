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
main(int argc, char *argv[])
{
	struct sockaddr_in	serv, cli;
	char				request[5000], reply[5000];
	int					listenfd, sockfd, n, clilen, spt;

	if (argc > 1)
		spt = atoi(argv[1]);	/* optional server processing time, in ms */
	else
		spt = 0;

	if ( (listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("socket error");

	memset(&serv, sizeof(serv), 0);
	serv.sin_family      = AF_INET;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port        = htons(TCP_SERV_PORT);

	if (bind(listenfd, (SA) &serv, sizeof(serv)) < 0)
		err_sys("bind error");

	if (listen(listenfd, SOMAXCONN) < 0)
		err_sys("listen error");

	for ( ; ; ) {
		clilen = sizeof(cli);
		if ( (sockfd = accept(listenfd, (SA) &cli, &clilen)) < 0)
			err_sys("accept error");

		if ( (n = read_stream(sockfd, request, sizeof(request))) < 0)
			err_sys("read error");

		/* process "n" bytes of request[] ... */
		if (spt)
			sleep_us(spt * 1000);

		if (write(sockfd, reply, n) != n)
			err_sys("write error");

		close(sockfd);
	}
}
