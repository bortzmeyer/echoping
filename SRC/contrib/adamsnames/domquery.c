/* echoping plugin to query (with XML-RPC) Adam's Names, the DNS registry.
  See http://www.adamsnames.tc/api/xmlrpc.html.
  $Id$ 
*/

#define IN_PLUGIN
#include "../../echoping.h"

#include <stdio.h>

/* http://xmlrpc-c.sourceforge.net/ */
#include <xmlrpc.h>
#include <xmlrpc_client.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.h"

void
domquery_usage (char *msg)
{
  fprintf (stderr, "%s\n", msg);
  poptPrintUsage (poptcon, stderr, 0);
  err_quit("   domain");
}

char *
init (int argc, char **argv)
{
  int value;
  xmlrpc_value *result;
  xmlrpc_bool free, read_contacts;
  xmlrpc_int32 reason;
  char *msg, *hostname;

  struct poptOption options[] = {
    {"read-contacts", 'c', POPT_ARG_NONE, &read_contacts, 0,
     "Read also the contacts of the domain [NOT IMPLEMENTED]",
     ""},
    POPT_AUTOHELP POPT_TABLEEND
  };
  poptcon = poptGetContext (NULL, argc,
                                argv, options, POPT_CONTEXT_KEEP_FIRST);
  while ((value = poptGetNextOpt (poptcon)) > 0)
    {
      if (value < -1)
        {
          sprintf (msg, "%s: %s",
                   poptBadOption (poptcon, POPT_BADOPTION_NOALIAS),
                   poptStrerror (value));
          domquery_usage (msg);
        }
    }
  hostname = (char *) poptGetArg (poptcon); /* Not used */
  domain = (char *) poptGetArg (poptcon);
  if (domain == NULL)
    domquery_usage ("Mandatory request missing");

  return NULL;
}

void
start_raw() {

  /* Start up our XML-RPC client library. */
  xmlrpc_client_init (XMLRPC_CLIENT_NO_FLAGS, CLIENT_NAME, CLIENT_VERSION);

  /* Initialize our error-handling environment. */
  xmlrpc_env_init (&env);

}

int
execute ()
{
  xmlrpc_value *result;
  xmlrpc_value *domain_h;
  xmlrpc_bool found;
  xmlrpc_value *error;
  char *dst;
  dst = HTAnchor_findAddress(ENDPOINT);
  /* Call the server */
  result = xmlrpc_client_call (&env, ENDPOINT, "domquery", "(s)", domain);
  die_if_fault_occurred (&env);

  xmlrpc_parse_value (&env, result, "{{},b,[]*}", "domain", domain_h, "found", &found, "error", error);
  die_if_fault_occurred (&env);
  if (found)
    {
      printf ("%s is there\n", domain);
    }
  /* Dispose of our result value. */
  xmlrpc_DECREF (result);
  return 0;
}

void
terminate ()
{
			  /* Clean up our error-handling environment. */
			  xmlrpc_env_clean (&env);

			  /* Shutdown our XML-RPC client library. */
			  xmlrpc_client_cleanup ();

			  }
