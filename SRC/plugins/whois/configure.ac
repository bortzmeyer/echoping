dnl $Id$

AC_INIT(whois.c)
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

WHOIS_BUILD=1
AC_CHECK_FUNCS(poptGetContext,, 
   [AC_CHECK_LIB(popt,poptGetContext,,[AC_MSG_WARN([No popt development environment found, not building this plugin]) && WHOIS_BUILD=0])])
AC_SUBST(WHOIS_BUILD,$WHOIS_BUILD)
AC_OUTPUT(Makefile)

