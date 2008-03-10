## Process this file with automake to produce Makefile.in

# $Id$

bin_PROGRAMS = echoping
man_MANS = echoping.1
echoping_SOURCES = echoping.c error.c readline.c writen.c util.c http.c icp.c HTParse.c echoping.h icp.h HTParse.h smtp.c
pkginclude_HEADERS = echoping.h compilation.h
echoping_LDADD = 
echoping_LDFLAGS = -export-dynamic
DOCS=DETAILS PLUGINS
SUBDIRS=plugins
SVN_REPO=http://echoping.svn.sourceforge.net/svnroot/echoping

TESTS=test-echoping-local test-echoping-remote 
MORE_TESTS=test-echoping-proxy test-echoping-icp test-echoping-crypto test-echoping-tos test-echoping-ipv6 test-echoping-idn test-echoping-plugins

test: tests
tests: check

# TODO: raise a warning at install-time when the prefix changes (just
# a warning because we do not want to prevent Debian packages for
# being built), otherwise, the plugins will not be found.

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

upload: 
	@echo "Have you updated configure-common.ac and ChangeLog and committed?"
	@echo "Then, have you ran \"make distcheck\" and tagged everything?"
	@echo "[Tag is 'svn copy -m \"TAG Release x.y.z\" ${SVN_REPO}/trunk ${SVN_REPO}/tags/RELEASE_x_y_z']"
	@echo "If so, make upload-force"

upload-force: dist
	rm -f upload.log
	ncftpput -d upload.log upload.sourceforge.net /incoming $(distdir).tar.gz

realclean: distclean
	rm -rf *~ autom4te.cache

EXTRA_DIST = $(man_MANS) $(TESTS) $(DOCS) $(MORE_TESTS)

## Otherwise, we depend on GNUmake, but for Subversion users, not for regular
## tarball users
##AUTOMAKE_OPTIONS = no-dependencies



