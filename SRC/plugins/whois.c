/*
 * Whois (RFC 954) plugin. 
 *
 * $Id$
 */

#define IN_PLUGIN
#include "../echoping.h"

#define MAX_REQUEST 256

struct addrinfo whois_server;
const char *request = "nic.fr";
int n;
int sockfd;
FILE *files = NULL;

char *
init (const int argc, const char **argv)
{
  return "nicname";
}

void
start (struct addrinfo *res)
{
  whois_server = *res;
}

void
execute ()
{
  int nr = 0;
  char recvline[MAX_LINE + 1];
  char complete_request[MAX_REQUEST];
  if ((sockfd =
       socket (whois_server.ai_family, whois_server.ai_socktype,
	       whois_server.ai_protocol)) < 0)
    err_sys ("Can't open socket");
  if (connect (sockfd, whois_server.ai_addr, whois_server.ai_addrlen) < 0)
    err_sys ("Can't connect to server");
  if ((files = fdopen (sockfd, "r")) == NULL)
    err_sys ("Cannot fdopen");
  sprintf (complete_request, "%s\r\n", request);
  n = strlen (complete_request);
  if (writen (sockfd, complete_request, n) != n)
    err_sys ("writen error on socket");
  /* Read from the server */
  nr = readline (files, recvline, n, 0);
  close (sockfd);
}
