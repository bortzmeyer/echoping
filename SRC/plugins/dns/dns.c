/*
 * DNS plugin. 
 * $Id$
 */

#define IN_PLUGIN
#include "../../echoping.h"

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

struct addrinfo name_server;
poptContext dns_poptcon;
char *request;
int type;
boolean use_tcp = FALSE;
boolean no_recurse = FALSE;

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
int
nsError (error, domain)
     int error;
     char *domain;
{
  switch (error)
    {
    case HOST_NOT_FOUND:
      err_ret ("Unknown domain: %s\n", domain);
      return -1;
    case NO_DATA:
      err_ret ("No records for %s in the Answer section\n", domain);
      return -1;
    case TRY_AGAIN:
      err_ret ("No response for query\n");
      return -2;
    default:
      err_ret ("Unexpected error\n");
      return -1;
    }
}

void
dns_usage (const char *msg)
{
  if (msg)
    {
      fprintf (stderr, "Error: %s\n", msg);
    }
  poptPrintUsage (dns_poptcon, stderr, 0);
  fprintf (stderr, "  request\n");
  exit (1);
}

char *
init (const int argc, const char **argv)
{
  int value;
  char *hostname;
  char *msg = malloc (256);
  char *type_name, *upper_type_name = NULL;
  /* popt variables */
  struct poptOption options[] = {
    {"type", 't', POPT_ARG_STRING, &type_name, 0,
     "Type of resources queried (A, MX, SOA, etc)",
     "type"},
    {"tcp", NULL, POPT_ARG_NONE, &use_tcp, 0,
     "Use TCP for the request (virtual circuit)",
     "tcp"},
    {"no-recurse", NULL, POPT_ARG_NONE, &no_recurse, 0,
     "Do not ask recursion",
     "no-recurse"},
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
  hostname = (char *) poptGetArg (dns_poptcon);	/* Not used */
  request = (char *) poptGetArg (dns_poptcon);
  if (request == NULL)
    dns_usage ("Mandatory request missing");
  if ((type_name == NULL) || !strcmp (type_name, ""))
    type = T_A;
  else
    {
      upper_type_name = to_upper (type_name);
      /* TODO: a better algorithm. Use dns_rdatatype_fromtext in BIND ? */
      if (!strcmp (upper_type_name, "A"))
	type = T_A;
      else if (!strcmp (upper_type_name, "AAAA"))
	type = T_AAAA;
      else if (!strcmp (upper_type_name, "NS"))
	type = T_NS;
      else if (!strcmp (upper_type_name, "SOA"))
	type = T_SOA;
      else if (!strcmp (upper_type_name, "MX"))
	type = T_MX;
      else if (!strcmp (upper_type_name, "SRV"))
	type = T_SRV;
      else if (!strcmp (upper_type_name, "CNAME"))
	type = T_CNAME;
      else if (!strcmp (upper_type_name, "PTR"))
	type = T_PTR;
      else if (!strcmp (upper_type_name, "TXT"))
	type = T_TXT;
      else
	dns_usage ("Unknown type");
    }
  return "domain";
}

void
start (struct addrinfo *res)
{
  struct sockaddr name_server_sockaddr;
  struct sockaddr_in name_server_sockaddr_in;
  name_server = *res;
  name_server_sockaddr = *name_server.ai_addr;
  /* Converts a generic sockaddr to an IPv4 sockaddr_in */
  (void) memcpy ((void *) &name_server_sockaddr_in, &name_server_sockaddr,
		 sizeof (struct sockaddr));
  if (res_init () < 0)
    err_sys ("res_init");
  _res.nsaddr_list[0] = name_server_sockaddr_in;	/* TODO: and IPv6? We now if we have _res_ext (xBSD) so we should use HAVE_RES_EXT here */
  _res.nscount = 1;
  _res.options &= ~(RES_DNSRCH | RES_DEFNAMES | RES_NOALIASES);
  if (use_tcp)
    {
      _res.options |= RES_USEVC;
    }
  if (no_recurse)
    {
      _res.options &= ~RES_RECURSE;
    }
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
				    type, (u_char *) & response,	/*response buffer */
				    sizeof (response)))	/*buffer size    */
      < 0)
    {				/*If negative    */
      nsError (h_errno, request);	/* report the error           */
      if (h_errno == TRY_AGAIN)
	return -1;		/* More luck next time? */
      else
	return -2;		/* Give in */
    }
  /* TODO: better analysis of the replies. For instance, replies can be in the authority section (delegation info) */
  return 0;
}

void
terminate ()
{
}
