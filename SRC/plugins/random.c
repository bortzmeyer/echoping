/*
 * Pseudo-random plugin. Just an example. 
 * 
 * $Id$
 */

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

char *
init (const int argc, const char *argv[])
{
  struct timeval tv;
  (void) gettimeofday (&tv, (struct timezone *) NULL);
  srand (tv.tv_usec);
  return NULL;
}

void start ()
{
}

int
execute ()
{
  usleep (rand () % 1000000);
  return 1;
}

void terminate() {}
