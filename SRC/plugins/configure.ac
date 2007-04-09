dnl $Id$

m4_include(../configure-common.ac)
AM_INIT_AUTOMAKE(foreign)
dnl TODO: write a XML-RPC plugin for Adamsnames' interface. See a first version in contrib/
AC_ARG_ENABLE(plugin,
[  --enable-plugin=LIST  Build only the following plugins],
[if test "$enableval" = "no"; then
   AC_MSG_ERROR([You cannot disable plugin support])
else
   if test "$enableval" = "yes"; then
      AC_MSG_ERROR([You must indicate a list of plugins with this option])
   else
      plugins=$enableval
   fi
fi],
plugins="random whois dns postgresql ldap")
for plugin in $plugins; do
  if test ! -d $srcdir/$plugin ; then
    AC_MSG_ERROR([No such plugin $plugin])
  fi
  eval ${plugin}_BUILD=1
done
m4_include(list-of-dirs.m4)
plugins_dir=`cat dir`
AC_SUBST(plugins_dir)
AC_OUTPUT(Makefile)
echo ""
echo Plugins to build: $plugins
