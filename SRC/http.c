/* $Id$ */

#include "echoping.h"

#ifdef HTTP
#include "HTParse.h"


char big_recvline[MAXTOREAD];

char *
make_http_sendline (char *url, char *host, int port, int nocache)
{
  short sport = (short) port;
  int size = 200;		/* Enough? */
  char *sendline = (char *) malloc (size);
  char *hostname = (char *) malloc (size);
  char *cache_directive = "";
#ifdef HTTP10
  if (nocache)
    cache_directive = "Pragma: no-cache\r\n";	/* RFC 1945, "Hypertext
						   Transfer Protocol --
						   HTTP/1.0" */
  sprintf (sendline,
	   "GET %s HTTP/1.0\r\nUser-Agent: Echoping/%s\r\n%s\r\n",
	   url, VERSION, cache_directive);
#else
  if (nocache)
    {
      if (nocache == 1)
	cache_directive = "Cache-control: max-age=0\r\n";	/* Simply force a
								   recheck with the
								   server */
      else
	cache_directive = "Cache-control: no-cache\r\n";	/* RFC 2616
								   "Hypertext
								   Transfer
								   Protocol --
								   HTTP/1.1" */
    }
  strcpy (hostname, HTParse (url, "", PARSE_HOST));
  if (!strcmp (hostname, ""))
    sprintf (hostname, "%s:%d", host, sport);
  sprintf (sendline,
	   "GET %s HTTP/1.1\r\nUser-Agent: Echoping/%s\r\nHost: %s\r\nConnection: close\r\n%s\r\n",
	   url, VERSION, hostname, cache_directive);
  free (hostname);
#endif
  return sendline;
}

int
read_from_server (CHANNEL fs, short ssl)
{
  int nr;
  int total = 0;
  char reply_code;
  int first_line = TRUE;
  short body = FALSE;
#ifdef OPENSSL
  int sslcode;
#endif
  while (!body)
    {
      if (!ssl)
	nr = readline (fs.fs, big_recvline, MAXTOREAD, TRUE);
#ifdef OPENSSL
      else {
	nr = SSL_readline (fs.ssl, big_recvline, MAXTOREAD, TRUE);
	if (nr == -1) {
	  sslcode = ERR_get_error ();
	  err_ret ("SSL_readline error: %s",
		   ERR_error_string (sslcode, NULL));
	}
      }
#endif
      /* printf ("DEBUG: reading \"%s\"\n (%d chars)\n", big_recvline, nr);  */
      /* HTTP replies should be separated by CR-LF. Unfortunately, some
         servers send only CR :-( */
      body = ((nr == 2) || (nr == 1));	/* Empty line CR-LF seen */
      if ((nr < 1) && (errno == EINTR))	/* Probably a timeout */
	return -1;
      if (nr < 1)
	/* SourceForge bug #109385 */
	/* err_sys ("Error reading HTTP header"); */
	return -1;
      /* if ((int) big_recvline[nr-1] == 10)
         nr--; */
      if (first_line)
	{
	  reply_code = big_recvline[9];	/* 9 because "HTTP/1.x 200..." */
	  if (reply_code != '2')	/* Status codes beginning with 3 are not errors
					   but should never appear in reply to echoping's requests */
	    err_quit ("HTTP error \"%s\"", big_recvline);
	}
      total = total + nr;
      first_line = FALSE;
    }
  /* Read the body */
  if (!ssl)
    nr = readline (fs.fs, big_recvline, MAXTOREAD, FALSE);
#ifdef OPENSSL
  else
    nr = SSL_readline (fs.ssl, big_recvline, MAXTOREAD, FALSE);
#endif
  /* printf ("DEBUG: reading body \"%s\"\n (%d chars)\n", big_recvline, nr);  */
  if ((nr < 2) && (errno == EINTR))	/* Probably a timeout */
    return -1;
  if (nr < 2)			/* Hmm, if the body is empty, we'll
				   get a meaningless error message */
    err_sys ("Error reading HTTP body");
  total = total + nr;
  return total;			/* How to do if we want only the body's size? */
}

#endif /* HTTP */
