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

#ifdef OPENSSL

char SSL_buffer[MAXTOREAD];
int buf_ptr;
int buf_end;

int
SSL_readline (sslh, ptr, maxlen, ln)
     SSL *sslh;
     char *ptr;
     int maxlen;
     unsigned short ln;
{
  int rc = 0;
  int i, oi;
  if (ln)
    {
      /* Empty buffer */
      if (buf_end == 0) {
	rc = SSL_read (sslh, SSL_buffer, maxlen);
	buf_end = rc;
      }
      /* No more data in the buffer */
      else if (buf_ptr == buf_end) {
	buf_ptr = 0;
	rc = SSL_read (sslh, SSL_buffer, maxlen);
	buf_end = rc;
      }
      for (oi=buf_ptr,i=buf_ptr; SSL_buffer[i] != '\n'; i++) {
	*ptr++ = SSL_buffer[i];
	buf_ptr++;
      }
      if (SSL_buffer[i] == '\n') 
	buf_ptr++;
      *ptr = '\0';
      /* printf ("DEBUG: %d bytes asked, I read \"%d\", newline was at
	 %d\n", maxlen, rc, i); */
      return (i-oi);
    }
  else {
    /* Since OpenSSL reads at most 4096 characters, we should loop
       here like we do in non-SSSL readline */
    if ((buf_end == 0) || (buf_ptr == buf_end)) {
      rc = SSL_read (sslh, ptr, maxlen);
      buf_end = 0;
      buf_ptr = 0;
    }
    else {
      for (i=buf_ptr; i<maxlen && i<=buf_end; i++) {
	*ptr++ = SSL_buffer[i];
	rc++;
      }
    }
    return rc;
  }
}
#endif


