/*
 * DNS plugin. 
 *
 * $Id$
 */

#define IN_PLUGIN
#include "../echoping.h"

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

struct addrinfo name_server;
poptContext dns_poptcon;
char *request;
int type;

/* nsError stolen from Liu & Albitz check_soa (in their book "DNS and BIND") */

/****************************************************************
 * nsError -- Print an error message from h_errno for a failure *
 *     looking up NS records.  res_query() converts the DNS     *
 *     packet return code to a smaller list of errors and       *
 *     places the error value in h_errno.  There is a routine   *
 *     called herror() for printing out strings from h_errno    *
 *     like perror() does for errno.  Unfortunately, the        *
 *     herror() messages assume you are looking up address      *
 *     records for hosts.  In this program, we are looking up   *
 *     NS records for domains, so we need our own list of error *
 *     strings.                                                 *
 ****************************************************************/
void
nsError (error, domain)
     int error;
     char *domain;
{
  switch (error)
    {
    case HOST_NOT_FOUND:
      (void) fprintf (stderr, "Unknown domain: %s\n", domain);
      break;
    case NO_DATA:
      (void) fprintf (stderr, "No NS records for %s\n", domain);
      break;
    case TRY_AGAIN:
      (void) fprintf (stderr, "No response for NS query\n");
      break;
    default:
      (void) fprintf (stderr, "Unexpected error\n");
      break;
    }
}

void
dns_usage (const char *msg)
{
  if (msg)
    {
      printf ("Error: %s\n", msg);
    }
  poptPrintUsage (dns_poptcon, stdout, 0);
  exit (1);
}

char *
init (const int argc, const char **argv)
{
  int value;
  char *msg = malloc (256);
  char *type_name;
  /* popt variables */
  struct poptOption options[] = {
    {"request", 'r', POPT_ARG_STRING, &request, 0,
     "Request (a domain name) to send to the name server",
     "request"},
    {"type", 't', POPT_ARG_STRING, &type_name, 0,
     "Type of resources queried (A, MX, SOA, etc)",
     "type"},
    POPT_AUTOHELP POPT_TABLEEND
  };
  dns_poptcon = poptGetContext (NULL, argc,
				argv, options, POPT_CONTEXT_KEEP_FIRST);
  while ((value = poptGetNextOpt (dns_poptcon)) > 0)
    {
      if (value < -1)
	{
	  sprintf (msg, "%s: %s",
		   poptBadOption (dns_poptcon, POPT_BADOPTION_NOALIAS),
		   poptStrerror (value));
	  dns_usage (msg);
	}
    }
  if (request == NULL)
    dns_usage ("Mandatory request missing");

  return "domain";
}

void
start (struct addrinfo *res)
{
  name_server = *res;
  if (res_init () < 0)
    err_sys ("res_init");
}

int
execute ()
{
  union
  {
    HEADER hdr;			/* defined in resolv.h */
    u_char buf[PACKETSZ];	/* defined in arpa/nameser.h */
  } response;			/* response buffers */
  int response_length;		/* buffer length */
  if ((response_length = res_query (request,	/* the domain we care about   */
				    C_IN,	/* Internet class records     */
				    T_NS,	/* Look up name server records */
				    (u_char *) & response,	/*response buffer */
				    sizeof (response)))	/*buffer size    */
      < 0)
    {				/*If negative    */
      nsError (h_errno, request);	/* report the error           */
      return -1;		/* and quit                   */
    }
  return 0;
}

void
terminate ()
{
}
