/*
 * Copyright (c) 1996 W. Richard Stevens.  All rights reserved.
 * Permission to use or modify this software and its documentation only for
 * educational purposes and without fee is hereby granted, provided that
 * the above copyright notice appear in all copies.  The author makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 */

#include	"cliserv.h"
#include	<sys/time.h>

void	tvsub(struct timeval *, struct timeval *);

struct timeval	tvstart, tvend;

void
start_timer()
{
	if (gettimeofday(&tvstart, NULL) < 0)
		err_sys("start_timer: gettimeofday error");
}

double
print_timer()
{
	double	triptime;

	if (gettimeofday(&tvend, NULL) < 0)
		err_sys("print_timer: gettimeofday error");

	tvsub(&tvend, &tvstart);
	triptime = ((double) tvend.tv_sec) * 1000.0 +
			   ((double) tvend.tv_usec) / 1000.0;		/* in millisec */
	printf("rtt = %9.3f ms\n", triptime);
	return(triptime);
}

/* Subtract two timeval structures: *end -= *start */

void
tvsub(struct timeval *end, struct timeval *start)
{
	if ((end->tv_usec -= start->tv_usec) < 0) {
		--end->tv_sec;
		end->tv_usec += 1000000;
	}
	end->tv_sec -= start->tv_sec;
}
