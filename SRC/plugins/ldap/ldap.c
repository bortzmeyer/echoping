/*
 * LDAP plugin. 
 * TODO: loops with and without opening the connection each time?
 * $Id$
 */

#define IN_PLUGIN
#include "../../echoping.h"

#include <ldap.h>

const char *request = NULL;
const char *base = NULL;
int scope = LDAP_SCOPE_BASE;
const char *hostname;
unsigned int port = 0;
LDAP *session;
poptContext ldap_poptcon;
echoping_options global_options;

void
ldap_usage (const char *msg)
{
  if (msg)
    {
      printf ("LDAP plugin error: %s\n", msg);
    }
  poptPrintUsage (ldap_poptcon, stdout, 0);
  exit (1);
}

char *
init (const int argc, const char **argv,
      const echoping_options global_external_options)
{
  int value;
  char *msg;
  char *scope_string = NULL;
  /* popt variables */
  struct poptOption options[] = {
    {"request", 'r', POPT_ARG_STRING, &request, 0,
     "Request (filter) to send to the LDAP server", 'r'},
    {"base", 'b', POPT_ARG_STRING, &base, 0,
     "Base of the LDAP tree", 'b'},
    {"scope", 's', POPT_ARG_STRING, &scope_string, 0,
     "Scope of the search in the LDAP tree (sub, one or base)", 's'},
    {"port", 'p', POPT_ARG_INT, &port, 0,
     "TCP port to connect to the LDAP server", 'p'},
    POPT_AUTOHELP POPT_TABLEEND
  };
  global_options = global_external_options;
  if (global_options.udp)
    err_quit ("UDP makes no sense for a LDAP connection");
  ldap_poptcon = poptGetContext (NULL, argc,
				 argv, options, POPT_CONTEXT_KEEP_FIRST);
  while ((value = poptGetNextOpt (ldap_poptcon)) > 0)
    {
      if (value < -1)
	{
	  sprintf (msg, "%s: %s",
		   poptBadOption (ldap_poptcon, POPT_BADOPTION_NOALIAS),
		   poptStrerror (value));
	  ldap_usage (msg);
	}
    }
  if (port == 0)
    port = LDAP_PORT;
  hostname = poptGetArg (ldap_poptcon);
  if (base == NULL)
    base = "";
  if (request == NULL || !strcmp (request, ""))
    request = "(objectclass=*)";
  if (scope_string != NULL)
    {
      scope_string = to_upper (scope_string);
      if (!strcmp (scope_string, "BASE"))
	scope = LDAP_SCOPE_BASE;
      else if (!strcmp (scope_string, "SUB"))
	scope = LDAP_SCOPE_SUBTREE;
      else if (!strcmp (scope_string, "ONE"))
	scope = LDAP_SCOPE_ONELEVEL;
      else
	err_quit ("Invalid scope \"%s\"", scope);
    }
  return "ldap";
}

void
start ()
{
  int result;
  session = ldap_init (hostname, port);
  if (session == NULL)
    err_sys ("Cannot initialize LDAP");
  /* TODO: allow non-anonymous connections, with ldap_bind_simple_s */
}

int
execute ()
{
  int result;
  LDAPMessage *response;
  result = ldap_search_s (session, base, scope, request, NULL,	/* Return all attributes */
			  0,	/* Return attribute types *and* values */
			  &response);
  if (result != 0)
    {
      err_ret ("Cannot search \"%s\": %s", request, ldap_err2string (result));
      return -1;
    }
  if (global_options.verbose)
    printf ("Retrieved: %d entries\n",
	    ldap_count_entries (session, response));
  return 0;
}

void
terminate ()
{
  ldap_unbind_s (session);
}
