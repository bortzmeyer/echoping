#include "inet.h"

#define STATES 32

#include <time.h>

char           *
random_string(unsigned length)
{

	char           *state = (char *) malloc(sizeof(char) * STATES);
	char           *result = (char *) malloc(length);
	int             i, number;
	unsigned        seed = (unsigned) time((time_t *) NULL);

	/* printf ("Seed is %u\n", seed); */

	/* Initialize random generator */
	(void) initstate(seed, state, STATES);

	for (i = 0; i < (length - 1); i++) {
		number = (random() % 94) + 33;
		/* printf ("Number for %d is %d\n", i, number); */
		result[i] = (char) number;
	}
	result[length - 1] = '\0';

	/* printf ("Result is %s\n", result); */

	return result;

}

/*
 * tvsub -- Subtract 2 timeval structs:  out = out - in. Out is assumed to be
 * >= in. Comes from the bing program. 
 */
void
tvsub(out, in)
	struct timeval *out, *in;
{
	if ((out->tv_usec -= in->tv_usec) < 0) {
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

/* tvadd -- Adds 2 timeval structs:  out = out + in. */
void
tvadd(out, in)
	struct timeval *out, *in;
{
	if ((out->tv_usec += in->tv_usec) >= 1000000) {
		++out->tv_sec;
		out->tv_usec -= 1000000;
	}
	out->tv_sec += in->tv_sec;
}

/* tvavg -- Averages a timeval struct */
void
tvavg(out, number)
	struct timeval *out;
	int             number;
{
	double          result;
	/*
	 * out->tv_sec = out->tv_sec/number; out->tv_usec =
	 * out->tv_usec/number; 
	 */
	result = (1000000 * out->tv_sec + out->tv_usec) / number;
	 /* printf ("Result of average is %f\n", result) */ ;
	out->tv_sec = (long) (result / 1000000);
	out->tv_usec = (long) (result - (out->tv_sec * 1000000));
}

/* tvcmp -- Compares two timeval structs */
int
tvcmp(left, right)
	struct timeval *left, *right;
{
	if (left->tv_sec < right->tv_sec) {
		return -1;
	}
	if (left->tv_sec > right->tv_sec) {
		return 1;
	}
	if (left->tv_usec < right->tv_usec) {
		return -1;
	}
	if (left->tv_usec > right->tv_usec) {
		return 1;
	}
	return 0;

}

/* tvmin */
int
tvmin(champion, challenger)
	struct timeval *champion, *challenger;
{
	if (tvcmp(champion, challenger) == 1) {
		champion->tv_sec = challenger->tv_sec;
		champion->tv_usec = challenger->tv_usec;
	}
}

/* tvmax */
int
tvmax(champion, challenger)
	struct timeval *champion, *challenger;
{
	if (tvcmp(champion, challenger) == -1) {
		champion->tv_sec = challenger->tv_sec;
		champion->tv_usec = challenger->tv_usec;
	}
}

double
tv2double(tv)
	struct timeval  tv;
{
	double          result;
	result = (((((double) tv.tv_sec) * 1000000.0) + (double) tv.tv_usec) / 1000000.0);
	/* printf ("Double is %9.3f\n", result); */
	return result;
}
