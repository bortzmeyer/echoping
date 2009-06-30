dnl $Id$

dnl Process this file with autoconf to produce a configure script.

m4_include(configure-common.ac)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

dnl Checking size of ints, we often need it for binary protocols like ICP
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)

dnl User options
dnl http://sources.redhat.com/ml/automake-prs/2001-q3/msg00033.html
max_iterations_specified=0
AC_ARG_ENABLE(max-iterations,
[  --enable-max-iterations=MAX   limit the number of iterations (-n option) to MAX],
[if test "$enableval" != "no"; then
   max_iterations_specified=1
fi])
if test "$max_iterations_specified" = 0; then
   AC_DEFINE(MAX_ITERATIONS, 20, Maximum number of iterations)
else
   AC_DEFINE_UNQUOTED(MAX_ITERATIONS, $enableval, Maximum number of iterations)
fi
max_line_specified=0
AC_ARG_ENABLE(max-request-size,
[  --enable-max-request-size=MAX   limit the size of a request to MAX],
[if test "$enableval" != "no"; then
   max_line_specified=1
fi])
if test "$max_line_specified" = 0; then
  AC_DEFINE(MAX_LINE, 65535, Maximum size of a request)
else
   AC_DEFINE_UNQUOTED(MAX_LINE, $enableval, Maximum size of a request)
