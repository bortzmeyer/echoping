/*
 * LDAP plugin. TODO: loops with and without opening the connection each
 * time? $Id$
 */

#define IN_PLUGIN
#include "../../echoping.h"
#ifdef HAVE_CONFIG_H
#include "../../config.h"
#include "config.h"
#endif
#include <ldap.h>

const char     *request = NULL;
const char     *base = NULL;
int             scope = LDAP_SCOPE_BASE;
const char     *hostname;
int             port = 0;
LDAP           *session;
poptContext     ldap_poptcon;
echoping_options global_options;

void
ldap_usage(const char *msg)
{
    if (msg) {
        printf("LDAP plugin error: %s\n", msg);
    }
    poptPrintUsage(ldap_poptcon, stdout, 0);
    exit(1);
}

char           *
init(const int argc, const char **argv,
     const echoping_options global_external_options)
{
    int             value;
    char           *msg = malloc(MAX_LINE);
    char           *rest, *port_text;
    char           *scope_string = NULL;
    /* popt variables */
    struct poptOption options[] = {
        {"request", 'r', POPT_ARG_STRING, &request, 0,
         "Request (filter) to send to the LDAP server", "r"},
        {"base", 'b', POPT_ARG_STRING, &base, 0,
         "Base of the LDAP tree", "b"},
        {"scope", 's', POPT_ARG_STRING, &scope_string, 0,
         "Scope of the search in the LDAP tree (sub, one or base)", "s"},
        {"port", 'p', POPT_ARG_INT, &port, 0,
         "TCP port to connect to the LDAP server", "p"},
        POPT_AUTOHELP POPT_TABLEEND
    };
    global_options = global_external_options;
    if (global_options.udp)
        err_quit("UDP makes no sense for a LDAP connection");
    ldap_poptcon = poptGetContext(NULL, argc,
                                  argv, options, POPT_CONTEXT_KEEP_FIRST);
    while ((value = poptGetNextOpt(ldap_poptcon)) > 0) {
    }
    if (value < -1) {
        sprintf(msg, "%s: %s",
                poptBadOption(ldap_poptcon, POPT_BADOPTION_NOALIAS),
                poptStrerror(value));
        ldap_usage(msg);
    }
    if (port == 0)
        port = LDAP_PORT;
    hostname = poptGetArg(ldap_poptcon);
    rest = poptGetArg(ldap_poptcon);
    if (rest != NULL) {
        fprintf(stderr, "%s: ", rest);
        ldap_usage("Additional arguments");
    }
    if (base == NULL)
        base = "";
    if (request == NULL || !strcmp(request, ""))
        request = "(objectclass=*)";    /* Default mentioned in OpenLDAP
                                         * documentation.  Joerg Roth fears that it
                                         * may trigger "Size limit exceeded" if
                                         * there are many objects at this node. RFC
                                         * 4515 seems silent here. */
    if (scope_string != NULL) {
        scope_string = (char *) to_upper(scope_string);
        if (!strcmp(scope_string, "BASE"))
            scope = LDAP_SCOPE_BASE;
        else if (!strcmp(scope_string, "SUB"))
            scope = LDAP_SCOPE_SUBTREE;
        else if (!strcmp(scope_string, "ONE"))
            scope = LDAP_SCOPE_ONELEVEL;
        else
            err_quit("Invalid scope \"%s\"", scope_string);
    }
    if (port == LDAP_PORT) {
        return "ldap";
    } else {
        port_text = malloc(99);
        sprintf(port_text, "%d", port);
        return port_text;
    }
}

void
start()
{
    int             result;
    LDAPMessage    *response;

    session = ldap_init(hostname, port);
    if (session == NULL)
        err_sys("Cannot initialize LDAP");
    /* TODO: allow non-anonymous connections, with ldap_bind_simple_s */
    /* 
     * Unfortunately, ldap_init does not connect to the LDAP server. So
     * connection errors (e.g. firewall), will not be detected here and
     * loop will go on.
     * 
     * To quote the man page: ldap_init() acts just like ldap_open(), but
     * does not open a connection to the LDAP server.  The actual
     * connection open will occur when the first operation is attempted.
     * At this time, ldap_init() is preferred.  ldap_open() will be
     * depreciated in a later release.
     * 
     * So, we perform a dummy search immediately.
     *  
     * See #1879652 for why we use "dummystuff" and not "*"
     *
     */
    result = ldap_search_s(session, base, LDAP_SCOPE_ONELEVEL, "(objectClass=dummystuff)", NULL,        /* Return
                                                                                                         * * all * 
                                                                                                         * * *
                                                                                                         * attributes 
                                                                                                         */
                           1, &response);
    if (result != 0) {
        err_quit
            ("Cannot connect to %s (no LDAP server or wrong base, probably): %s",
             hostname, ldap_err2string(result));
    }
}

int
execute()
{
    int             result;
    LDAPMessage    *response;
    result = ldap_search_s(session, base, scope, request, NULL, /* Return all
                                                                 * attributes */
                           0,   /* Return attribute types *and* values */
                           &response);
    if (result != 0) {
        err_ret("Cannot search \"%s\": %s", request, ldap_err2string(result));
        return -1;
    }
    if (global_options.verbose)
        printf("Retrieved: %d entries\n", ldap_count_entries(session, response));
    return 0;
}

void
terminate()
{
    ldap_unbind_s(session);
}
