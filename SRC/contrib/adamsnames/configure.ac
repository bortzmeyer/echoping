dnl $Id$

AC_INIT(adamsnames, 0.0-BETA)
AC_PROG_CC(cc gcc)
AC_PROG_LIBTOOL
AM_INIT_AUTOMAKE(foreign)
AC_CONFIG_HEADERS(config.h)

AC_CHECK_PROG(ADAMSNAMES_BUILD, xmlrpc-c-config, 1, 0)
if test "$ADAMSNAMES_BUILD" = 1; then
   LIBS="${LIBS} `xmlrpc-c-config libwww-client --libs`"
   CCFLAGS="${CCFLAGS} `xmlrpc-c-config libwww-client --cflags`"
else
   AC_ERROR([No XML-RPC C developement environment found, get it from http://xmlrpc-c.sourceforge.net/])
fi
AM_CONDITIONAL(ADAMSNAMESBUILD, test "$ADAMSNAMES_BUILD" = 1)
AC_OUTPUT(Makefile)
