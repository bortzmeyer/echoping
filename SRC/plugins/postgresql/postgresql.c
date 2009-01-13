/*
 * PostgreSQL plugin. 
 * $Id$
 */

#define IN_PLUGIN
#include "../../echoping.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef POSTGRESQL_PREFIX
#include <postgresql/libpq-fe.h>
#else
#include <libpq-fe.h>
#endif

const char     *request = NULL;
int             readall = FALSE;
int             connect_each_time = FALSE;
poptContext     postgresql_poptcon;
PGconn         *conn;
PGresult       *res;
char           *conninfo;
echoping_options global_options;

void
postgresql_usage(const char *msg)
{
    if (msg) {
        printf("PostgreSQL plugin error: %s\n", msg);
    }
    poptPrintUsage(postgresql_poptcon, stdout, 0);
    fprintf(stderr, "  [SQL-request]\n");
    exit(1);
}

char           *
init(const int argc, const char **argv,
     const echoping_options global_external_options)
{
    int             value;
    char           *msg = malloc(256);
    char           *rest;
    /* popt variables */
    struct poptOption options[] = {
        {"conninfo", 'c', POPT_ARG_STRING, &conninfo, 0,
         "Connection information for the Postgresql server. Something like 'host=foo dbname=bar''",
         ""},
        {"readall", 'a', POPT_ARG_NONE, &readall, 0,
         "Read all the data sent by the Postgresql server",
         ""},
        {"connect-each-time", 'e', POPT_ARG_NONE, &connect_each_time, 0,
         "(Re)Connect to the Postgresql server at each iteration",
         ""},
        POPT_AUTOHELP POPT_TABLEEND
    };
    global_options = global_external_options;
    if (global_options.udp)
        err_quit("UDP makes no sense for a PostgreSQL connection");
    postgresql_poptcon = poptGetContext(NULL, argc,
                                        argv, options, POPT_CONTEXT_POSIXMEHARDER);
    while ((value = poptGetNextOpt(postgresql_poptcon)) > 0) {
    }
    if (value < -1) {
        sprintf(msg, "%s: %s",
                poptBadOption(postgresql_poptcon, POPT_BADOPTION_NOALIAS),
                poptStrerror(value));
        postgresql_usage(msg);
    }
    request = poptGetArg(postgresql_poptcon);
    if (request == NULL)
        request = "SELECT now()";
    rest = poptGetArg(postgresql_poptcon);
    if (rest != NULL)
        postgresql_usage("Erroneous additional arguments");
    if (conninfo == NULL)
        conninfo = "";
    return NULL;                /* We only use the conninfo, echoping does not see
                                 * our hostname or port */
}

void
start_raw()
{
    if (!connect_each_time) {
        conn = PQconnectdb(conninfo);
        if (conn == NULL) {
            err_quit("Cannot create connection\n");
        }
        if (PQstatus(conn) == CONNECTION_BAD) {
            err_quit("Connection failed: %s\n", PQerrorMessage(conn));
        }
    }
}

int
execute()
{
    unsigned int    row, column;
    char           *result;
    if (connect_each_time) {
        conn = PQconnectdb(conninfo);
        if (conn == NULL) {
            err_ret("Cannot create connection\n");
            return -1;
        }
        if (PQstatus(conn) == CONNECTION_BAD) {
            err_ret("Connection failed: %s\n", PQerrorMessage(conn));
            return -1;
        }
    }
    res = PQexec(conn, request);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        err_ret("Cannot run \"%s\": %s\n", request, PQresultErrorMessage(res));
        return -1;
    }
    if (global_options.verbose)
        printf("%d tuples returned\n", PQntuples(res));
    if (readall) {
        for (row = 0; row < PQntuples(res); row++) {
            for (column = 0; column < PQnfields(res); column++) {
                result = PQgetvalue(res, row, column);
                if (result == NULL) {
                    err_ret("Cannot retrieve value [%d,%d]\n", row, column);
                    return -1;
                }
                /* else { printf ("DEBUG: [%d,%d] %s\n", row, column, result); } */
            }
        }
    }
    if (connect_each_time)
        PQfinish(conn);
    return 0;
}

void
terminate()
{
    if (!connect_each_time)
        PQfinish(conn);
}
