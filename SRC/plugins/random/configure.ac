dnl $Id$

AC_INIT(random.c)
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

RANDOM_BUILD=1

AC_CHECK_FUNCS(usleep,, [AC_MSG_WARN([No usleep() found, not building this plugin]) && RANDOM_BUILD=0])
AC_SUBST(RANDOM_BUILD,$RANDOM_BUILD)
AC_OUTPUT(Makefile)