fi
AC_ARG_ENABLE(http,
 [  --enable-http   HTTP (Web's main protocol) support],dnl
 [if test "$enableval" = "yes"; then
 AC_DEFINE(HTTP,,[HTTP support])
 HTTP=1
 fi],
 dnl Default: enable it
 [AC_DEFINE(HTTP,,[HTTP support])
  HTTP=1])
AC_ARG_ENABLE(icp,
 [  --enable-icp   ICP (for testing Web proxies/caches) support],dnl
 [if test "$enableval" = "yes"; then
 AC_DEFINE(ICP,,[ICP support])
 ICP=1
 fi])
AC_ARG_ENABLE(smtp,
 [  --enable-smtp   SMTP (Mail's main protocol) support],dnl
 [if test "$enableval" = "yes"; then
 AC_DEFINE(SMTP,,[SMTP support])
 SMTP=1
 fi],
 dnl Default: enable it
 [AC_DEFINE(SMTP,,[SMTP support])
  SMTP=1])
dnl IDN
dnl Default: enable it	
LIBIDN=1
AC_ARG_WITH(libidn,
 [  --with-libidn[=DIR]   Internationalized Domain Names support (needs GNU libidn)],dnl
 [if test "$withval" != "no"; then
   if test "$withval" != "yes"; then
	IDNROOT=$withval
	LDFLAGS="${LDFLAGS} -L$IDNROOT/lib"
        CPPFLAGS="${CPPFLAGS} -I$IDNROOT/include"  
   fi
 else
   LIBIDN=0
 fi],
)
if test "$LIBIDN" = "1"; then
AC_DEFINE(LIBIDN,,[Internationalized Domain Names support])
fi
AC_ARG_WITH(ssl,
 [  --with-ssl[=DIR]   SSL crypt support (needs OpenSSL)],dnl
 [if test "$withval" != "no"; then
   AC_DEFINE(OPENSSL,,[Crypto (SSL) support])
   OPENSSL=1
   if test "$withval" != "yes"; then
	SSLROOT=$withval
	LDFLAGS="${LDFLAGS} -L$SSLROOT/lib"
        CPPFLAGS="${CPPFLAGS} -I$SSLROOT/include"  
   fi
 fi],
 dnl Default: disable it
)
AC_ARG_WITH(gnutls,
 [  --with-gnutls[=DIR]   SSL/TLS crypt support (needs GNU TLS), the argument DIR should not be necessary],dnl
 [if test "$withval" != "no"; then
  if test "$withval" = "yes"; then
      PKG_CHECK_MODULES([LIBGNUTLS], [gnutls >= 1.0.0],
      [
      echo "GnuTLS found"
      AC_DEFINE(GNUTLS,,[Crypto (TLS) support])
      GNUTLS=1
      CFLAGS="$CFLAGS $LIBGNUTLS_CFLAGS"
      LIBS="$LIBS $LIBGNUTLS_LIBS"
      ],)
      else
      AC_DEFINE(GNUTLS,,[Crypto (TLS) support])
      GNUTLS=1
      GNUTLSROOT=$withval
      LDFLAGS="${LDFLAGS} -L$GNUTLSROOT/lib -lgnutls"
      CPPFLAGS="${CPPFLAGS} -I$GNUTLSROOT/include"
   fi
 fi],
 dnl Default: disable it 
) 
dnl TODO: test if the getopt variable optreset exists and, if it
dnl does, we can use getopt (Free BSDs and MacOS X). This implies to
dnl maintain the two versions, popt and getopt, while echoping has 
dnl many options. It probably means a high-level language to describe
dnl the options and to produce the two versions. gengetopt is an obvious
dnl candidate.
AC_ARG_WITH(popt,
 [  --with-popt[=DIR]  popt command-line parsing library],dnl
 [if test "$withval" != "no"; then
   if test "$withval" != "yes"; then
	POPTROOT=$withval
	LDFLAGS="${LDFLAGS} -L$POPTROOT/lib"
        CPPFLAGS="${CPPFLAGS} -I$POPTROOT/include"  
   fi
  else
	AC_MSG_ERROR([The popt command-line parsing library is mandatory for echoping])
 fi],
 dnl Default: disable it
)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/time.h unistd.h)

case $host_os in
osf*)
	# Stupid bug appeared in Tru64-OSF1 v5. socklen_t is undefined without
	# the following workaround. 
        CPPFLAGS="$CPPFLAGS -D_POSIX_PII_SOCKET"
        ;;
*darwin*)
 	# See bug #748145 and #765777
        CPPFLAGS="$CPPFLAGS -D_BSD_SOCKLEN_T_=int -no-cpp-precomp"
	;;
esac

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

dnl Checks for library functions.
CF_LIB_SOCKET
CF_LIB_NSL
CF_LIB_MATH
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
dnl Some Unices like Tru64 or Mac OS X has getaddrinfo() or
dnl getnameinfo() but has it renamed in libc as something else so we
dnl must include <netdb.h> to get the redefinition. (Stolen from rsync)
dnl autoconf AC_CHECK_FUNCS does not allow headers to be easily included :-(
AC_CHECK_FUNCS(getaddrinfo getnameinfo inet_ntop, ,
	[AC_MSG_CHECKING([$ac_func again by including <netdb.h>])
	AC_TRY_LINK([#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>],[$ac_func(NULL, NULL, NULL, NULL);],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_ERROR([Missing mandatory function - echoping now uses the new network functions (RFC 2133) which are mandatory for IPv6])]
         	)])
		
AC_CHECK_FUNCS(gettimeofday socket sigaction strerror, , AC_MSG_ERROR(Missing mandatory function))
AC_CHECK_FUNCS(poptGetContext, , 
   [AC_CHECK_LIB(popt,poptGetContext, ,
      [AC_MSG_ERROR([Missing popt library, get it from ftp://ftp.rpm.org/pub/rpm/dist/rpm-x.y.z])])])
AC_CHECK_FUNCS(usleep) dnl TODO: use the Autoconf macro http://www.gnu.org/software/ac-archive/htmldoc/etr_short_sleep.html
AC_CHECK_FUNCS(dlopen, , 
   [AC_CHECK_LIB(dl,dlopen, ,
       [AC_MSG_ERROR([echoping requires dlopen (dynamic loading of libraries) for plugins])])])

if test "$LIBIDN" = "1"; then
CF_LIB_LIBIDN
fi
if test "$OPENSSL" = "1" && test "$GNUTLS" = "1"; then
AC_MSG_ERROR([Choose OpenSSL or GNU TLS but not both])
fi
if test "$OPENSSL" = "1"; then
CF_LIB_OPENSSL
fi
if test "$GNUTLS" = "1"; then
CF_LIB_GNUTLS
fi

AC_MSG_CHECKING([TCP info from socket])
AC_TRY_COMPILE(
 [#include <sys/types.h>
  #include <sys/socket.h>
  /* TODO: including tcp.h may not be necessary */
  #include <netinet/tcp.h>
 ],
 [int foobar = SOL_TCP;],
 [AC_DEFINE(HAVE_TCP_INFO,,[TCP info from socket])
  ac_have_tcp_info="yes"],
 ac_have_tcp_info=no)
AC_MSG_RESULT($ac_have_tcp_info)
AC_ARG_ENABLE(tcp_info,
 [  --enable-tcp-info   TCP info from socket],
 [if test "$enableval" = "yes"; then
   if test $ac_have_tcp_info = "yes"; then
     AC_DEFINE(HAVE_TCP_INFO,,[TCP info from socket])
     TCP_INFO=1
   else
     AC_MSG_WARN([No TCP info support on this system, request ignored])
   fi
  fi],
 dnl Default: enable it if supported
 if test $ac_have_tcp_info = "yes"; then
   AC_DEFINE(HAVE_TCP_INFO,,[TCP info])
   TCP_INFO=1
 fi)

dnl Type Of Service
AC_MSG_CHECKING([Type Of Service])
AC_TRY_COMPILE(
 [#include <sys/types.h>
  #include <netinet/in.h>
 ],
 [int foobar = IP_TOS;],
 [AC_DEFINE(HAVE_TOS,,[Type Of Service support])
  ac_have_tos="yes"],
 ac_have_tos="no")
AC_MSG_RESULT($ac_have_tos)
AC_ARG_ENABLE(tos,
 [  --enable-tos   TOS (Type Of Service) support],
 [if test "$enableval" = "yes"; then
   if test $ac_have_tos = "yes"; then
     AC_DEFINE(HAVE_TOS,,[Type Of Service support])
     TOS=1
   else
     AC_MSG_WARN([No TOS support on this system, request ignored])
   fi
  fi],
 dnl Default: enable it if supported
 if test $ac_have_tos = "yes"; then
   AC_DEFINE(HAVE_TOS,,[Type Of Service support])
   TOS=1
 fi)

dnl Socket priority
dnl Linux only, it seems. Anyone knows a standard way to do so?
AC_MSG_CHECKING([Socket priority])
AC_TRY_COMPILE(
 [#include <sys/types.h>
  #include <sys/socket.h>
 ],
 [int foobar = SO_PRIORITY;],
 [AC_DEFINE(HAVE_SOCKET_PRIORITY)
  ac_have_priority="yes"],
 ac_have_priority="no")
AC_MSG_RESULT($ac_have_priority)
AC_ARG_ENABLE(priority,
 [  --enable-priority   PRIORITY (socket priority) support],
 [if test "$enableval" = "yes"; then
   if test $ac_have_priority = "yes"; then
     AC_DEFINE(HAVE_SOCKET_PRIORITY,,[Socket priority support])
     PRIORITY=1
   else
     AC_MSG_WARN([No socket priority support on this system, request ignored])
   fi
  fi],
 dnl Default: enable it if supported
 if test $ac_have_priority = "yes"; then
   AC_DEFINE(HAVE_SOCKET_PRIORITY,,[Socket priority support])
   PRIORITY=1
 fi)

dnl SCTP, RFC 4960, feature request #1676608
AC_MSG_CHECKING([SCTP])
AC_TRY_COMPILE(
 [#include <sys/types.h>
  #include <netinet/in.h>
 ],
 [int foobar = IPPROTO_SCTP;],
 [AC_DEFINE(HAVE_SCTP,,[SCTP support])
  ac_have_sctp="yes"],
 ac_have_sctp="no")
AC_MSG_RESULT($ac_have_sctp)
AC_ARG_ENABLE(sctp,
 [  --enable-sctp   SCTP (Stream Control Transmission Protocol) support],
 [if test "$enableval" = "yes"; then
   if test $ac_have_sctp = "yes"; then
     AC_DEFINE(HAVE_SCTP,,[Stream Control Transmission Protocol support])
     SCTP=1
   else
     AC_MSG_WARN([No SCTP support on this system, request ignored])
   fi
  fi],
 dnl Default: enable it if supported
 if test $ac_have_sctp = "yes"; then
   AC_DEFINE(HAVE_SCTP,,[Stream Control Transmission Protocol support])
   SCTP=1
 fi)

if test "$GCC" = yes; then
        CFLAGS="$CFLAGS -Wall"
fi

interpolate() {  
  old=$1
  eval new=$old
  if test "$new" != "$old" ; then
	interpolate $new
  else 
     echo $new
  fi
}
dnl TODO: the following is broken because installation prefix can be changed at instal-time,
dnl not compilation time.
if test "x$libdir" = 'x${exec_prefix}/lib' ; then # Default value
	if test "x$exec_prefix" = "xNONE" ; then # Default value
		if test "x$prefix" = "xNONE" ; then # Default value
  			plugins_dir=`interpolate '$ac_default_prefix/lib/echoping'`
		else
  			exec_prefix='${prefix}'	
  			plugins_dir=`interpolate '$libdir/echoping'`
		fi
	else
  		plugins_dir=`interpolate '$libdir/echoping'`
	fi
else
	plugins_dir=`interpolate '$libdir/echoping'`
fi
AC_DEFINE_UNQUOTED(PLUGINS_DIR, "$plugins_dir", [Directory where the plugins will be installed])
AC_SUBST(plugins_dir)
# TODO: make distcheck complains here that "plugins" does not exist???
echo $plugins_dir > plugins/dir
AC_CONFIG_SUBDIRS(plugins)

compil_options=""
AC_DEFUN([DISPLAY_SETTING],
[
echo $ECHO_N $1 ": "
compil_options="$compil_options\n$1:"
if [ eval 'test "$$1" = "1"' > /dev/null]; then
	AC_MSG_RESULT( enabled)
	compil_options="$compil_options enabled"	
else 
	AC_MSG_RESULT( disabled)
	compil_options="$compil_options disabled "
fi
])dnl
echo ""
echo "Configuration of echoping:"

DISPLAY_SETTING(HTTP)
DISPLAY_SETTING(ICP)
DISPLAY_SETTING(OPENSSL)
DISPLAY_SETTING(GNUTLS)
DISPLAY_SETTING(SMTP)
DISPLAY_SETTING(LIBIDN)
DISPLAY_SETTING(TOS)
DISPLAY_SETTING(SCTP)
DISPLAY_SETTING(PRIORITY)

compil_date=`date +%Y-%m-%d`
hostname=$ac_hostname
AC_SUBST(hostname)
AC_SUBST(compil_options)
AC_SUBST(compil_date)
AC_OUTPUT(Makefile echoping.1 compilation.h)






