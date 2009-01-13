/* Code contributed by Samuel Tardieu <sam@inf.enst.fr>
 *
 * $Id$
 *
 */

#include "echoping.h"

#ifdef SMTP

char            big_recvline[MAXTOREAD];

int
smtp_read_response_from_server(FILE * fs)
{
    int             nr;
    int             i;

    for (i = 0; i < MAXSMTPLINES; i++) {
        nr = readline(fs, big_recvline, MAXTOREAD, TRUE);
        if (nr <= 4) {
            return -1;
        }
        if (big_recvline[3] == ' ') {
            return nr;
        }
        if (big_recvline[3] != '-') {
            return -1;
        }
    }
    return -1;
}

#endif                          /* SMTP */
