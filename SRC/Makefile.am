## Process this file with automake to produce Makefile.in

# $Id$

bin_PROGRAMS = echoping
man_MANS = echoping.1
echoping_SOURCES = echoping.c error.c readline.c writen.c util.c http.c icp.c HTParse.c echoping.h icp.h HTParse.h smtp.c
echoping_LDADD = 
DOCS=DETAILS

TESTS=test-echoping-local test-echoping-remote test-echoping-icp
MORE_TESTS=test-echoping-crypto 

test: check
tests: check

EXTRA_DIST = $(man_MANS) $(TESTS) $(DOCS) $(MORE_TESTS)

## Otherwise, we depend on GNUmake, but for CVS users, not for regular
## tarball users
##AUTOMAKE_OPTIONS = no-dependencies



