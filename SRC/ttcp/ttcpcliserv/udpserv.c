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
main()					/* simple UDP server */
{
	struct sockaddr_in	serv, cli;
	char				request[REQUEST], reply[REPLY];
	int					sockfd, n, clilen;

	if ( (sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		err_sys("socket error");

	memset(&serv, 0, sizeof(serv));
	serv.sin_family      = AF_INET;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port        = htons(UDP_SERV_PORT);

	if (bind(sockfd, (SA) &serv, sizeof(serv)) < 0)
		err_sys("bind error");

	for ( ; ; ) {
		clilen = sizeof(cli);
		if ( (n = recvfrom(sockfd, request, REQUEST, 0,
						   (SA) &cli, &clilen)) < 0)
			err_sys("recvfrom error");

		/* process "n" bytes of request[] and create reply[] ... */

		if (sendto(sockfd, reply, REPLY, 0,
				   (SA) &cli, sizeof(cli)) != REPLY)
			err_sys("sendto error");
	}
}
