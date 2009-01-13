#include <stdio.h>

/* http://xmlrpc-c.sourceforge.net/ */
#include <xmlrpc.h>
#include <xmlrpc_client.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define CLIENT_NAME "XML-RPC Adams Names plugin for echoping"
#define CLIENT_VERSION "0.0"

#define ENDPOINT "http://www.adamsnames.tc/api/xmlrpc"

void
die_if_fault_occurred(xmlrpc_env * env)
{
    if (env->fault_occurred) {
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n",
                env->fault_string, env->fault_code);
        exit(1);
    }
}

int
main(int argc, char **argv)
{
    int             value;
    xmlrpc_value   *result;
    xmlrpc_bool     free, read_contacts;
    xmlrpc_int32    reason;
    xmlrpc_value   *domain_h;
    xmlrpc_int32    found;
    xmlrpc_value   *error;
    xmlrpc_env      env;
    char           *domain;
    char           *date, *holder;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s domain\n", argv[0]);
        exit(1);
    }

    domain = argv[1];

    /* Start up our XML-RPC client library. */
    xmlrpc_client_init(XMLRPC_CLIENT_NO_FLAGS, CLIENT_NAME, CLIENT_VERSION);

    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);

    /* Call the server */
    result = xmlrpc_client_call(&env, ENDPOINT, "domquery", "(s)", domain);
    die_if_fault_occurred(&env);

    xmlrpc_parse_value(&env, result, "{s:i,*}", "found", &found);
    die_if_fault_occurred(&env);
    if (found) {
        printf("%s is there\n", domain);
        xmlrpc_parse_value(&env, result, "{s:S,s:i,s:A,*}", "domain",
                           &domain_h, "found", &found, "error", &error);
        die_if_fault_occurred(&env);
        /* printf ("Type of domain: %d\n", xmlrpc_value_type(domain_h)); */
        xmlrpc_parse_value(&env, domain_h, "{s:s,s:s,*}", "registered",
                           &date, "org", &holder);
        die_if_fault_occurred(&env);
        printf("Registered on %s by %s\n", date, holder);
    } else {
        printf("Unknown domain %s\n", domain);
    }
    /* Dispose of our result value. */
    xmlrpc_DECREF(result);
    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(&env);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_client_cleanup();

}
