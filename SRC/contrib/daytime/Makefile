CCFLAGS=-Wall -O0 -g -fPIC
LDFLAGS=-shared

OBJECTS=daytime.o

all: daytime.so

%.o: %.c
	${CC} ${CCFLAGS} -c -o $@ $<

%.so: %.o
	${CC} ${LDFLAGS} -o $@ $<

clean: 
	-rm -f *.o *.so 

.SECONDARY: ${OBJECTS}
