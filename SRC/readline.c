/*
 * Read a line from a descriptor.  Read the line one byte at a time, looking
 * for the newline.  We store the newline in the buffer, then follow it with
 * a null (the same as fgets(3)). We return the number of characters up to,
 * but not including, the null (the same as strlen(3)). If ln == 0, we treat
 * newline as an ordinary charracter. 
 */

/* Stolen from Stevens' book */

#include "echoping.h"

int
readline (fd, ptr, maxlen, ln)
     int fd;
     char *ptr;
     int maxlen;
     unsigned short ln;
{
  int n, rc;
  char c;

  for (n = 1; n < maxlen; n++)
    {
      if (timeout_flag)
	return n;
      if ((rc = read (fd, &c, 1)) == 1)
	{
	  *ptr++ = c;
	  if (c == '\n' && ln == 1)
	    break;
	}
      else if (rc == 0)
	{
	  if (n == 1)
	    return (0);		/* EOF, no data read */
	  else
	    break;		/* EOF, some data was read */
	}
      else
	return (-1);		/* error */
    }

  *ptr = 0;
  return (n);
}
