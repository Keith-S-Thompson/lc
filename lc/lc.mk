#
#    "@(#)lc.mk	1.6 9/7/90 - Kent Landfield (c) 1984,1985,1986,1987,1988,1989,1990
#
# This makefile is used to compile lc. 
#
#      Initially designed on an IBM-XT running Coherent in 1984.
#      Ported to XENIX on an IBM-AT in 1984.
#      Ported to System V on AT&T 3Bs in 1985.
#      Ported to DEC Vax 11/750 running System V in 1986.
#      Ported to BSD4.2 on a Sequent Balance 8000 in 1986.
#      Jeff Minnig added the initial support for links. 
#      Ported to SunOS 4.0 on a Sun 3/60 in 1988.
#      Rick Ohnemus did major surgery to remove static storage
#      and *greatly* enhanced the link support. Thanks rick!
#      Tested with Ultrix 3.0 & 3.1 on a DECstation 3100 in 1989.
#      Tested with Ultrix 3.0 & 3.1 on a VAXstation 3500 in 1989.
#      Ported to AIX 2.2 on an IBM RT.
#      Tested with UTek on a Tektronix 4319 in 1989.
#      Tested with IRIX System V on a Silicon Graphics Iris 4D/210GTX in 1989.
#      Tested with AmigaDOS 1.3 on an Amiga 1000 in 1989.
#      Tested with SunOS 4.0.3 on a Sparkstation 1 in 1989.
#      Tested with UTek on a Tektronix XD8810 in 1989.
#      Runs on AIX 3.+ on a Risc System/6000 in 1990. 
#                                                               
I =	/usr/include
S = 	$(I)/sys
#
#  Have a favorite C compiler that is not cc... Too bad. ;-)
CC=cc
#CC=gcc
#
# If you are running on a BSD 4.2 box:
# (note - if compiling on a sequent in att environment...`ucb make -f lc.mk`)
# FLAGS = -DBSD -DDIRECT
#         or
#
# If you are running on a BSD (4.3 or later), SunOS (4.0 or later),
# or Ultrix (3.0 or later) box:
FLAGS = -DBSD
#         or
#
# If you are running on an Ultrix box and using the POSIX environment:
# FLAGS = -DPOSIX
#         or
#
# If you are running on a Xenix box:
# FLAGS = -DXENIX
#         or
#
# If you are running System V or AIX 2.2:
# FLAGS = 
#         or
#
# This runs on AIX but it does not lint well due to the include
# files on AIX. It works, that's all I can say...
# If you are running AIX 3.0 or later:
# FLAGS = -D_BSD -DBSD
#         or
#
# If you are running System V with Doug Gwyn's directory routines
# or Silicon Graphics or Utek 3.2d:
# FLAGS = -DPOSIX
# 
#
# OPTIM is used for setting debugging or optimizing
# flags for the compilation.
#OPTIM=-O -Wall
OPTIM=-O

# Are the directory routines in another library ?
# Or do you wish to use shared libraries ?
# Add additional libraries here...
# LDFLAGS = -lndir
# LDFLAGS = -lc_s
LDFLAGS = 
#
# 'qsort' function in C library
# QSORTO =
# QSORTC =
#
# 'qsort' function not in C library Or Your qsort library
# function is slooow.
#
# QSORTO = qsort.o
# QSORTC = qsort.c
#
# Installation ownership and directory. Customize
# for your installation. Warning, if you do not
# install this on your root partition, it will
# not be available for use in single user mode.
# Yes I know that this last statement is obvious
# but it is extremely irritating not to have it
# available...
#
# This is one program that we use so often, that we have set
# the sticky bit on (chmod +t lc) on our older systems were
# it matters...
#
BINDIR=/bin
MODE=755
OWNER=bin
GROUP=bin

CFLAGS = $(OPTIM) $(FLAGS)
OBJS = lc.o $(QSORTO)

lc: lc.o $(QSORTO)
	$(CC) $(CFLAGS) lc.o $(QSORTO) -o lc $(LDFLAGS)

lint:
	lint $(FLAGS) $(QSORTC) lc.c

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f lc

install: lc
	strip lc
	cp lc $(BINDIR)/lc
	chmod $(MODE)  $(BINDIR)/lc
	chown $(OWNER) $(BINDIR)/lc
	chgrp $(GROUP) $(BINDIR)/lc
