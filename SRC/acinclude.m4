dnl $Id$

dnl Macros beginning with CF_ (mostly) stolen from Lynx
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
AC_DEFUN([CF_LIB_MATH],
[
AC_CHECK_LIB(m,pow,
[
AC_MSG_CHECKING(if libmath is mandatory)
AC_TRY_LINK([#include <math.h>
             double a,b;  ], 
 [pow(a,b)], dnl
 [AC_MSG_RESULT(no)], dnl
 [AC_MSG_RESULT(yes); LIBS="${LIBS} -lm"])
]) 
])

dnl Check the port name for HTTP. Everyone should declare "http" but
dnl not everyone does. This test is BUGgy, we should use a program
dnl which calls getservbyname() otherwise we miss NIS tables, for
dnl instance.
AC_DEFUN([CF_CHECK_SERVICES],
[
AC_MSG_CHECKING(what is the name of the HTTP port in your services database)
dnl BUG: We should test it is really the good port and not any mention of "http"
if grep http /etc/services > /dev/null; then
    AC_DEFINE(HTTP_TCP_PORT,"http")
    AC_MSG_RESULT(http)
else
    dnl BUG: Trap on Solaris with a port whose name begins with "www"...
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

# Check GNU libidn
# TODO: check the patched libc with AI_IDN. See libidn, in libc/getaddrinfo-idn.txt.
AC_DEFUN([CF_LIB_LIBIDN],
[
AC_CHECK_LIB(idn,idna_to_ascii_8z,
[LIBS="${LIBS} -lidn"],
[AC_ERROR([Get the GNU libidn library (http://www.josefsson.org/libidn/) in order to use Unicode - multi-script - domain names or use --without-libidn to disable it])], dnl
)])

# Check OpenSSL
AC_DEFUN([CF_LIB_OPENSSL],
[
AC_CHECK_LIB(ssl,SSL_CTX_new,
[LIBS="${LIBS} -lssl -lcrypto"],
[AC_ERROR([Get the OpenSSL library (http://www.openssl.org/)])], dnl
-lcrypto
)])

# Check GNU TLS
AC_DEFUN([CF_LIB_GNUTLS],
[
AC_CHECK_LIB(gnutls,gnutls_global_init,
[LIBS="${LIBS} `libgnutls-config --libs`"],
[AC_ERROR([Get the GNU TLS library (http://www.gnutls.org/)])], dnl
)])

dnl experimental
AC_DEFUN([CF_CHECK_TCP_SERVICE],
[
AC_TRY_RUN([
#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/types.h>
#include        <netdb.h>
#include        <sys/socket.h>
#include        <netinet/in.h>
#include        <arpa/inet.h>
#include        <unistd.h>
int 
main (argc, argv)
     int argc;
     char *argv[];
{
  struct servent *sp;
  if ((sp = getservbyname ("$1", "tcp")) == 0)
	exit (1);
  else
	exit (0);
} 
],
ac_last_port=$1
,
ac_last_port=
)])

dnl Our (W. Richard Stevens') macro to check for a function prototype in 
dnl a given header.  
AC_DEFUN([AC_CHECK_FUNC_PROTO],
        [AC_CACHE_CHECK(for $1 function prototype in $2, ac_cv_have_$1_proto,
                AC_EGREP_HEADER($1, $2,
                        ac_cv_have_$1_proto=yes,
                        ac_cv_have_$1_proto=no))
        if test $ac_cv_have_$1_proto = yes ; then
                ac_tr_func=HAVE_`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`_PROTO
                AC_DEFINE_UNQUOTED($ac_tr_func)
        fi
])

dnl BROKEN: do not use
dnl Copied from autoconf and edited to add an argument: an include file
dnl AC_CHECK_FUNC_WITH_HEADER(FUNCTION, HEADER, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN([AC_CHECK_FUNC_WITH_HEADER],
[AC_MSG_CHECKING([for $1])
AC_CACHE_VAL(ac_cv_func_$1,
[AC_TRY_LINK(
dnl Don't include <ctype.h> because on OSF/1 3.0 it includes <sys/types.h>
dnl which includes <sys/select.h> which contains a prototype for
dnl select.  Similarly for bzero.
[/* System header to define __stub macros and hopefully few prototypes,
    which can conflict with char $1(); below.  */
#include <assert.h>
/* Override any gcc2 internal prototype to avoid an error.  */
]ifelse(AC_LANG, CPLUSPLUS, [#ifdef __cplusplus
extern "C"
#endif
])dnl
[/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char $1();
#include <$2>
], [
/* The GNU C library defines this for functions which it implements
    to always fail with ENOSYS.  Some functions are actually named
    something starting with __ and the normal name is an alias.  */
#if defined (__stub_$1) || defined (__stub___$1)
choke me
#else
$1();
#endif
], eval "ac_cv_func_$1=yes", eval "ac_cv_func_$1=no")])
if eval "test \"`echo '$ac_cv_func_'$1`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$3], , :, [$3])
else
  AC_MSG_RESULT(no)
ifelse([$4], , , [$4
])dnl
fi
])


