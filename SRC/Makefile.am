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

debug: 
	$(MAKE) CFLAGS='-g -O0'

upload: dist
	@echo "Have you updated ChangeLog and tagged (cvs tag RELEASE_x_x_x) everything?"
	@echo "If so, make upload-force"

upload-force: dist
	scp $(distdir).tar.gz  bortz@ftp.internatif.org:'~ftp/pub/unix/echoping'
	rm -f upload.log
	ncftpput -d upload.log upload.sourceforge.net /incoming $(distdir).tar.gz

EXTRA_DIST = $(man_MANS) $(TESTS) $(DOCS) $(MORE_TESTS)

## Otherwise, we depend on GNUmake, but for CVS users, not for regular
## tarball users
##AUTOMAKE_OPTIONS = no-dependencies



