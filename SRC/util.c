/* Most of it stolen from Pierre Beyssac's bing */

/* $Id$ */

#include "echoping.h"

#define STATES 32

#include <time.h>
#include <ctype.h>

char           *
random_string(unsigned length)
{

    char           *state = (char *) malloc(sizeof(char) * STATES);
    char           *result = (char *) malloc(length + 1);
    int             i, number;
    unsigned        seed = (unsigned) time((time_t *) NULL);

    /* printf ("Seed is %u\n", seed); */

    /* Initialize random generator */
    (void) initstate(seed, state, STATES);

    for (i = 0; i < length; i++) {
        number = (random() % 94) + 33;
        /* printf ("Number for %d is %d\n", i, number); */
        result[i] = (char) number;
    }
    result[length] = '\0';

    /* printf ("Result is %s\n", result); */

    return result;

}

char           *
to_upper(char *input)
{
    int             c;
    char           *result;
    result = (char *) malloc(strlen(input));
    for (c = 0; c < strlen(input); c++)
        result[c] = toupper((int) input[c]);
    result[strlen(input)] = '\0';
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

/* tvstddev -- Computes the standard deviation of a set of results */
void
tvstddev(out, number, average, results)
    struct timeval *out;
    int             number;
    struct timeval  average;
    struct result  *results;
{
    int             i;
    struct timeval  result = null_timeval;
    struct timeval  avg = null_timeval;
#ifdef DEBUG
    struct timeval  var = null_timeval;
#endif
    struct timeval  large, small;
    double          d_offset, d_square, d_variance = 0;
    *out = null_timeval;
    for (i = 0; i < number; i++) {
        if (results[i].valid == 1) {
            result = results[i].timevalue;
#ifdef DEBUG
            printf("DEBUG: Value is %f (average is %f)\n", tv2double
                   (result), tv2double(average));
#endif
            avg = average;
            if (tvcmp(&result, &avg) == -1) {
                small = result;
                large = avg;
            } else {
                large = result;
                small = avg;
            }
            tvsub(&large, &small);
#ifdef DEBUG
            printf("abs offset is %f\n", tv2double(large));
#endif
            d_offset = tv2double(large);
            d_square = d_offset * d_offset;
            d_variance += d_square;
#ifdef DEBUG
            printf("variance is now %f\n", tv2double(var));
#endif
        }
    }
    result = double2tv(sqrt(d_variance / (double) number));
    out->tv_sec = result.tv_sec;
    out->tv_usec = result.tv_usec;
}


 /* tvstddevavg -- Computes the average of values within a set of results where the
  * sample is within the given number of standard deviations from the average */
/* TODO: IWBN to return the number of excluded outliers */
void
tvstddevavg(out, number, average, results, n_stddev)
    struct timeval *out;        /* contains std dev on entry */
    int             number;
    struct timeval  average;
    struct result  *results;
    double          n_stddev;
{
    int             i, valid = 0;
    struct timeval  result;     /* working value */
    struct timeval  var = null_timeval; /* result accumulator */
    double          x;
    double          maxdev = tv2double(*out) * n_stddev;

    if (tvcmp(out, &null_timeval) == 0) {
        /* if the SD is 0 then we just return the average */
        *out = average;
        return;
    }

    for (i = 0; i < number; i++) {
        if (results[i].valid == 1) {
            result = results[i].timevalue;
            tvsub(&result, &average);
            /* printf ("value is %f (stddev is %f)\n", tv2double (result), tv2double 
             * (stddev)); */
            /* ensure that result (difference to average) is absolute value */
            if (tvcmp(&result, &null_timeval) == -1) {
                result = average;
                tvsub(&result, &results[i].timevalue);
            }
            x = tv2double(result);
            /* printf("value is %g maxdev %g\n",x,maxdev); */
            if (x <= maxdev) {
                /* deviation is less than stddev */
                tvadd(&var, &results[i].timevalue);
                valid++;
            } else {
                /* printf("dropped\n"); */
            }
        }
    }
    /* printf ("total is %f in %d samples\n", tv2double (var), valid); */
    if (valid > 0) {
        *out = double2tv(tv2double(var) / valid);
    } else {
        *out = null_timeval;
    }

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
void
tvmin(champion, challenger)
    struct timeval *champion, *challenger;
{
    if (tvcmp(champion, challenger) == 1) {
        champion->tv_sec = challenger->tv_sec;
        champion->tv_usec = challenger->tv_usec;
    }
}

/* tvmax */
void
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
    result =
        (((((double) tv.tv_sec) * 1000000.0) + (double) tv.tv_usec) / 1000000.0);
    /* printf ("Double is %9.3f\n", result); */
    return result;
}

struct timeval
double2tv(x)
    double          x;
{
    struct timeval  result;
    result.tv_sec = (int) (x);
    result.tv_usec = (int) ((x - result.tv_sec) * 1000000);
    return result;
}
