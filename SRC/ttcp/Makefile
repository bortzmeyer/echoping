CC = gcc 
#CC = cc 
CFLAGS = -c -O
#CFLAGS = -g -c -O0
LD = $(CC) 
LDFLAGS = -o echoping

OBJS = echoping.o error.o readline.o writen.o util.o
SOURCES = echoping.c error.c readline.c writen.c util.c inet.h
MISC = README Makefile
DISTRIB= echo/README echo/Makefile echo/echoping.ptk echo/echoping.c echo/error.c echo/readline.c echo/writen.c echo/util.c echo/inet.h

all: echoping

echoping: $(OBJS) 
	@ echo Linking $@ with new $?
	$(LD) $(LDFLAGS) $(OBJS)

.c.o:   
	$(CC) $(CFLAGS) $<

clean:
	-rm echoping $(OBJS)
	@ echo Erased

distrib: 
	@(cd .. ; \
 	tar cvf "echo/echoping.tar" $(DISTRIB); \
 	gzip -v -f "echo/echoping.tar")

checkout:
	co -l $(SOURCES) README
	
