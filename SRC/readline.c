/*
 * Read a line from a descriptor.   
 *
 * To save read() system calls, we first check the static buffer if it
 * still contains data. If so, we send it back. Otherwise, we call
 * read().
 *
 * If ln == 0, we treat newline as an ordinary charracter.
 *
 * $Id$
 *  */

#include "echoping.h"

char buffer[MAXTOREAD]; /* Must survive between calls to readline */

int
readline (fd, ptr, maxlen, ln)
     int fd;
     char *ptr;
     int maxlen;
     unsigned short ln;
{
  char *origptr = ptr;
  int n, i, j, rc;
  char s[maxlen];
  int bufend = strlen(buffer);

  n = 0;

  /* Use the buffer if it is still full */
  for (i = 0; i < strlen(buffer); i++) {
    *ptr++ = buffer[i];
    n++;
    if (buffer[i] == '\n' && ln == 1) 
      break;
  }
  if (n) {
    /* printf ("DEBUG, got %d bytes from the buffer\n", i); */
    strcpy (buffer, (char *)(buffer+i+1));
    *ptr = 0;
    return (n);
  }
  bufend = strlen (buffer);
 
  reading:
  for (; n < maxlen;)
    {
      if (timeout_flag)
	return n;
      if ((rc = read (fd, &s, maxlen)) > 0)
	{
	  /* printf ("DEBUG, %d bytes asked (nl = %d) %d bytes read\n", maxlen, ln, rc);  */
	  parsing:
	  for (i = 0; i < rc; i++) {
	    *ptr++ = s[i];
	    /* printf ("DEBUG, adding %c ", s[i]); */
	    n++;
	    if (s[i] == '\n' && ln == 1) {
	      
	      for (j = i+1; j < rc; j++) {
		buffer[bufend++] = s[j];
	      }
	      break;
	    }
	  }
	  if (s[i] == '\n' && ln == 1) {
	    break;
	  }
	}
      else if (rc == 0)
	{
	  if (n == 1)
	    return (0);		/* EOF, no data read */
	  else {
	    n++;
	    break;		/* EOF, some data was read */
	  }
	}
      else
	return (-1);		/* error */
    }

  *ptr = 0;
  /* printf ("DEBUG, Returning %d bytes (%s)\n", n, origptr); */
  return (n);
}

void clear_read_buffer () {
  buffer[0] = 0;
}
