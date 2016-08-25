#binrary program
CC=gcc
CFLAGS = -g
PROJ_NAME:=test
PROJ_OBJS:=test.o  \
		   cmn.o \
		   convert_fun.o \
	       convrnx.o \
	       decode.o \
	       ephemeris.o \
	       HRCCoder.o \
	       novatel.o \
	       pntpos.o \
	       rinex.o \
	       rt17.o \
	       rtcm.o \
	       rtcm3e.o \
	       ub370_e.o 

edit: $(PROJ_OBJS)
	gcc -g -o test $(PROJ_OBJS) -lm

test.o: test.c
	gcc -g -c test.c

cmn.o: cmn.c
	gcc -g -c cmn.c

convert_fun.o: convert_fun.c
	gcc -g -c convert_fun.c

convrnx.o: convrnx.c
	gcc -g -c convrnx.c

decode.o: decode.c
	gcc -g -c decode.c

ephemeris.o: ephemeris.c
	gcc -g -c ephemeris.c

HRCCoder.o: HRCCoder.c
	gcc -g -c HRCCoder.c

novatel.o: novatel.c
	gcc -g -c novatel.c

pntpos.o: pntpos.c
	gcc -g -c pntpos.c

rinex.o: rinex.c
	gcc -g -c rinex.c

rt17.o: rt17.c
	gcc -g -c rt17.c

rtcm.o: rtcm.c
	gcc -g -c rtcm.c

rtcm3e.o: rtcm3e.c
	gcc -g -c rtcm3e.c

ub370_e.o: ub370_e.c
	gcc -g -c ub370_e.c





#test: PROJ_OBJS
#	$(CC) -o $@  $(PROJ_OBJS) $(LDFLAGS) 

clean:
	rm *.o
