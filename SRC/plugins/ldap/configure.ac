dnl $Id$

m4_include([../../configure-common.ac])
AM_INIT_AUTOMAKE(foreign)
AC_CONFIG_HEADERS(config.h)

m4_include([../configure-plugins.ac])

LDAP_BUILD=1
AC_CHECK_FUNCS(poptGetContext,, 
   [AC_CHECK_LIB(popt,poptGetContext,,[AC_MSG_WARN([No popt development environment found, not building this plugin]) && LDAP_BUILD=0])])
if test "$LDAP_BUILD" != "0"; then
AC_CHECK_FUNCS(ldap_open, , 
   [AC_CHECK_LIB(ldap,ldap_open,,
      [AC_MSG_WARN([No LDAP development environment found, not building this plugin]) && LDAP_BUILD=0])])
fi
AM_CONDITIONAL(LDAPBUILD, test "$LDAP_BUILD" = 1)
AC_OUTPUT(Makefile)

