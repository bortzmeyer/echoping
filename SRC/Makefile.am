## Process this file with automake to produce Makefile.in

# $Id$

bin_PROGRAMS = echoping
man_MANS = echoping.1
echoping_SOURCES = echoping.c error.c readline.c writen.c util.c http.c icp.c HTParse.c echoping.h icp.h HTParse.h smtp.c
echoping_LDADD = 
DOCS=DETAILS PLUGINS
SUBDIRS=plugins

# Export global symbols to plugins
AM_CFLAGS=-rdynamic

TESTS=test-echoping-local test-echoping-remote 
MORE_TESTS=test-echoping-proxy test-echoping-icp test-echoping-crypto test-echoping-tos test-echoping-ipv6 test-echoping-idn

test: check
tests: check

fulltest: test moretest

moretests: moretest
moretest: $(MORE_TESTS)
	@failed=0; all=0; \
	srcdir=$(srcdir); export srcdir; \
	for tst in $(MORE_TESTS); do \
	  if test -f $$tst; then dir=.; \
	  else dir="$(srcdir)"; fi; \
	  if $(TESTS_ENVIRONMENT) $$dir/$$tst; then \
	    all=`expr $$all + 1`; \
	    echo "PASS: $$tst"; \
	  elif test $$? -ne 77; then \
	    all=`expr $$all + 1`; \
	    failed=`expr $$failed + 1`; \
	    echo "FAIL: $$tst"; \
	  fi; \
	done; \
	if test "$$failed" -eq 0; then \
	  banner="All $$all tests passed"; \
	else \
	  banner="$$failed of $$all tests failed"; \
	fi; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"; \
	test "$$failed" -eq 0

internal_test: internal_tests
internal_tests: internal_tests.o $(echoping_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $< util.o 
	./internal_tests

debug: 
	$(MAKE) DEBUG=1 CFLAGS='-g -O0'

upload: dist
	@echo "Have you updated ChangeLog and tagged (cvs tag RELEASE_x_x_x) everything?"
	@echo "If so, make upload-force"

upload-force: dist
	scp $(distdir).tar.gz  bortzmeyer@ftp.internatif.org:'~ftp/pub/unix/echoping'
	rm -f upload.log
	ncftpput -d upload.log upload.sourceforge.net /incoming $(distdir).tar.gz

realclean: distclean
	rm -rf *~ autom4te.cache

EXTRA_DIST = $(man_MANS) $(TESTS) $(DOCS) $(MORE_TESTS)

## Otherwise, we depend on GNUmake, but for CVS users, not for regular
## tarball users
##AUTOMAKE_OPTIONS = no-dependencies



