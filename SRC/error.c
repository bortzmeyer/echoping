#include "echoping.h"

/* $Id$ */

/* Most of error-handling routines stolen from Stevens' books */

void
my_perror()
{
    fprintf(stderr, " %s\n", sys_err_str());
}

/*
 * Recoverable error.  Print a message, and return to caller. 
 *
 * err_ret(str, arg1, arg2, ...) 
 *
 * The string "str" must specify the conversion specification for any args. 
 */

/* VARARGS1 */
void
err_ret(char *str, ...)
{
    va_list         args;

    va_start(args, str);
    vfprintf(stderr, str, args);
    va_end(args);

    my_perror();

    fflush(stdout);
    fflush(stderr);

    return;
}

/*
 * Fatal error.  Print a message and terminate. Don't dump core and don't
 * print the system's errno value. 
 *
 * err_quit(str, arg1, arg2, ...) 
 *
 * The string "str" must specify the conversion specification for any args. 
 */

/* VARARGS1 */
void
err_quit(char *str, ...)
{
    va_list         args;

    va_start(args, str);
    vfprintf(stderr, str, args);
    fputc('\n', stderr);
    va_end(args);

    exit(1);
}

/*
 * Fatal error related to a system call.  Print a message and terminate.
 * Don't dump core, but do print the system's errno value and its associated
 * message. 
 *
 * err_sys(str, arg1, arg2, ...) 
 *
 * The string "str" must specify the conversion specification for any args. 
 */

/* VARARGS1 */
void
err_sys(char *str, ...)
{
    va_list         args;

    va_start(args, str);
    vfprintf(stderr, str, args);
    va_end(args);

    my_perror();

    exit(1);
}

void
usage(poptContext context)
{
    poptPrintUsage(context, stderr, 0);
    fprintf(stderr, " hostname [plugin-options...]\n");
    exit(1);
}

/*
 * Return a string containing some additional operating-system dependent
 * information. Note that different versions of UNIX assign different
 * meanings to the same value of "errno" (compare errno's starting with 35
 * between System V and BSD, for example).  This means that if an error
 * condition is being sent to another UNIX system, we must interpret the
 * errno value on the system that generated the error, and not just send the
 * decimal value of errno to the other system. 
 */

char           *
sys_err_str()
{
    static char     msgstr[200];

    if (errno != 0) {
        sprintf(msgstr, "(%s)", strerror(errno));
    } else {
        msgstr[0] = '\0';
    }
    return (msgstr);
}
