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

-- Keith Thompson <Keith.S.Thompson@gmail.com> Sun 2012-03-11
