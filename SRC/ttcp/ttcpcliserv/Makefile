# Change the following as required:
CC	= gcc

CFLAGS	= -Wall
#CFLAGS	= -ansi -Wall -Dsun -D__STDC__=0 -DMSG_EOF=0
# My flags for gcc/solaris 2.3:  -ansi -Wall -Dsun -D__STDC__=0
#   (Solaris -DMSG_EOF just to let me compile it under Solaris; it won't run)
# Add in -DGCC_STRUCT_PROBLEM for gcc versions 1.x under SunOS 4.x
# Add in -D__STDC__=0 for gcc under Solaris 2 (for Sun's screwy headers)
# Add in -D_BSD=44 for AIX 3.2.2 (see <sys/socket.h>)
# Add in -D_SOCKADDR_LEN for DEC OSF/1 (see <sys/socket.h>)

# Following line for SVR4, Solaris 2.x; 2.5 no longer requires libucb.a.
#LIBS	= ./libmisc.a /usr/ucblib/libucb.a -lsocket -lnsl

# Following line for 4.4BSD, BSD/386, SunOS 4.x, AIX 3.2.2, OSF/1
LIBS	= ./libmisc.a

PROGS =	udpcli udpserv tcpcli tcpserv ttcpcli ttcpserv \
	ttcpclibig ttcpservbig ttcphttpcli \
	udpclitime udpservtime tcpclitime tcpservtime ttcpclitime ttcpservtime

all:	${PROGS}

udpcli:		udpcli.o
		${CC} ${CCFLAGS} -o $@ udpcli.o ${LIBS}

udpserv:	udpserv.o
		${CC} ${CCFLAGS} -o $@ udpserv.o ${LIBS}

tcpcli:		tcpcli.o readstream.o
		${CC} ${CCFLAGS} -o $@ tcpcli.o readstream.o ${LIBS}

tcpserv:	tcpserv.o readstream.o
		${CC} ${CCFLAGS} -o $@ tcpserv.o readstream.o ${LIBS}

ttcpcli:	ttcpcli.o readstream.o
		${CC} ${CCFLAGS} -o $@ ttcpcli.o readstream.o ${LIBS}

ttcpserv:	ttcpserv.o readstream.o
		${CC} ${CCFLAGS} -o $@ ttcpserv.o readstream.o ${LIBS}

ttcpclibig:	ttcpclibig.o readstream.o
		${CC} ${CCFLAGS} -o $@ ttcpclibig.o readstream.o ${LIBS}

ttcpservbig:	ttcpservbig.o readstream.o
		${CC} ${CCFLAGS} -o $@ ttcpservbig.o readstream.o ${LIBS}

ttcphttpcli:	ttcphttpcli.o readstream.o
		${CC} ${CCFLAGS} -o $@ ttcphttpcli.o readstream.o ${LIBS}

udpclitime:	udpclitime.o timer.o
		${CC} ${CCFLAGS} -o $@ udpclitime.o timer.o ${LIBS}

udpservtime:	udpservtime.o timer.o
		${CC} ${CCFLAGS} -o $@ udpservtime.o timer.o ${LIBS}

tcpclitime:	tcpclitime.o readstream.o timer.o
		${CC} ${CCFLAGS} -o $@ tcpclitime.o readstream.o timer.o ${LIBS}

tcpservtime:	tcpservtime.o readstream.o timer.o
		${CC} ${CCFLAGS} -o $@ tcpservtime.o readstream.o timer.o ${LIBS}

ttcpclitime:	ttcpclitime.o readstream.o timer.o
		${CC} ${CCFLAGS} -o $@ ttcpclitime.o readstream.o timer.o ${LIBS}

ttcpservtime:	ttcpservtime.o readstream.o timer.o
		${CC} ${CCFLAGS} -o $@ ttcpservtime.o readstream.o timer.o ${LIBS}

clean:
		rm -f ${PROGS} core core.* *.o temp.* *.out typescript*
