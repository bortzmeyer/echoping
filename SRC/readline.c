/*
 * Read a line from a descriptor with fgets 
 *
 * $Id$
 *
 */

#include "echoping.h"

int
readline (fs, ptr, maxlen, ln)
     FILE *fs;
     char *ptr;
     int maxlen;
     unsigned short ln;
{
  int n = 1;
  char *rc;

  if (ln)
    {
      rc = fgets (ptr, maxlen + 1, fs);
      /* printf ("DEBUG: %d bytes asked, I read \"%s\"\n", maxlen, rc); */
      if (rc == NULL)
	{
	  return (-1);
	}
      n = strlen (rc);
      return n;
    }
  else
    {
      while (n < maxlen)
	{
	  rc = fgets (ptr, maxlen, fs);
	  if (rc == NULL)
	    {
	      if (timeout_flag)
		return n;
	      if (n == 1)
		return (0);	/* EOF, no data read */
	      else
		break;		/* EOF, some data was read */
	    }
	  n = n + strlen (rc);
	}
    }

  return (n);
}
