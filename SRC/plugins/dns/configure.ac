dnl $Id$

AC_INIT(dns.c)
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

AC_ARG_WITH(popt,
 [--with-popt[=DIR]  popt command-line parsing library],dnl
 [if test "$withval" != "no"; then
   if test "$withval" != "yes"; then
        POPTROOT=$withval
        LDFLAGS="${LDFLAGS} -L$POPTROOT/lib"
        CPPFLAGS="${CPPFLAGS} -I$POPTROOT/include"  
   fi
  else
        AC_MSG_ERROR([The popt command-line parsing library is mandatory for this plugin])
 fi],
)

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

AC_SUBST(DNS_BUILD,$DNS_BUILD)
AC_OUTPUT(Makefile)

