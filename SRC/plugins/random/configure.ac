dnl $Id$

AC_INIT(random.c)
AM_INIT_AUTOMAKE(example, 6.0-BETA)
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

RANDOM_BUILD=1

AC_CHECK_FUNCS(usleep,, [AC_MSG_WARN([No usleep() found, not building this plugin]) && RANDOM_BUILD=0])
AC_SUBST(RANDOM_BUILD,$RANDOM_BUILD)
AC_OUTPUT(Makefile)

