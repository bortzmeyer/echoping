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
	struct sockaddr_in	serv;
	char				request[5000], reply[5000];
	int					sockfd, n, len, i;
	double				rtt, minrtt;

	if (argc != 2)
		err_quit("usage: ttcpcli <IP address of server>");

	memset(&serv, sizeof(serv), 0);
	serv.sin_family		 = AF_INET;
	serv.sin_addr.s_addr = inet_addr(argv[1]);
	serv.sin_port		 = htons(TTCP_SERV_PORT);

	for (len = 0; len <= 1400; len += 100) {
		printf("len = %d\n", len);
		minrtt = 9999999;
		for (i = 0; i < 30; i++) {
			if ( (sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
				err_sys("socket error");

			/* form request[] ... */

			start_timer();
			if (sendto(sockfd, request, len, MSG_EOF,
			   		(SA) &serv, sizeof(serv)) != len)
				err_sys("sendto error");

			if ( (n = read_stream(sockfd, reply, len)) < 0)
				err_sys("read error");
			rtt = print_timer();
			minrtt = min(minrtt, rtt);

			/* process "n" bytes of reply[] ... */
			close(sockfd);
		}
		printf("min rtt = %9.3f\n\n", minrtt);
	}

	exit(0);
}
