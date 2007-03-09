dnl $Id$

m4_include([../../configure-common.ac])
AM_INIT_AUTOMAKE(foreign)
AC_CONFIG_HEADERS(config.h)

m4_include([../configure-plugins.ac])

DNS_BUILD=1
AC_CHECK_FUNCS(poptGetContext,, 
   [AC_CHECK_LIB(popt,poptGetContext,,[AC_MSG_WARN([No popt development environment found, not building this plugin]) && DNS_BUILD=0])])
if test "$DNS_BUILD" != "0"; then

 AC_CHECK_FUNC(res_mkquery, , 
  [AC_CHECK_LIB(bind, res_mkquery, , 
   [AC_CHECK_LIB(resolv, res_mkquery, , 
dnl http://bugs.debian.org/254089
	[ saved_LIBS="$LIBS"
	LIBS="$LIBS -lresolv"
	AC_MSG_CHECKING(for res_mkquery in -lresolv again)
	AC_LINK_IFELSE([[
#include <resolv.h>
int main()
{
	res_mkquery (0, 0, 0, 0, 0, 0, 0, 0, 0);
	return 0;
}]],
	LIBS="$saved_LIBS -lresolv"; AC_MSG_RESULT(yes),
	LIBS="$saved_LIBS"; AC_MSG_RESULT(no)
        [AC_MSG_WARN([No DNS resolver development environment found, not building this plugin]) && DNS_BUILD=0])])])])
fi

AC_MSG_CHECKING([_res ])
AC_TRY_LINK([#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>],
              [_res.options = RES_INIT;],
                 AC_DEFINE(HAVE_RES, , [Define if you have the _res variable]) 
                 AC_MSG_RESULT(yes), 
               AC_MSG_RESULT(no));
AC_MSG_CHECKING([_res_ext ])
AC_TRY_LINK([#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>],
              [_res_ext.nsaddr_list[0].ss_family = AF_INET6;],
                 AC_DEFINE(HAVE_RES_EXT, , [Define if you have the _res_ext variable (for IPv6)]) 
                 AC_MSG_RESULT(yes), 
               AC_MSG_RESULT(no));

AM_CONDITIONAL(DNSBUILD, test "$DNS_BUILD" = 1)
AC_OUTPUT(Makefile)

