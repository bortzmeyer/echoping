/*
 * Write "n" bytes to a descriptor. Use in place of write() when fd is a
 * stream socket. 
 */

/* Stolen from Stevens' book */

#include "echoping.h"

int
writen(fd, ptr, nbytes)
    register int    fd;
    register char  *ptr;
    register int    nbytes;
{
    int             nleft, nwritten;

    nleft = nbytes;
    while (nleft > 0) {
        nwritten = write(fd, ptr, nleft);
        if (nwritten <= 0)
            return (nwritten);  /* error */
        /* Some systems, such as Digital's OSF1 (Digital Unix) doesn't set the
         * returned value to -1, even when interrupted by an alarm, whatever says
         * the documentation. errno is not set. */
        if ((nwritten < nleft) && timeout_flag)
            return nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return (nbytes - nleft);
}
