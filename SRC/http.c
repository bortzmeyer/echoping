#include "echoping.h"

#ifdef HTTP
#include "HTParse.h"


char big_recvline[MAXTOREAD];

char *
make_http_sendline (char *url, char *host, int port)
{
  short sport = (short) port;
  int size = 200;		/* Enough? */
  char *sendline = (char *) malloc (size);
  char *hostname = (char *) malloc (size);
#ifdef HTTP10
  sprintf (sendline, "GET %s HTTP/1.0\r\nUser-Agent: Echoping/%s\r\n\r\n",
	   url, VERSION);
#else
  hostname = HTParse (url, "", PARSE_HOST);
  if (!strcmp (hostname, ""))
    sprintf (hostname, "%s:%d", host, sport);
  sprintf (sendline,
	   "GET %s HTTP/1.1\r\nUser-Agent: Echoping/%s\r\nHost: %s\r\nConnection: close\r\n\r\n",
	   url, VERSION, hostname);
  /* free (hostname); */ /* At least on AI/X, it segfaults for an unknown reason */
#endif
  return sendline;
}

void
find_server_and_port (char *server, short *port, char *default_port)
{
  char *text_port, *p;
  struct servent *sp;
  for (p = server; *p; p++)
    {
      if (*p == ':')
	{
	  *p = 0;
	  text_port = p + 1;
	  *port = atoi (text_port);
	}
    }
  if (*port == 0)
    {
      if ((sp = getservbyname (default_port, "tcp")) == NULL)
	{
	  err_quit ("tcp_open: unknown service: %s/tcp", default_port);
	}
      *port = sp->s_port;
    }
  else
    *port = htons (*port);
}

int
read_from_server (int fd)
{
  int nr;
  int total = 0;
  char reply_code;
  int first_line = TRUE;
  short body = FALSE;
  while (!body)
    {
      nr = readline (fd, big_recvline, MAXTOREAD, TRUE);
      /* HTTP replies should be separated by CR-LF. Unfortunately, some
         servers send only CR :-( */
      body = ((nr == 2) || (nr == 1));	/* Empty line CR-LF seen */
      if ((nr < 1) && (errno == EINTR))		/* Probably a timeout */
	return -1;
      if (nr < 1)
	/* SourceForge bug #109385 */
	/* err_sys ("Error reading HTTP header"); */
	return -1;
      /* if ((int) big_recvline[nr-1] == 10)
         nr--; */
      if (first_line)
	{
	  reply_code = big_recvline[9];		/* 9 because "HTTP/1.x 200..." */
	  if (reply_code != '2')	/* Status codes beginning with 3 are not errors
					   but should never appear in reply to echoping's requests */
	    err_quit ("HTTP error \"%s\"", big_recvline);
	}
      total = total + nr;
      first_line = FALSE;
    }
  /* Read the body */
  nr = readline (fd, big_recvline, MAXTOREAD, FALSE);
  if ((nr < 2) && (errno == EINTR))	/* Probably a timeout */
    return -1;
  if (nr < 2)			/* Hmm, if the body is empty, we'll
                                   get a meaningless error message */
    err_sys ("Reading HTTP body");
  total = total + nr;
  return total;			/* How to do if we want only the body's size? */
}

#endif /* HTTP */
