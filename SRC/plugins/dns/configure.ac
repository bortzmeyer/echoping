dnl $Id$

AC_INIT(dns.c)
m4_include([../../configure-common.ac])
AC_CONFIG_HEADERS(config.h)

m4_include([../configure-plugins.ac])

DNS_BUILD=1
AC_CHECK_FUNCS(poptGetContext,, 
   [AC_CHECK_LIB(popt,poptGetContext,,[AC_MSG_WARN([No popt development environment found, not building this plugin]) && DNS_BUILD=0])])
if test "$DNS_BUILD" != "0"; then
AC_CHECK_FUNCS(res_init, , 
dnl Some Unices has res_init but has it renamed in libc as something else so we
dnl must include <resolv.h> to get the redefinition. (Stolen from rsync)
dnl autoconf AC_CHECK_FUNCS does not allow headers to be easily included :-(
	[AC_MSG_CHECKING([$ac_func again by including <resolv.h>])
	 LIBS=-lresolv AC_TRY_LINK([#include <resolv.h>
                     ],
		[$ac_func();],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_WARN([No DNS resolver development environment found, not building this plugin]) && DNS_BUILD=0]
         	)])
fi

AM_CONDITIONAL(DNSBUILD, test "$DNS_BUILD" = 1)
AC_OUTPUT(Makefile)

