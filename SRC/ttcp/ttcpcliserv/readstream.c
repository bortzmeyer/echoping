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
read_stream(int fd, char *ptr, int maxbytes)
{
	int		nleft, nread;

	nleft = maxbytes;
	while (nleft > 0) {
		if ( (nread = read(fd, ptr, nleft)) < 0)
			return(nread);		/* error, return < 0 */
		else if (nread == 0)
			break;				/* EOF, return #bytes read */
		nleft -= nread;
		ptr   += nread;
	}
	return(maxbytes - nleft);		/* return >= 0 */
}
