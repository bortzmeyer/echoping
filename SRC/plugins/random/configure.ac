dnl $Id$

AC_INIT(random.c)
m4_include(../../configure-common.ac)
AC_CONFIG_HEADERS(config.h)

RANDOM_BUILD=1

AC_CHECK_FUNCS(usleep,, [AC_MSG_WARN([No usleep() found, not building this plugin]) && RANDOM_BUILD=0])
AC_SUBST(RANDOM_BUILD,$RANDOM_BUILD)
AC_OUTPUT(Makefile)

