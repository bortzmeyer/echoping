dnl $Id$

m4_include([../../configure-common.ac])
AM_INIT_AUTOMAKE(foreign)
AC_CONFIG_HEADERS(config.h)

m4_include([../configure-plugins.ac])

PGSQL_BUILD=1
AC_ARG_WITH(pgsql,
 [  --with-pgsql[=DIR]  PostgreSQL development library],dnl
 [if test "$withval" != "no"; then
   if test "$withval" != "yes"; then
        PGSQLROOT=$withval
        LDFLAGS="${LDFLAGS} -L$PGSQLROOT/lib"
        CPPFLAGS="${CPPFLAGS} -I$PGSQLROOT/include"  
   fi
  else
       PGSQL_BUILD=0 
 fi],
)
AC_CHECK_FUNCS(poptGetContext,, 
   [AC_CHECK_LIB(popt,poptGetContext,,[AC_MSG_WARN([No popt development environment found, not building this plugin]) && PGSQL_BUILD=0])])
if test "$PGSQL_BUILD" != "0"; then
AC_CHECK_FUNCS(PQconnectdb, , 
   [AC_CHECK_LIB(pq,PQconnectdb,,
      [AC_MSG_WARN([No PostgreSQL development environment found, not building this plugin]) && PGSQL_BUILD=0])])
fi
AC_CHECK_HEADER([libpq-fe.h],,
	[AC_CHECK_HEADER([postgresql/libpq-fe.h],AC_DEFINE(POSTGRESQL_PREFIX,1,[Define if you need to include postgresql/libpq-fe.h, not just libpq-fe.h]),
		[AC_MSG_WARN([No PostgreSQL header found, not building this plugin]) && PGSQL_BUILD=0])])
AM_CONDITIONAL(PGSQLBUILD, test "$PGSQL_BUILD" = 1)
AC_OUTPUT(Makefile)

