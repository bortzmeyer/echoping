XCFLAGS=-Wall -O0 -g -fPIC ${CFLAGS}
LDFLAGS=-shared

OBJECTS=daytime.o

all: daytime.so

%.o: %.c
	${CC} ${XCFLAGS} -c -o $@ $<

%.so: %.o
	${CC} ${LDFLAGS} -o $@ $<

clean: 
	-rm -f *.o *.so 

.SECONDARY: ${OBJECTS}
