/*
 * Read a line from a descriptor with fgets 
 *
 * $Id$
 *
 */

#include "echoping.h"

int
readline(fs, ptr, maxlen, ln)
    FILE           *fs;
    char           *ptr;
    int             maxlen;
    unsigned short  ln;
{
    int             n = 1;
    char           *rc;
    int             r;

    /* Reading with fgets or fread instead of read one-character-at-a-time is more
     * than ten times faster, on a local server. */
    if (ln) {
        rc = fgets(ptr, maxlen + 1, fs);
        if (rc == NULL) {
            if (timeout_flag)
                return n;
            return (-1);
        }
        n = strlen(rc);
        return n;
    } else {
        while (n < maxlen) {
            r = fread(ptr, 1, maxlen, fs);
            if (timeout_flag)
                return r;
            if (r == 0) {
                if (n == 1)
                    return (0); /* EOF, no data read */
                else
                    break;      /* EOF, some data was read */
            }
            n = n + r;
        }
    }
    return (n);
}

#ifdef OPENSSL

char            SSL_buffer[MAXTOREAD];
int             buf_ptr;
int             buf_end;

int
SSL_readline(sslh, ptr, maxlen, ln)
    SSL            *sslh;
    char           *ptr;
    int             maxlen;
    unsigned short  ln;
{
    int             rc = 0;
    int             n = 0;
    int             i, oi;
    if (ln) {
        /* Empty buffer */
        if (buf_end == 0) {
            rc = SSL_read(sslh, SSL_buffer, maxlen);
            if (rc == -1)
                return rc;
            buf_end = rc;
            buf_ptr = 0;
        }
        /* No more data in the buffer */
        else if (buf_ptr == buf_end) {
            buf_ptr = 0;
            rc = SSL_read(sslh, SSL_buffer, maxlen);
            if (rc == -1)
                return rc;
            buf_end = rc;
        } else if (SSL_buffer[buf_end] != '\n') {
            /* We have a probleme here is the first SSL_read sent back a text not
             * finished by a \n. See www.SSL.de for an example. We get more data.
             * See bug #230384 */
            rc = SSL_read(sslh, SSL_buffer + buf_end, maxlen);
            if (rc == -1)
                return rc;
            buf_end = buf_end + rc;
        }
        for (oi = buf_ptr, i = buf_ptr; i <= buf_end && SSL_buffer[i] != '\n'; i++) {
            *ptr++ = SSL_buffer[i];
            buf_ptr++;
        }
        if (SSL_buffer[i] == '\n')
            buf_ptr++;
        *ptr = '\0';
        /* if (ln) printf ("SSL_readline returns %d (%s)\n", i - oi, SSL_buffer); */
        return (i - oi);
    } else {
        /* OpenSSL reads at most 4096 characters */
        rc = 1;                 /* Just to avoid exiting too soon */
        while (n < maxlen && rc != 0) {
            if ((buf_end == 0) || (buf_ptr > buf_end)) {
                rc = SSL_read(sslh, ptr, maxlen);
                buf_end = 0;
                buf_ptr = 0;
            } else {
                for (i = buf_ptr; i < maxlen && i <= buf_end; i++) {
                    *ptr++ = SSL_buffer[i];
                    rc++;
                }
                buf_ptr = i;
            }
            n = n + rc;
        }
        return n;
    }
}
#endif

#ifdef GNUTLS

char            TLS_buffer[MAXTOREAD];
int             buf_ptr;
int             buf_end;

int
TLS_readline(session, ptr, maxlen, ln)
    gnutls_session  session;
    char           *ptr;
    int             maxlen;
    unsigned short  ln;
{
    int             rc = 0;
    int             n = 0;
    int             i, oi;
    if (ln) {
        /* Empty buffer */
        if (buf_end == 0) {
            rc = gnutls_record_recv(session, TLS_buffer, maxlen);
            if (rc == -1)
                return rc;
            buf_end = rc;
            buf_ptr = 0;
        }
        /* No more data in the buffer */
        else if (buf_ptr == buf_end) {
            buf_ptr = 0;
            rc = gnutls_record_recv(session, TLS_buffer, maxlen);
            if (rc == -1)
                return rc;
            buf_end = rc;
        } else if (TLS_buffer[buf_end] != '\n') {
            rc = gnutls_record_recv(session, TLS_buffer + buf_end, maxlen);
            if (rc == -1)
                return rc;
            buf_end = buf_end + rc;
        }
        for (oi = buf_ptr, i = buf_ptr; i <= buf_end && TLS_buffer[i] != '\n'; i++) {
            *ptr++ = TLS_buffer[i];
            buf_ptr++;
        }
        if (TLS_buffer[i] == '\n')
            buf_ptr++;
        *ptr = '\0';
        /* printf ("DEBUG: TLS_readline returns %d (%s)\n", i - oi, TLS_buffer); */
        return (i - oi);
    } else {
        rc = 1;                 /* Just to avoid exiting too soon */
        while (n < maxlen && rc > 0) {
            if ((buf_end == 0) || (buf_ptr > buf_end)) {
                rc = gnutls_record_recv(session, ptr, maxlen);
                buf_end = 0;
                buf_ptr = 0;
            } else {
                for (i = buf_ptr; i < maxlen && i <= buf_end; i++) {
                    *ptr++ = TLS_buffer[i];
                    rc++;
                    /* printf ("DEBUG: Now %d chars read\n", rc); */
                }
                buf_ptr = i;
            }
            if (rc > 0)
                n = n + rc;
            /* printf ("DEBUG: Now %d chars to send\n", n); */
        }
        return n;
    }
}
#endif
