/*
 * Whois (RFC 954) plugin. 
 *
 * $Id$
 */

#define IN_PLUGIN
#include "../../echoping.h"

#define MAX_REQUEST 256

struct addrinfo whois_server;
const char *request = NULL;
int dump = FALSE;
int n;
int sockfd;
FILE *files = NULL;
poptContext whois_poptcon;

void
whois_usage (const char *msg)
{
  if (msg)
    {
      printf ("Error: %s\n", msg);
    }
  poptPrintUsage (whois_poptcon, stdout, 0);
  exit (1);
}

char *
init (const int argc, const char **argv)
{
  int value;
  char *msg = malloc (256);
  /* popt variables */
  struct poptOption options[] = {
    {"request", 'r', POPT_ARG_STRING, &request, 'r',
     "Request/query (a domain name or something else, depending on the server) to send to the whois server",
     "request"},
    {"dump", 'd', POPT_ARG_NONE, &dump, 'd',
     "Dumps the reply from the whois server",
     ""},
    POPT_AUTOHELP POPT_TABLEEND
  };
  whois_poptcon = poptGetContext (NULL, argc,
				  argv, options, POPT_CONTEXT_KEEP_FIRST);
  while ((value = poptGetNextOpt (whois_poptcon)) > 0)
    {
      if (value < -1)
	{
	  sprintf (msg, "%s: %s",
		   poptBadOption (whois_poptcon, POPT_BADOPTION_NOALIAS),
		   poptStrerror (value));
	  whois_usage (msg);
	}
      switch ((char) value)
	{
	case 'r':
	  break;
	case 'd':
	  break;
	default:
	  sprintf (msg, "Wrong option %d (%c)", value, (char) value);
	  whois_usage (msg);
	}
    }
  if (request == NULL)
    whois_usage ("Mandatory request missing");

  return "nicname";
}

void
start (struct addrinfo *res)
{
  whois_server = *res;
}

int
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
  while ((nr = readline (files, recvline, n, 0)) > 0)
    if (dump)
      printf ("%s", recvline);
  if (dump)
    printf ("\n");
  close (sockfd);
  return 1;
}

void terminate() {
}
