/*
 * PostgreSQL plugin. 
 * TODO: loops with and without opening the connection each time?
 * $Id$
 */

#define IN_PLUGIN
#include "../../echoping.h"

#include <postgresql/libpq-fe.h>

const char *request = NULL;
int readall = FALSE;
poptContext postgresql_poptcon;
PGconn *conn;
PGresult *res;
char *conninfo;

void
postgresql_usage (const char *msg)
{
  if (msg)
    {
      printf ("PostgreSQL plugin error: %s\n", msg);
    }
  poptPrintUsage (postgresql_poptcon, stdout, 0);
  exit (1);
}

char *
init (const int argc, const char **argv)
{
  int value;
  char *msg = malloc (256);
  /* popt variables */
  struct poptOption options[] = {
    {"conninfo", 'c', POPT_ARG_STRING, &conninfo, 0,
     "Connection information for the Postgresql server. Something like 'host=foo dbname=bar''",
     "request"},
    {"request", 'r', POPT_ARG_STRING, &request, 0,
     "Request/query (in SQL) to send to the Postgresql server. Only SELECT are supported.",
     "request"},
    {"readall", 'a', POPT_ARG_NONE, &readall, 0,
     "Read all the data sent by the Postgresql server",
     ""},
    POPT_AUTOHELP POPT_TABLEEND
  };
  postgresql_poptcon = poptGetContext (NULL, argc,
				       argv, options,
				       POPT_CONTEXT_KEEP_FIRST);
  while ((value = poptGetNextOpt (postgresql_poptcon)) > 0)
    {
      if (value < -1)
	{
	  sprintf (msg, "%s: %s",
		   poptBadOption (postgresql_poptcon, POPT_BADOPTION_NOALIAS),
		   poptStrerror (value));
	  postgresql_usage (msg);
	}
    }
  if (request == NULL)		/* TODO: a default like SELECT now()? */
    postgresql_usage ("Mandatory request missing");
  if (conninfo == NULL)
    postgresql_usage ("Mandatory connection information missing");
  return NULL;			/* We only use the conninfo, echoping does not see our hostname or port */
}

void
start_raw ()
{
  conn = PQconnectdb (conninfo);
  if (conn == NULL)
    {
      err_quit ("Cannot create connection\n");
    }
  if (PQstatus (conn) == CONNECTION_BAD)
    {
      err_quit ("Connection failed: %s\n", PQerrorMessage (conn));
    }
}

int
execute ()
{
  unsigned int row, column;
  res = PQexec (conn, request);
  if (PQresultStatus (res) != PGRES_TUPLES_OK)
    {
      printf ("Cannot run \"%s\": %s\n", request, PQresultErrorMessage (res));
      return -1;
    }
  if (readall)
    {
      for (row = 0; row++; row < PQntuples (res))
	{
	  for (column = 0; column++; column < PQnfields (res))
	    {
	      PQgetvalue (res, row, column);
	      /* TODO: test the return code */
	    }
	}
    }
  return 0;
}

void
terminate ()
{
  PQfinish (conn);
}
