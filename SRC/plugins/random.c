/*
 * Pseudo-random plugin. Just an example. $Id$
 */

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

void
init (const int argc, const char *argv[])
{
  struct timeval tv;
  (void) gettimeofday (&tv, (struct timezone *) NULL);
  srand (tv.tv_usec);
}

void
execute ()
{
  usleep (rand () % 1000000);
}
