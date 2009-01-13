/*
 *
 * $Id: daytime.c 395 2007-04-04 19:26:19Z bortz $
 */

#define IN_PLUGIN
#include <echoping/echoping.h>

struct addrinfo smallservices_server;
int             sockfd;
echoping_options options;

char           *
init(const int argc, const char **argv, echoping_options global_options)
{
    options = global_options;
    /* TODO: the service returned must depend on the options */
    return "echo";
}

void
start(struct addrinfo *res)
{
    smallservices_server = *res;
}

int
execute()
{
    int             nr;
#define MAX 256
#define TEST_STRING "test"
    char            result[MAX];
    if ((sockfd =
         socket(smallservices_server.ai_family, smallservices_server.ai_socktype,
                smallservices_server.ai_protocol)) < 0)
        err_sys("Can't open socket");
    if (connect
        (sockfd, smallservices_server.ai_addr, smallservices_server.ai_addrlen) < 0)
        err_sys("Can't connect to server");
    if (write(sockfd, TEST_STRING, strlen(TEST_STRING)) != strlen(TEST_STRING))
        err_sys("Cannot write");
    nr = read(sockfd, result, strlen(TEST_STRING));
    if (nr != strlen(TEST_STRING))
        err_sys("Cannot read (only %i bytes)", nr);     /* TODO: the server may send 
                                                         * the result in chunks, we 
                                                         * should loop */
    if (strcmp(result, TEST_STRING) != 0)
        err_sys("Result \"%s\" is different from test string \"%s\"",
                result, TEST_STRING);
    close(sockfd);
    return 1;
}

void
terminate()
{
}
