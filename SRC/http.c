/* $Id$ */

#include "echoping.h"

#ifdef HTTP
#include "HTParse.h"


char            big_recvline[MAXTOREAD];

char           *
make_http_sendline(char *url, char *host, int port, int nocache)
{
    short           sport = (short) port;
    int             size = 350; /* Enough? RFC 2616, section 3.2.1 says 255 should
                                 * be enough, although there is no hard limit. We
                                 * reserve more because there * are the protocol
                                 * elements, the HTTP headers, etc */
    char           *sendline = (char *) malloc(size);
    char           *hostname = (char *) malloc(size);
    char           *cache_directive = "";
    int             result;
#ifdef HTTP10
    if (nocache)
        cache_directive = "Pragma: no-cache\r\n";       /* RFC 1945, "Hypertext
                                                         * Transfer Protocol * --
                                                         * HTTP/1.0" */
    result = snprintf(sendline, size,
                      "GET %s HTTP/1.0\r\nUser-Agent: Echoping/%s\r\n%s\r\n",
                      url, VERSION, cache_directive);
#else
    if (nocache) {
        if (nocache == 1)
            cache_directive = "Cache-control: max-age=0\r\n";   /* Simply force a
                                                                 * recheck with
                                                                 * the server */
        else
            cache_directive = "Cache-control: no-cache\r\n";    /* RFC 2616
                                                                 * "Hypertext
                                                                 * Transfer Protocol 
                                                                 * -- HTTP/1.1" */
    }
    strncpy(hostname, HTParse(url, "", PARSE_HOST), size);      /* See bug #1688940
                                                                 * to see why we use 
                                                                 * * * strNcpy. */
    hostname[size] = '\0';      /* Not added automatically */
    if (!strcmp(hostname, ""))
        snprintf(hostname, size, "%s:%d", host, sport);
    result = snprintf(sendline, size,
                      "GET %s HTTP/1.1\r\nUser-Agent: Echoping/%s\r\nHost: %s\r\nConnection: close\r\n%s\r\n",
                      url, VERSION, hostname, cache_directive);
    free(hostname);
#endif
    if (result >= size)
        err_quit("URL and/or hostname too long(s)");
    return sendline;
}

int
read_from_server(CHANNEL fs, short ssl, boolean accept_redirects)
{
    int             nr = 0;
    int             total = 0;
    int             reply_code;
    int             first_line = TRUE;
    short           body = FALSE;
#ifdef OPENSSL
    int             sslcode;
#endif
    while (!body && !timeout_flag) {
        if (!ssl)
            nr = readline(fs.fs, big_recvline, MAXTOREAD, TRUE);
#ifdef OPENSSL
        else {
            nr = SSL_readline(fs.ssl, big_recvline, MAXTOREAD, TRUE);
            if (nr == -1) {
                sslcode = ERR_get_error();
                err_ret("SSL_readline error: %s", ERR_error_string(sslcode, NULL));
            }
        }
#endif
#ifdef GNUTLS
        else
        {
            nr = TLS_readline(fs.tls, big_recvline, MAXTOREAD, TRUE);
            if (nr == -1) {
                err_ret("TLS_readline error: %s", gnutls_strerror(nr));
            }
        }
#endif
        /* 
         * printf ("DEBUG: reading \"%s\"\n (%d chars)\n",
         * big_recvline, nr);
         */
        /* 
         * HTTP replies should be separated by CR-LF. Unfortunately,
         * some servers send only CR :-(
         */
        body = ((nr == 2) || (nr == 1));        /* Empty line CR-LF seen */
        if ((nr < 1) && (timeout_flag)) /* Probably a timeout */
            return -1;
        if (nr < 1)
            /* SourceForge bug #109385 */
            /* err_sys ("Error reading HTTP header"); */
            return -1;
        /* 
         * if ((int) big_recvline[nr-1] == 10) nr--;
         */
        if (first_line) {
            /* sscanf parse "HTTP/1.x 200" */
            sscanf(big_recvline,"%*s %d", &reply_code);

            /* 204 No Content is not an error, message body is empty by definition, see RFC 2616 */
            if (reply_code == 204)
                return 0;       /* zero bytes is correct */

            if (!  (reply_code >= 200 && reply_code < 300) &&
                ! ((reply_code >= 300 && reply_code < 400) && accept_redirects))
                /* 
                 * Status codes beginning with 3 are not
                 * errors See bug #850674 and RFC 2616,
                 * section 10.3
                 */
                err_quit("HTTP error \"%s\"", big_recvline);
        }
        total = total + nr;
        first_line = FALSE;
    }
    /* Read the body */
    if (!ssl)
        nr = readline(fs.fs, big_recvline, MAXTOREAD, FALSE);
#ifdef OPENSSL
    else
        nr = SSL_readline(fs.ssl, big_recvline, MAXTOREAD, FALSE);
#endif
#ifdef GNUTLS
    else
    nr = TLS_readline(fs.tls, big_recvline, MAXTOREAD, FALSE);
#endif
    /* 
     * printf ("DEBUG: reading body \"%s\"\n (%d chars)\n", big_recvline,
     * nr);
     */
    if ((nr < 2) && (timeout_flag))     /* Probably a timeout */
        return -1;
    if (nr < 2)                 /* Hmm, if the body is empty, we'll get a * * * * *
                                 * * * meaningless error message */
        err_sys("Error reading HTTP body");
    total = total + nr;
    return total;               /* How to do if we want only the body's size? */
}

#endif                          /* HTTP */
