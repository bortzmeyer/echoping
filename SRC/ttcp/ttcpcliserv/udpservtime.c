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
	int					sockfd, n, clilen, spt;

	if (argc > 1)
		spt = atoi(argv[1]);	/* optional server processing time, in ms */
	else
		spt = 0;

	if ( (sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		err_sys("socket error");

	memset(&serv, sizeof(serv), 0);
	serv.sin_family      = AF_INET;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port        = htons(UDP_SERV_PORT);

	if (bind(sockfd, (SA) &serv, sizeof(serv)) < 0)
		err_sys("bind error");

	for ( ; ; ) {
		clilen = sizeof(cli);
		if ( (n = recvfrom(sockfd, request, sizeof(request), 0,
						   (SA) &cli, &clilen)) < 0)
			err_sys("recvfrom error");

		/* process "n" bytes of request[] ... */
		if (spt)
			sleep_us(spt * 1000);

		if (sendto(sockfd, reply, n, 0,
				   (SA) &cli, sizeof(cli)) != n)
			err_sys("sendto error");
	}
}
