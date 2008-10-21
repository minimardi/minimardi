#############################################################################
#  If your compiler complains about 'strerror()', then add
#  "-DNO_STRERROR" to the COMPILEFLAGS below.

COMPILEFLAGS = -Wall -O2 

#  If you don't have zlib (needed for logging to .gz files), you can
#  get it at: http://www.gzip.org - if you don't want to install it,
#  you can just remove "-lz" from the LNFLAGS below.
#
#  Some systems may require you link against the socket and/or nsl 
#  library, so add "-lsocket" and "-lnsl" to the LNFLAGS below if
#  nessesary.

LNFLAGS = -lz 

#############################################################################
#  The rest should be just fine.
#############################################################################

VERSION = 1.5.0
BINNAME = icsdrone
RELNAME = icsDrone
DISTNAME = ${RELNAME}-${VERSION}

CFLAGS = -DPRODUCTNAME=\"${RELNAME}\" -DPRODUCTVERSION=\"${VERSION}\" $(COMPILEFLAGS)
CC = gcc
OBJS = utils.o argparser.o net.o computer.o fics.o main.o 

all: 	  main

clean:    
	rm -f *.o *~ ${BINNAME}

dist:   clean
	(cd .. && tar cvfz $(DISTNAME).tgz \
		${RELNAME}/COPYRIGHT \
		${RELNAME}/TODO \
		${RELNAME}/README \
		${RELNAME}/THANKS \
		${RELNAME}/ChangeLog \
		${RELNAME}/Makefile \
		${RELNAME}/*.[ch] \
		${RELNAME}/sample-script.sh \
		${RELNAME}/olipow-patch )

main: $(OBJS)
	$(CC) $(OBJS) $(LNFLAGS) -o ${BINNAME} -g
