#
#    "@(#)lc.mk	1.13 10/18/92 
#
#  Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990,
#                1991, 1992 by Kent Landfield.
#
# This makefile is used to compile lc. 
#
I =	/usr/include
S = 	$(I)/sys
#
#  Have a favorite C compiler that is not cc... Too bad. ;-)
CC=cc
#CC=gcc

# The number of columns displayed is be computed on a file type basis or by 
# the length of the longest element in any type category. By defining the
# LENS flag, the computation will be type independent.
LENGTH=-DLENS

# Specify the multiple of 512 that your du(1) reports blocksizes System V 
# du(1) gives the number of 512 byte blocks while BSD specifies kilobytes.
#RPTSIZ = -DBLK_MULTIPLE=1
RPTSIZ = -DBLK_MULTIPLE=2
#RPTSIZ = -DBLK_MULTIPLE=8

# If you want to specify the block size here uncomment the following line:
#BLKSIZE = -DBLOCKSIZE=512

FLAGS = -DPOSIX

# If you are running on a BSD 4.2 box:
# (note - if compiling on a sequent in att environment...`ucb make -f lc.mk`)
# FLAGS = -DBSD -DDIRECT
#         or
#
# If you are running on a BSD (4.3 or later), SunOS (4.0 or later),
# HPUX or Ultrix (3.0 or later) box:
# FLAGS = -DBSD
#         or
#
# If you are running Silicon Graphics such as the SGI Personal Iris:
# FLAGS = -DPOSIX -cckr
#
# If you are running on a Cray under CSOS: (and set RPTSIZ to 8)
# FLAGS = -DPOSIX
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
# FLAGS =  -DSYSV
#         or
#
# This runs on AIX but it does not lint well due to the include
# files on AIX. It works, that's all I can say...
# If you are running AIX 3.0 or later:
# FLAGS = -D_BSD -DBSD
#         or
#
# If you are running System V with Doug Gwyn's directory routines
# or Utek 3.2d:
# FLAGS = -DPOSIX
# 
#
# OPTIM is used for setting debugging or optimizing
# flags for the compilation.
#OPTIM=-O -Wall
OPTIM=-O

# If you wish to have LC to automagically adjust to the window size,
# you need to define TCAP and specify the Termcap library containing 
# the tgetent() and tgetnum() routines.
#
TCAP=-DTCAP
#TERMLIB=-ltermcap
TERMLIB=-lcurses
#TERMLIB=-ltermlib

# Are the directory routines in another library ?
# Or do you wish to use shared libraries ?
# Add additional libraries here...
# LIBS = -lndir
# LIBS = -lc_s
LIBS =

#
# 'qsort' function in C library
# QSORTO =
# QSORTC =
#
# 'qsort' function not in C library Or your qsort library
# function is slooow.
#
# QSORTO = qsort.o
# QSORTC = qsort.c

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
MANDIR=/usr/local/man/man1
MANEXT=1
MODE=755
OWNER=bin
GROUP=bin
LDFLAGS = $(TERMLIB) $(LIBS)

CFLAGS = $(OPTIM) $(FLAGS) $(BLKSIZE) $(RPTSIZ) $(LENGTH) $(TCAP)
LINTFLAGS = $(FLAGS) $(BLKSIZE) $(RPTSIZ) $(LENGTH) $(TCAP)
SRCS = lc.c $(QSORTC)
OBJS = lc.o $(QSORTO)

lc: lc.o $(QSORTO)
	$(CC) $(CFLAGS) lc.o $(QSORTO) -o lc $(LDFLAGS)

lint:
	lint $(LINTFLAGS) $(SRCS)

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

man: $(MANDIR)/lc.$(MANEXT)

$(MANDIR)/lc.$(MANEXT):	lc.1
	cp lc.1 $(MANDIR)/lc.$(MANEXT)
	chmod 444 $(MANDIR)/lc.$(MANEXT)
	chown $(OWNER) $(MANDIR)/lc.$(MANEXT)
	chgrp $(GROUP) $(MANDIR)/lc.$(MANEXT)

print:
	cprint MANIFEST  | lpr -Plw
	cprint README    | lpr -Plw
	cprint lc.1      | lpr -Plw
	cprint lc.c      | lpr -Plw
	cprint lc.mk     | lpr -Plw
	cprint qsort.c   | lpr -Plw

# CodeCenter lines

lc_src: $(SRCS)
	#load $(CFLAGS) $(SRCS)

# Purify lines

purify: lc.o $(QSORTO)
	purify $(CC) $(CFLAGS) lc.o $(QSORTO) -o lc $(LDFLAGS)

