/* Most of it stolen from Pierre Beyssac's bing */

/* $Id$ */

#include "echoping.h"

#define STATES 32

#include <time.h>
#include <ctype.h>

char *
random_string (unsigned length)
{

  char *state = (char *) malloc (sizeof (char) * STATES);
  char *result = (char *) malloc (length);
  int i, number;
  unsigned seed = (unsigned) time ((time_t *) NULL);

  /* printf ("Seed is %u\n", seed); */

  /* Initialize random generator */
  (void) initstate (seed, state, STATES);

  for (i = 0; i < length; i++)
    {
      number = (random () % 94) + 33;
      /* printf ("Number for %d is %d\n", i, number); */
      result[i] = (char) number;
    }
  result[length] = '\0';

  /* printf ("Result is %s\n", result); */

  return result;

}

char *
to_upper (char *input)
{
  int c;
  char *result;
  result = (char *) malloc (strlen (input));
  for (c = 0; c < strlen (input); c++)
    result[c] = toupper (input[c]);
  result[strlen (input)] = '\0';
  return result;
}

/*
 * tvsub -- Subtract 2 timeval structs:  out = out - in. Out is assumed to be
 * >= in. Comes from the bing program. 
 */
void
tvsub (out, in)
     struct timeval *out, *in;
{
  if ((out->tv_usec -= in->tv_usec) < 0)
    {
      --out->tv_sec;
      out->tv_usec += 1000000;
    }
  out->tv_sec -= in->tv_sec;
}

/* tvadd -- Adds 2 timeval structs:  out = out + in. */
void
tvadd (out, in)
     struct timeval *out, *in;
{
  if ((out->tv_usec += in->tv_usec) >= 1000000)
    {
      ++out->tv_sec;
      out->tv_usec -= 1000000;
    }
  out->tv_sec += in->tv_sec;
}

/* tvavg -- Averages a timeval struct */
void
tvavg (out, number)
     struct timeval *out;
     int number;
{
  double result;
  /*
   * out->tv_sec = out->tv_sec/number; out->tv_usec =
   * out->tv_usec/number; 
   */
  result = (1000000 * out->tv_sec + out->tv_usec) / number;
  /* printf ("Result of average is %f\n", result) */ ;
  out->tv_sec = (long) (result / 1000000);
  out->tv_usec = (long) (result - (out->tv_sec * 1000000));
}

/* tvstddev -- Computes the standard deviation of a set of results */
void
tvstddev (out, number, average, results)
     struct timeval *out;
     int number;
     struct timeval average;
     struct result *results;
{
  int i;
  struct timeval result, avg, var = null_timeval;
  struct timeval square, large, small;
  *out = null_timeval;
  for (i = 0; i < number; i++)
    {
      if (results[i].valid == 1)
	{
	  result = results[i].timevalue;
	  /* printf ("value is %f (average is %f)\n", 
	     tv2double (result), tv2double (average)); */
	  avg = average;
	  if (tvcmp (&result, &avg) == -1)
	    {
	      small = result;
	      large = avg;
	    }
	  else
	    {
	      large = result;
	      small = avg;
	    }
	  tvsub (&large, &small);
	  /* printf ("abs offset is %f\n", tv2double (large)); */
	  square = double2tv (pow (tv2double (large), 2));
	  tvadd (&var, &square);
	  /* printf ("variance is now %f\n", tv2double (var)); */
	}
    }
  result = double2tv (sqrt (tv2double (var) / number));
  out->tv_sec = result.tv_sec;
  out->tv_usec = result.tv_usec;
}

/* tvcmp -- Compares two timeval structs */
int
tvcmp (left, right)
     struct timeval *left, *right;
{
  if (left->tv_sec < right->tv_sec)
    {
      return -1;
    }
  if (left->tv_sec > right->tv_sec)
    {
      return 1;
    }
  if (left->tv_usec < right->tv_usec)
    {
      return -1;
    }
  if (left->tv_usec > right->tv_usec)
    {
      return 1;
    }
  return 0;

}

/* tvmin */
void
tvmin (champion, challenger)
     struct timeval *champion, *challenger;
{
  if (tvcmp (champion, challenger) == 1)
    {
      champion->tv_sec = challenger->tv_sec;
      champion->tv_usec = challenger->tv_usec;
    }
}

/* tvmax */
void
tvmax (champion, challenger)
     struct timeval *champion, *challenger;
{
  if (tvcmp (champion, challenger) == -1)
    {
      champion->tv_sec = challenger->tv_sec;
      champion->tv_usec = challenger->tv_usec;
    }
}

double
tv2double (tv)
     struct timeval tv;
{
  double result;
  result =
    (((((double) tv.tv_sec) * 1000000.0) + (double) tv.tv_usec) / 1000000.0);
  /* printf ("Double is %9.3f\n", result); */
  return result;
}

struct timeval
double2tv (x)
     double x;
{
  struct timeval result;
  result.tv_sec = (int) (x);
  result.tv_usec = (int) ((x - result.tv_sec) * 1000000);
  return result;
}
