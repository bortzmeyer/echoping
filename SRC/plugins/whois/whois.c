/*
 * Whois (RFC 3912) plugin. 
 *
 * $Id$
 */

#define IN_PLUGIN
#include "../../echoping.h"
#ifdef HAVE_CONFIG_H
#include "../../config.h"
#include "config.h"
#endif

#define MAX_REQUEST 256

struct addrinfo whois_server;
char           *hostname;
const char     *request = NULL;
int             dump = FALSE;
int             n;
int             sockfd;
FILE           *files = NULL;
poptContext     whois_poptcon;
echoping_options general_options;

void
whois_usage(const char *msg)
{
    if (msg) {
        printf("Error: %s\n", msg);
    }
    poptPrintUsage(whois_poptcon, stdout, 0);
    fprintf(stderr, "  request\n");
    exit(1);
}

char           *
init(const int argc, const char **argv, echoping_options global_options)
{
    int             value;
    char           *msg = malloc(256);
    char           *rest;
    /* popt variables */
    struct poptOption options[] = {
        {"dump", 'd', POPT_ARG_NONE, &dump, 'd',
         "Dumps the reply from the whois server",
         ""},
        POPT_AUTOHELP POPT_TABLEEND
    };
    general_options = global_options;
    if (global_options.udp)
        err_quit("UDP is incompatible with this whois plugin");
    /* Will probably be catched before because /etc/services have no entry for UDP
     * port 43 */
    whois_poptcon = poptGetContext(NULL, argc,
                                   argv, options, POPT_CONTEXT_POSIXMEHARDER);
    while ((value = poptGetNextOpt(whois_poptcon)) > 0) {
        switch ((char) value) {
        case 'd':
            break;
        default:
            sprintf(msg, "Wrong option %d (%c)", value, (char) value);
            whois_usage(msg);
        }
    }
    if (value < -1) {
        sprintf(msg, "%s: %s",
                poptBadOption(whois_poptcon, POPT_BADOPTION_NOALIAS),
                poptStrerror(value));
        whois_usage(msg);
    }
    request = (char *) poptGetArg(whois_poptcon);
    if (request == NULL)
        whois_usage("Mandatory request missing");
    rest = (char *) poptGetArg(whois_poptcon);
    if (rest != NULL && strcmp(rest, ""))
        whois_usage("Extraneous arguments ignored");
    return "nicname";
}

void
start(struct addrinfo *res)
{
    whois_server = *res;
}

int
execute()
{
    int             nr = 0;
    char            recvline[MAX_LINE + 1];
    char            complete_request[MAX_REQUEST];
#ifdef HAVE_TCP_INFO
    struct tcp_info tcpinfo;
    socklen_t       socket_length = sizeof(tcpinfo);
#endif
    if ((sockfd =
         socket(whois_server.ai_family, whois_server.ai_socktype,
                whois_server.ai_protocol)) < 0)
        err_sys("Can't open socket");
    if (connect(sockfd, whois_server.ai_addr, whois_server.ai_addrlen) < 0)
        err_sys("Can't connect to server");
    if ((files = fdopen(sockfd, "r")) == NULL)
        err_sys("Cannot fdopen");
    sprintf(complete_request, "%s\r\n", request);
    n = strlen(complete_request);
    if (writen(sockfd, complete_request, n) != n)
        err_sys("writen error on socket");
    /* Read from the server */
    while ((nr = readline(files, recvline, MAX_LINE, 0)) > 0)
        if (dump)
            printf("%s", recvline);
    if (dump)
        printf("\n");
#ifdef HAVE_TCP_INFO
    /* Thanks to Perry Lorier <perry@coders.net> for the tip */
    if (general_options.verbose) {
        if (getsockopt(sockfd, SOL_TCP, TCP_INFO, &tcpinfo, &socket_length)
            != -1) {
            printf("Estimated TCP RTT: %.04f seconds\n",
                   tcpinfo.tcpi_rtt / 1000000.0);
        }
    }
#endif
    close(sockfd);
    return 1;
}

void
terminate()
{
}
