/*
 * Pseudo-random plugin. Just an example. 
 * 
 * $Id$
 */

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

char           *
init(const int argc, const char *argv[])
{
    struct timeval  tv;
    (void) gettimeofday(&tv, (struct timezone *) NULL);
    srand(tv.tv_usec);
    return "7";                 /* Not used, just to say we use the cooked interface 
                                 */
}

void
start()
{
}

int
execute()
{
    usleep(rand() % 1000000);
    return 1;
}

void
terminate()
{
}
