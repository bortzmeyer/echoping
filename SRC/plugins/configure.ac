dnl $Id$

m4_include(../configure-common.ac)
AM_INIT_AUTOMAKE(foreign)
dnl TODO: allow to exclude or include plugins
dnl TODO: writes a LDAP plugin
AC_CONFIG_SUBDIRS(random whois dns postgresql)
plugins_dir=`cat dir`
AC_SUBST(plugins_dir)
AC_OUTPUT(Makefile.plugins Makefile)

