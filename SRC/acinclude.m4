dnl Macros beginning with CF_ stolen from Lynx
dnl Thanks to "T.E.Dickey" <dickey@clark.net>

dnl ---------------------------------------------------------------------------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
changequote(,)dnl
$1=`echo $2 | tr '[a-z]' '[A-Z]'`
changequote([,])dnl
])dnl

dnl ---------------------------------------------------------------------------
dnl Check for existence of external data in the current set of libraries.  If
dnl we can modify it, it's real enough.
dnl $1 = the name to check
dnl $2 = its type
AC_DEFUN([CF_CHECK_EXTERN_DATA],
[
AC_MSG_CHECKING(if external $1 exists)
AC_CACHE_VAL(cf_cv_have_$1,[
    AC_TRY_LINK([
#undef $1
extern $2 $1;
],
    [$1 = 2],
    [eval 'cf_cv_have_'$1'=yes'],
    [eval 'cf_cv_have_'$1'=no'])])

eval 'cf_result=$cf_cv_have_'$1
AC_MSG_RESULT($cf_result)

if test "$cf_result" = yes ; then
    eval 'cf_result=HAVE_'$1
    CF_UPPER(cf_result,$cf_result)
    AC_DEFINE_UNQUOTED($cf_result)
fi

])dnl

dnl ---------------------------------------------------------------------------
dnl Check for data that is usually declared in <stdio.h> or <errno.h>, e.g.,
dnl the 'errno' variable.  Define a DECL_xxx symbol if we must declare it
dnl ourselves.
dnl
dnl (I would use AC_CACHE_CHECK here, but it will not work when called in a
dnl loop from CF_SYS_ERRLIST).
dnl
dnl $1 = the name to check
AC_DEFUN([CF_CHECK_ERRNO],
[
AC_MSG_CHECKING(if external $1 is declared)
AC_CACHE_VAL(cf_cv_dcl_$1,[
    AC_TRY_COMPILE([
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include <errno.h> ],
    [long x = (long) $1],
    [eval 'cf_cv_dcl_'$1'=yes'],
    [eval 'cf_cv_dcl_'$1'=no]')
])
eval 'cf_result=$cf_cv_dcl_'$1
AC_MSG_RESULT($cf_result)

if test "$cf_result" = no ; then
    eval 'cf_result=DECL_'$1
    CF_UPPER(cf_result,$cf_result)
    AC_DEFINE_UNQUOTED($cf_result)
fi

# It's possible (for near-UNIX clones) that the data doesn't exist
CF_CHECK_EXTERN_DATA($1,int)
])dnl

dnl ---------------------------------------------------------------------------
dnl Check for declaration of sys_nerr and sys_errlist in one of stdio.h and
dnl errno.h.  Declaration of sys_errlist on BSD4.4 interferes with our
dnl declaration.  Reported by Keith Bostic.
AC_DEFUN([CF_SYS_ERRLIST],
[
for cf_name in sys_nerr sys_errlist
do
    CF_CHECK_ERRNO($cf_name)
done
])dnl

dnl Useful macros to check libraries which are not implicit 
dnl in Solaris, for instance.
AC_DEFUN([CF_LIB_NSL],
[
AC_CHECK_LIB(nsl,gethostbyname,
[
AC_MSG_CHECKING(if libnsl is mandatory)
AC_TRY_LINK([#include <sys/types.h>
             #include <netinet/in.h>
             char *domain;  ], 
 [gethostbyname(domain)], dnl
 [AC_MSG_RESULT(no)], dnl
 [AC_MSG_RESULT(yes); LIBS="${LIBS} -lnsl"])
]) 
])
AC_DEFUN([CF_LIB_SOCKET],
[
AC_CHECK_LIB(socket,socket,
[
AC_MSG_CHECKING(if libsocket is mandatory)
AC_TRY_LINK([#include <sys/types.h>
             #include <netinet/in.h>
             union
             {
                HEADER hdr;
                u_char buf[512];
             }
             response;
             char *domain;
             int requested_type;        ], 
 [socket (AF_INET, SOCK_STREAM, 0) ], dnl
 [AC_MSG_RESULT(no)], dnl
 [AC_MSG_RESULT(yes); LIBS="${LIBS} -lsocket"]) 
])
])


dnl Check the port name for HTTP. Everyone should declare "http" but
dnl not everyone does. this test is not perfect, we should use a program
dnl which calls getservbyname() otherwise we miss NIS tables, for
dnl instance.
AC_DEFUN([CF_CHECK_SERVICES],
[
AC_MSG_CHECKING(what is the name of the HTTP port in your services database)
dnl We should test it is really port 80 and not any mention of "http"
if grep http /etc/services > /dev/null; then
    AC_DEFINE(HTTP_TCP_PORT,"http")
    AC_MSG_RESULT(http)
else
    if grep www /etc/services > /dev/null; then
	AC_DEFINE(HTTP_TCP_PORT,"www")
        AC_MSG_RESULT(www)	
    else
	AC_DEFINE(HTTP_TCP_PORT,"undefined:use_:80")
	AC_MSG_RESULT([undefined, you should add it in your database])
    fi
fi
AC_MSG_CHECKING(what is the name of the ICP port in your services database)
if grep icp /etc/services > /dev/null; then
    AC_DEFINE(ICP_UDP_PORT,"icp")
    AC_MSG_RESULT(icp)
else
    AC_DEFINE(ICP_UDP_PORT,"undefined:use_:3130")
    AC_MSG_RESULT([undefined, you should add it in your database])
fi
])

AC_DEFUN([CF_CHECK_HTTP_SERVICE],
[
AC_MSG_CHECKING(what is the name of the HTTP port in your services database)
AC_TRY_RUN([
int 
main (argc, argv)
     int argc;
     char *argv[];
{
  struct servent *sp;
  if ((sp = getservbyname ("http", "tcp")) == 0)
	exit (1);
  else
	exit (0);
} 
],
[AC_DEFINE(HTTP_TCP_PORT,"http")
AC_MSG_RESULT(http)]
,)])
