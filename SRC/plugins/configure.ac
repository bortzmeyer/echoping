dnl $Id$

AC_INIT(Makefile.plugins.in)
m4_include(../configure-common.ac)
dnl TODO: allow to exclude or include plugins
dnl TODO: writes a LDAP plugin
AC_CONFIG_SUBDIRS(random whois dns postgresql)
plugins_dir=`cat dir`
AC_SUBST(plugins_dir)
AC_OUTPUT(Makefile.plugins Makefile)

