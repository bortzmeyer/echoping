dnl $Id$

AC_INIT(postgresql.c)
AM_INIT_AUTOMAKE(example, 6.0-BETA)
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

PGSQL_BUILD=1
AC_CHECK_FUNCS(poptGetContext,, 
   [AC_CHECK_LIB(popt,poptGetContext,,[AC_MSG_WARN([No popt development environment found, not building this plugin]) && PGSQL_BUILD=0])])
if test "$PGSQL_BUILD" != "0"; then
AC_CHECK_FUNCS(PQconnectdb, , 
   [AC_CHECK_LIB(pq,PQconnectdb,,
      [AC_MSG_WARN([No PostgreSQL development environment found, not building this plugin]) && PGSQL_BUILD=0])])
fi
AM_CONDITIONAL(PGSQLBUILD, test "$PGSQL_BUILD" = 1)
AC_OUTPUT(Makefile)

