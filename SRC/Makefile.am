bin_PROGRAMS = echoping
HEADERS = echoping.h icp.h HTParse.h
echoping_SOURCES = echoping.c error.c readline.c writen.c util.c http.c icp.c HTParse.c $(HEADERS)
echoping_LDADD = 

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

