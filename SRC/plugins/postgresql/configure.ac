dnl $Id$

AC_INIT(postgresql.c)
m4_include([../../configure-common.ac])
AC_CONFIG_HEADERS(config.h)

m4_include([../configure-plugins.ac])

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

