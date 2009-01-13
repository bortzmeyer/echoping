/*
 * Daytime (RFC 867) plugin. 
 *
 * $Id$
 */

#define IN_PLUGIN
#include <echoping/echoping.h>

struct addrinfo daytime_server;
int             sockfd;
echoping_options options;

char           *
init(const int argc, const char **argv, echoping_options global_options)
{
    if (global_options.udp)
        err_quit("Sorry, UDP is not yet compatible with this daytime plugin");
    options = global_options;
    return "daytime";
}

void
start(struct addrinfo *res)
{
    daytime_server = *res;
}

int
execute()
{
    int             nr;
    FILE           *file;
#define MAX 256
    char            recvline[MAX];
    if ((sockfd =
         socket(daytime_server.ai_family, daytime_server.ai_socktype,
                daytime_server.ai_protocol)) < 0)
        err_sys("Can't open socket");
    if (connect(sockfd, daytime_server.ai_addr, daytime_server.ai_addrlen) < 0)
        err_sys("Can't connect to server");
    if ((file = fdopen(sockfd, "r")) == NULL)
        err_sys("Cannot fdopen");
    nr = readline(file, recvline, MAX, 1);
    if (options.verbose)
        printf("%s", recvline);
    close(sockfd);
    return 1;
}

void
terminate()
{
}
