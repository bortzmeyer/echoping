dnl Created automatically on Sat Jun 12 22:56:52 CEST 2004
if test "$dns_BUILD" = 1; then
  AC_CONFIG_SUBDIRS(dns)
fi
if test "$postgresql_BUILD" = 1; then
  AC_CONFIG_SUBDIRS(postgresql)
fi
if test "$random_BUILD" = 1; then
  AC_CONFIG_SUBDIRS(random)
fi
if test "$whois_BUILD" = 1; then
  AC_CONFIG_SUBDIRS(whois)
fi
