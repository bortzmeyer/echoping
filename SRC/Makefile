# Choose your C compiler
CC = gcc 
#CC = cc 

# Possible options:
# HTTP : supports the HTTP protocol (Add HTTP10 is for the old HTTP 1.0 protocol,
#        HTTP 0.9 is not supported)
# TTCP : supports the T/TCP protocol (few systems support it)
# ICP : supports the ICP protocol (Web proxy/caches). Requires HTTP.
# USE_SIGACTION: uses sigaction instead of signal. Necessary on pure BSD
# machines because we need to change the semantic of signals.
OPTIONS =  -DHTTP -DUSE_SIGACTION

# Flags for gcc
CFLAGS = -c  -O3 $(OPTIONS) -Wall
# Flags for cc
#CFLAGS = -c -O $(OPTIONS)

# Flags for the linker
LD = $(CC) 
LDFLAGS = -o echoping 
# For Solaris
#LDFLAGS = -o echoping -lsocket -lnsl

INSTALL=install
INSTALL_BIN_FLAGS=-m 755
INSTALL_MAN_FLAGS=-m 644

ROOT=/usr/local
DESTBIN=$(ROOT)/bin
DESTMAN=$(ROOT)/man/man1

########## Do not touch below this line #########

OBJS = echoping.o error.o readline.o writen.o util.o http.o icp.o HTParse.o
HEADERS = echoping.h icp.h HTParse.h
SOURCES = echoping.c error.c readline.c writen.c util.c http.c icp.c HTParse.c $(HEADERS)
MISC = README Makefile
DISTRIB= README INSTALL CHANGES DETAILS CREDITS Makefile echoping.ptk $(SOURCES) echoping.1 
VERSION=`grep VERSION echoping.h | cut -d ' ' -f 3 | sed s/\"//g`

all: echoping

echoping: $(OBJS) 
	@ echo Linking $@ with new $?
	$(LD) $(LDFLAGS) $(OBJS)

.c.o:    $(HEADERS)
	$(CC) $(CFLAGS) $<

clean:
	-rm echoping $(OBJS)
	@ echo Erased

distrib.old: 
	@(cd .. ; \
	tar cvf "echoping/echoping.tar" $(DISTRIB); \
	gzip -v -9 -f "echoping/echoping.tar"; \
	uuencode echoping/echoping.tar.gz echoping.tar.gz > echoping/echoping.tar.gz.uu)

distrib: 
	@(echo Echoping is version ${VERSION}; \
	mkdir echoping-${VERSION}; \
	cp $(DISTRIB) echoping-${VERSION};\
	tar cvf echoping-${VERSION}.tar echoping-${VERSION}; \
	rm -rf echoping-${VERSION}; \
	gzip -v -9 -f echoping-${VERSION}.tar; \
	uuencode echoping-${VERSION}.tar.gz echoping-${VERSION}.tar.gz > \
		echoping-${VERSION}.tar.gz.uu)

checkout:
	co -l $(SOURCES) README

install:  echoping
	$(INSTALL) $(INSTALL_BIN_FLAGS) echoping $(DESTBIN)
	$(INSTALL) $(INSTALL_MAN_FLAGS) echoping.1 $(DESTMAN)
