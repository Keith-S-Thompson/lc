Kent Landfield's `lc` command

I've retrieved several versions of the `lc` command, a tool to
categorize and list files in columns, from old `comp.sources.misc`
archives from 1990 to 1993 and gathered them into this git repository.
I've also applied a few small patches of my own so it builds on
modern systems.

I have not touched any of the SCCS version information in these files;
for example, the `"@(#)lc.c 1.37 10/18/92 Kent Landfield"` in lc.c is
not current (git doesn't insert version information into checked out files).
I have updated patchlevel.h, changing the patch level from 2 to 3.

The `comp.sources.misc` subdirectory contains the original articles
posted to `comp.sources.misc`.  In some cases, I manually cleaned up
the mangling that Google Groups does to e-mail addresses.

To build:

    cd src
    make

This will create the executable `lc`.  There is currently no
installation script.  Just copy the `lc` executable into a directory
in your `$PATH`, and `lc.1` into the `man1` subdirectory of some
directory in your `$MATHPATH`.

I've *minimally* tested this (it builds and executes) on Ubuntu 11.04
and Solaris 9.

On Cygwin, the `curses.h` header is at `/usr/include/ncurses/curses.h`,
not `/usr/include/curses.h`; you'll need to edit the `Makefile`
(search for "Cygwin").  (**TODO:** Automate this.)

-- Keith Thompson <Keith.S.Thompson@gmail.com> Sun 2012-03-11
