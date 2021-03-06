/*
** This software is 
**
** Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990,
**               1991, 1992 by Kent Landfield.
**
** Permission is hereby granted to copy, distribute or otherwise 
** use any part of this package as long as you do not try to make 
** money from it or pretend that you wrote it.  This copyright 
** notice must be maintained in any copy made.
**
** Use of this software constitutes acceptance for use in an AS IS 
** condition. There are NO warranties with regard to this software.  
** In no event shall the author be liable for any damages whatsoever 
** arising out of or in connection with the use or performance of this 
** software.  Any use of this software is at the user's own risk.
**
**  If you make modifications to this software that you feel 
**  increases it usefulness for the rest of the community, please 
**  email the changes, enhancements, bug fixes as well as any and 
**  all ideas to me. Thanks!
**
**              Kent Landfield
**              kent@sterling.com
**              sparky!kent
**
**  Subsystem:   lc - List Columnwise/Categories
**
**  Abstract :   lc -- categorize files in a directory and list columnwize
**
**  Usage:       lc [ options ] [ directory ... ]
**
**  Options:
**               -a      List dot files as well.
**               -b      List block special files only
** JB            -B      Display the size in blocks
**               -c      List character special files only
** JB            -C      Sort down the columns instead of across
**               -d      List directories only
**               -D      Do not display singular files
**               -e      Mark executable files with '*'
**               -f      List regular files only
**               -F      List fifo files only
** JB            -i      Display the inode number
**               -I      Suppress unresolved symbolic link messages.
**               -L      Display symbolic links
**               -l      Mark symbolic links with '@'
**               -m      List shared memory name space entry files only
**               -M      List semaphore name space entry files only
**               -r      Do not sort the filenames before displaying.
**               -S      List socket file only
**               -s      List symbolic links only
**               -v      Print the version of lc 
** JB            -x      Only display those files the user owns or has access to
** JB            -X      Only display those files the user does not own and
**                       does not have access to.
**               -1      List files one per line instead of in columns
**
** The "only" options can be combined.
** If there is no 'directory' specified, the current directory is used.
** Not all options are supported on every system. (e.g. no symbolic links
** on your system ? Options -s, -L or -l won't be available..)
**
*/
static char *sccsid = "@(#)lc.c	1.37 10/18/92 Kent Landfield";
#include "patchlevel.h"

#include <stdio.h>
#ifdef BSD
#  include <strings.h>
#  include <sys/param.h>
#else
#  include <string.h>
#  include <sys/types.h>
#endif
#include <sys/stat.h>
#ifdef POSIX
#  include <limits.h>
#  include <dirent.h>
#  include <stdlib.h>
#  include <curses.h>
#  include <term.h>
#  include <unistd.h>
#else
#  ifdef BSD
#    ifdef DIRECT
#      include <sys/dir.h>
#    else
#      include <dirent.h>
#    endif
#  else
#    include <sys/dir.h>
#  endif
#endif

#ifndef NAME_MAX
#  ifdef BSD
#    define NAME_MAX    MAXNAMLEN
#  else
#    ifdef DIRSIZ
#      define NAME_MAX  DIRSIZ
#    else
#      define NAME_MAX  14
#    endif
#  endif
#endif

#ifndef PATH_MAX
#  ifdef MAXPATHLEN
#    define PATH_MAX    MAXPATHLEN
#  else
#    ifdef MAXNAMLEN
#      define PATH_MAX  MAXNAMLEN
#    else
#      define PATH_MAX  255
#    endif
#  endif
#endif

#define BUFSIZE         PATH_MAX
#define NODES_PER_HUNK       256
#define DEFAULT_SCREEN_WIDTH  80

#ifndef TRUE
#define TRUE            1
#endif
#ifndef FALSE
#define FALSE           0
#endif

#ifndef S_IXUSR
#  define S_IXUSR       S_IEXEC
#endif

#ifndef S_IXGRP
#  define S_IXGRP       (S_IEXEC >> 3)
#endif

#ifndef S_IXOTH
#  define S_IXOTH       (S_IEXEC >> 6)
#endif

#define DIR_ONLY        1<<0
#define FILE_ONLY       1<<2
#ifdef S_IFCHR
#  define CHAR_ONLY     1<<3
#endif
#ifdef S_IFBLK
#  define BLOCK_ONLY    1<<4
#endif
#ifndef apollo
# ifdef S_IFIFO
#  define FIFO_ONLY     1<<5
# endif
#endif
#ifdef S_IFLNK
#  define LNK_ONLY      1<<6
#endif
#ifdef S_IFSOCK
#  define SOCK_ONLY     1<<7
#endif
#ifdef S_IFNAM
#  define SEM_ONLY      1<<8
#  define SD_ONLY       1<<9
#endif

#ifdef BSD
#  define strrchr       rindex
#  define strchr        index
#endif

/*
** File name storage structure
*/

struct list {
    int num;
    int max;
    char **names;
#ifdef LENS
    int maxlen;
#endif
};

/*
** File name storage arrays
*/

#ifdef LENS

#ifdef S_IFBLK
struct list Blks = { 0, 0, (char **) NULL, 0 };
#endif

#ifdef S_IFCHR
struct list Chrs = { 0, 0, (char **) NULL, 0 };
#endif

struct list Dirs = { 0, 0, (char **) NULL, 0 };
struct list Fls = { 0, 0, (char **) NULL, 0 };

#ifndef apollo
#ifdef S_IFIFO
struct list Fifos = { 0, 0, (char **) NULL, 0 };
#endif
#endif

#ifdef S_IFLNK
struct list Lnks = { 0, 0, (char **) NULL, 0 };
struct list Lnksn = { 0, 0, (char **) NULL, 0 };
#endif

#ifdef S_IFSOCK
struct list Socks = { 0, 0, (char **) NULL, 0 };
#endif

#ifdef S_IFNAM
struct list Sds = { 0, 0, (char **) NULL, 0 };
struct list Sems = { 0, 0, (char **) NULL, 0 };
#endif

#else   /* ifndef LENS */

#ifdef S_IFBLK
struct list Blks = { 0, 0, (char **) NULL };
#endif

#ifdef S_IFCHR
struct list Chrs = { 0, 0, (char **) NULL };
#endif

struct list Dirs = { 0, 0, (char **) NULL };
struct list Fls = { 0, 0, (char **) NULL };

#ifndef apollo
#ifdef S_IFIFO
struct list Fifos = { 0, 0, (char **) NULL };
#endif
#endif

#ifdef S_IFLNK
struct list Lnks = { 0, 0, (char **) NULL };
struct list Lnksn = { 0, 0, (char **) NULL };
#endif

#ifdef S_IFSOCK
struct list Socks = { 0, 0, (char **) NULL };
#endif

#ifdef S_IFNAM
struct list Sds = { 0, 0, (char **) NULL };
struct list Sems = { 0, 0, (char **) NULL };
#endif

#endif /* LENS */

char *Progname;

int Allfiles = FALSE;       /* display '.' files as well    */
int Display_accessable = 0; /* display accessable files     */
int Display_inode = FALSE;  /* Display inode number         */
int Display_size = FALSE;   /* Display file size            */
int Display_single = TRUE;  /* Display on file at a time    */
int Executables = FALSE;    /* mark executable files        */
int Level = 0;              
int Maxlen = 0;             /* longest filename in category */
int Only = FALSE;           /* limit display to types       */
int Single = FALSE;         /* display files one per line   */
int Sort_wanted = TRUE;     /* display in directory order ? */
int Sort_down = FALSE;      /* sort by columns              */
int Sort_offset = 0;        
int Screen_width;           /* display Screen width         */

#define ACCESSABLE_ONLY    1
#define INACCESSABLE_ONLY  2

#ifdef S_IFLNK
int Ignore = FALSE;         /* ignore unresolved errors   */
int Current = 0;
int Disp_links = FALSE;
int Mark_links = FALSE;
/* int lstat(); */
/* int readlink(); */
#endif

void lc(char *name, int cnt);

/* S T R _ S A V  
 *
 *   str_sav() returns a pointer to a new string which is a dupli-
 *   cate  of the string pointed to by s.  The space for the new
 *   string is obtained using malloc(3).  If the new string  can-
 *   not be created, the process prints an error message on stderr
 *   and terminates.
 */

char *str_sav(char *s)
{
    char *p;

    if ((p = malloc((unsigned)(strlen(s) + 1))) == (char *) NULL) {
        (void) fprintf(stderr, "%s: malloc: out of memory\n", Progname);
        exit(1);
    }
    (void) strcpy(p, s);
    return (p);
}

/* D I R E C T O R Y
 *
 *  directory() is used to open and read the directory and pass
 *  the filenames found to the routine lc();
 */
#if (POSIX || BSD)
void directory(char *dname)
{
    register char *nbp;
    register char *nep;
    DIR *dstream;
    int i;
#ifdef DIRECT
    struct direct *dp;
#else
    struct dirent *dp;
#endif

    /* add a slash to the end of the directory name */
    nbp = dname + strlen(dname);
#ifdef AMIGA
    if (*(nbp - 1) != ':') {
        *nbp++ = '/';
        *nbp = '\0';
    }
#else
    *nbp++ = '/';
    *nbp = '\0';
#endif

    if ((dstream = opendir(dname)) == NULL) {
        (void) fprintf(stderr, "%s: can't open %s\n", Progname, dname);
        return;
    }

    while ((dp = readdir(dstream)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0
            || strcmp(dp->d_name, "..") == 0
            || (!Allfiles && *(dp->d_name) == '.'))
            continue;

        for (i = 0, nep = nbp; dp->d_name[i] && i < NAME_MAX; i++)
            *nep++ = dp->d_name[i];
        *nep++ = '\0';
        lc(dname, 2);
    }
    (void) closedir(dstream);
    *--nbp = '\0';
    return;
}

#else /* not POSIX or BSD */

/* D I R E C T O R Y
 *
 *  directory() is used to open and read the directory and pass
 *  the filenames found to the routine lc();
 */
void directory(char *dname)
{
    register char *nbp;
    register char *nep;
    FILE *fd;
    int i;
    struct direct dir;

    /* add a slash to the end of the directory name */
    nbp = dname + strlen(dname);
    *nbp++ = '/';
    *nbp = '\0';

    if ((nbp + NAME_MAX + 2) >= (dname + BUFSIZE))  { /* dname too long */
        (void) fprintf(stderr, "%s: dirname too long: %s\n",
                       Progname, dname);
        return;
    }

    if ((fd = fopen(dname, "r")) == (FILE *) NULL) { /* open the directory */
        (void) fprintf(stderr, "%s: can't open %s\n", Progname, dname);
        return;
    }

    while (fread((char *) &dir, sizeof(dir), 1, fd) > 0) {
        if (dir.d_ino == 0
            || strcmp(dir.d_name, ".") == 0
            || strcmp(dir.d_name, "..") == 0)
            continue;
        for (i = 0, nep = nbp; i < NAME_MAX; i++)
            *nep++ = dir.d_name[i];
        *nep++ = '\0';
        lc(dname, 2);
    }
    (void) fclose(fd);
    *--nbp = '\0';     /* restore dname */
    return;
}
#endif

/* G E T L I N K
 *
 *  getlink() calls readlink() which places the contents of the 
 *  symbolic link referred to by fn in the buffer wk. The contents 
 *  of the link are null terminated and the path is returned to
 *  the calling funtion as a pointer to a storage area created
 *  by using malloc(3);
 */

#ifdef S_IFLNK
char *getlink(char *fn)
{
    char wk[PATH_MAX + 1];
    int rc;

    rc = readlink(fn, wk, sizeof(wk));
    if (rc < 0)
        return ((char *) NULL);
    wk[rc] = '\0';
    return (str_sav(wk));
}
#endif

/* P R I N T _ L I N E 
 *
 *  print_line() is used to format and output the files previously 
 *  located. This routine could use a lot more smarts but currently
 *  it is rather crude...
 */

int print_line(struct list *files, int ind)
{
    register char *frmt;
    char out_str[PATH_MAX + 5];
    int i;
    int prt_limit;
    int numrows = 0;

    if (Single) {
#ifdef S_IFLNK
        if (Current == LNK_ONLY) {
            if (*(Lnksn.names + ind) != (char *) NULL)
                (void) printf("    %s -> %s\n",
                              *(Lnks.names + ind), *(Lnksn.names + ind));
            else
                (void) printf("    %s -> %s\n",
                              *(Lnks.names + ind), "UNRESOLVED");
            ind++;
            return (ind);
        }
#endif
        (void) puts(*(files->names + ind));
        ind++;
    }
    else if (Maxlen > ((Screen_width - 4) / 2)) {
        (void) printf("    %s\n", *(files->names + ind));
        ind++;
    }
    else {
         frmt = out_str;
         for (i = 0; i < 4; i++)
              *frmt++ = ' ';

        /* The prt_limit may need to be smarter */

         prt_limit = (Screen_width - 4) / (Maxlen + 1);

         /*  sort by columns */
#ifdef S_IFLNK
         if (Sort_down && Current != LNK_ONLY) {
#else       
         if (Sort_down) {
#endif
             numrows = (int)( (float)files->num / (float)prt_limit + 1.0);
             prt_limit = (int) ( (float)files->num / (float)numrows + (float)(numrows - 1) / (float)numrows);
         }

         if (Maxlen == 3 || Maxlen == 1)
             prt_limit--;

         while ((ind < files->num) && (prt_limit-- > 0)) {
              i = 0;
              do {
                   if (*(*(files->names + ind) + i) == '\0') {
                       while (i++ <= Maxlen)
                             *frmt++ = ' ';
                   }
                   else
                       *frmt++ = *(*(files->names + ind) + i);
                   i++;
              } while (i <= Maxlen);
#ifdef S_IFLNK
              if (Sort_down && Current != LNK_ONLY)
#else       
              if (Sort_down)
#endif
                 ind += numrows;
              else
                 ind++;
         }
         *frmt = '\0';
         while (*--frmt == ' ')  /* strip trailing blanks */
             *frmt = '\0';
        
         (void) puts(out_str);
    }
    return (ind);
}

/* S T R _ C M P
 *
 *  str_cmp is the comparison routine used by
 *  qsort(3) to order the filenames inplace.
 */

int str_cmp(const void *p1, const void *p2)
{
    const char *const *s1 = p1;
    const char *const *s2 = p2;
    /* inode/file sizes */
 
    return strcmp(&**s1 + Sort_offset, &**s2 + Sort_offset);
}

/* P R _ I N F O
 *
 *  pr_info() is used to sort the data if required
 *  and then pass it to print_line to actually output
 *  the data.`
 */

int pr_info(char *strng, struct list *files, int flg, int sort_needed)
{
    int pnum = 0;

#ifdef LENS
    Maxlen = files->maxlen;
#endif

#ifdef S_IFLNK
    if (!Single || Current == LNK_ONLY) {
        if (flg)
            (void) puts("");
        (void) puts(strng);
    }
#else
    if (!Single) {
        if (flg)
            (void) puts("");
        (void) puts(strng);
    }
#endif

    if (sort_needed)
        qsort((char *) (files->names), files->num, sizeof(char *), str_cmp);

    /* sort by columns */
    Maxlen++; /* this is to force an extra space between columns */
#ifdef S_IFLNK
    if (Sort_down && Current != LNK_ONLY) {
#else
    if (Sort_down) {
#endif
        int numcols = (Screen_width - 4) / (Maxlen + 1);
        int numrows = (int)( (float)files->num / (float)numcols + 1.0);
         
        numcols = (int) ( (float)files->num / (float)numrows + (float)(numrows - 1) / (float)numrows);
 
        do {
                (void) print_line(files, pnum);
                pnum++;
        } while (pnum < numrows);
    }    
    else {
        do {
            pnum = print_line(files, pnum);
        } while (pnum < files->num);
    }    
    return (1);
}


/* P R I N T _ I N F O
 *
 *  print_info() is called to display all the filenames
 *  located in the directory reading and storage functions.
 */

void print_info()
{
    int flag = 0;

#ifdef S_IFLNK
    int ssing;

    Current = 0;

    if (Lnks.num > 0 && (Disp_links == TRUE || Only & LNK_ONLY)) {
        ssing = Single;
        Single = TRUE;
        Current = LNK_ONLY;
        flag = pr_info("Symbolic Links: ", &Lnks, flag, FALSE);
        Single = ssing;
        Current = 0;
    }
#endif

#ifdef S_IFSOCK
    if (Socks.num > 0 && (Only == 0 || Only & SOCK_ONLY))
        flag = pr_info("Sockets: ", &Socks, flag, Sort_wanted);
#endif

#ifdef S_IFNAM
    if (Sems.num > 0 && (Only == 0 || Only & SEM_ONLY))
        flag = pr_info("Semaphore Files: ", &Sems, flag, Sort_wanted);

    if (Sds.num > 0 && (Only == 0 || Only & SD_ONLY))
        flag = pr_info("Shared Data Files: ", &Sds, flag, Sort_wanted);
#endif

#ifndef apollo
# ifdef S_IFIFO
    if (Fifos.num > 0 && (Only == 0 || Only & FIFO_ONLY))
        flag = pr_info("Fifo Files: ", &Fifos, flag, Sort_wanted);
# endif
#endif

#ifdef S_IFCHR
    if (Chrs.num > 0 && (Only == 0 || Only & CHAR_ONLY))
        flag = pr_info("Character Special Files: ", &Chrs, flag, Sort_wanted);
#endif

#ifdef S_IFBLK
    if (Blks.num > 0 && (Only == 0 || Only & BLOCK_ONLY))
        flag = pr_info("Block Special Files: ", &Blks, flag, Sort_wanted);
#endif

    if (Dirs.num > 0 && (Only == 0 || Only & DIR_ONLY))
        flag = pr_info("Directories: ", &Dirs, flag, Sort_wanted);

    if (Fls.num > 0 && (Only == 0 || Only & FILE_ONLY))
        flag = pr_info("Files: ", &Fls, flag, Sort_wanted);

    return;
}

/* B A S E N A M E
 *
 *  basename() is used to return the base file name of a
 *  path refered to by name. The base file name is stored
 *  into a storage location refered to by str. It is the
 *  calling function's responsibility to assure adequate
 *  storage is supplied.
 */

void basename(name, str)
    char *name;
    char *str;
{
    char *p;

    if ((p = strrchr(name, '/')) == (char *) NULL) {
#ifdef AMIGA
        if ((p = strchr(name, ':')) == (char *) NULL)
            (void) strcpy(str, name);
        else
            (void) strcpy(str, ++p);
#else
        (void) strcpy(str, name);
#endif
    }
    else
        (void) strcpy(str, ++p);
    return;
}

/* A D D _ T O _ L I S T
 *
 * add_to_list() is used to add the supplied filename refered to
 * by str to the appropriate category storage array.
 */

void add_to_list(files, str)
    struct list *files;
    char *str;
{
    if (files->max == 0) {
        files->names = (char **) malloc(sizeof(char *) * NODES_PER_HUNK);
        if (files->names == (char **) NULL) {
            (void) fprintf(stderr,
                           "%s: malloc: out of memory\n", Progname);
            exit(1);
        }
        files->max = NODES_PER_HUNK;
    }
    else if (files->num == files->max) {
        files->names =
            (char **) realloc((char *) files->names,
                              (unsigned) sizeof(char *)
                              * (files->max + NODES_PER_HUNK));
        if (files->names == (char **) NULL) {
            (void) fprintf(stderr,
                           "%s: realloc: out of memory\n", Progname);
            exit(1);
        }
        files->max += NODES_PER_HUNK;
    }
    if (str == (char *) NULL)
        *(files->names + files->num++) = (char *) NULL;
    else
        *(files->names + files->num++) = str_sav(str);
    return;
}

/* L C
 *
 * lc() is main function for determining the type of
 * the file refered to by name.
 */

void lc(char *name, int cnt)
{
#ifdef S_IFLNK
    char *link;
#endif
    char sav_str[BUFSIZE + 2 + 10]; /* inode/file size */
    char tmp[BUFSIZE + 2 + 10];     /* inode/file size */
    int mlen;
    struct stat sbuf;
    int display = TRUE;             /* access */

#ifdef S_IFLNK
    if (lstat(name, &sbuf) < 0) {
        (void) fprintf(stderr, "%s: can't stat %s\n", Progname, name);
        return;
    }
#else
    if (stat(name, &sbuf) == -1) {
        (void) fprintf(stderr, "%s: can't stat %s\n", Progname, name);
        return;
    }
#endif

    /*
    ** Only list files the user owns or has access to.
    */

    if (Display_accessable &&
        Display_accessable != (ACCESSABLE_ONLY | INACCESSABLE_ONLY)) {   
        static int first = 1;
        static uid_t uid;
        static uid_t euid;
        static gid_t gid;
        static gid_t egid;

        if (first) {
            first = 0;
            uid = getuid();
            euid = geteuid();
            gid = getgid();
            egid = getegid();
        }
        if (uid != sbuf.st_uid && euid != sbuf.st_uid &&
            gid != sbuf.st_gid && egid != sbuf.st_gid &&
            ((sbuf.st_mode & 0007) == 0) ) {
            if (Display_accessable & ACCESSABLE_ONLY)
                return;
        }
        else {
            if ((Display_accessable & INACCESSABLE_ONLY) &&
                    (sbuf.st_mode & S_IFMT) != S_IFDIR
#ifdef S_IFLNK
                 && (sbuf.st_mode & S_IFMT) != S_IFLNK
#endif
                                                        )
                return;
            if ((Display_accessable & INACCESSABLE_ONLY)
#ifdef S_IFLNK
                 && (sbuf.st_mode & S_IFMT) == S_IFLNK
#endif
                                                           )
                display = FALSE;
        }
    }

    basename(name, sav_str);

    /* inode/file size */
 
    *tmp = 0;
    if (Display_inode) 
        (void) sprintf(tmp, "%5ld", (long)sbuf.st_ino);
    
    if (Display_size) {
        /* Make our best guess here for the block size.  Allow the user */
        /* compiling the program to override any system constant by     */
        /* specifying the blocksize on the command line.   JB           */
 
#ifdef BLOCKSIZE
        long st_blocks = (BLOCKSIZE - 1 + sbuf.st_size) / BLOCKSIZE;
#else
# ifdef BSD
        long st_blocks = sbuf.st_blocks / BLK_MULTIPLE;
# else    
#  ifdef STD_BLK
        long st_blocks = (STD_BLK - 1 + sbuf.st_size) / STD_BLK;
#  else
#   ifdef BUFSIZ
        long st_blocks = (BUFSIZ - 1 + sbuf.st_size) / BUFSIZ;
#   else
        long st_blocks = (511 + sbuf.st_size) / 512;
#   endif
#  endif
# endif
#endif

        if (*tmp)
            (void) sprintf(&tmp[strlen(tmp)], " %4ld", (long)st_blocks);
        else    
            (void) sprintf(tmp, "%4ld", (long)st_blocks);
 
    }
    if (*tmp) {
        Sort_offset = strlen(tmp) + 1;
        (void) sprintf(&tmp[strlen(tmp)], " %s", sav_str);
    }
    else
        (void) sprintf(tmp, "%s", sav_str);
    (void) strcpy(sav_str, tmp);
 
    mlen = strlen(sav_str);

#ifndef LENS
    if (mlen > Maxlen)
        Maxlen = mlen;
#endif

    switch (sbuf.st_mode & S_IFMT) {

    case S_IFDIR:
        if (!Allfiles && sav_str[0] == '.' && Level != 0)
            break;
        if (cnt != 1)        /* dont store the dir name on entry */
            add_to_list(&Dirs, sav_str);
        /* never called - left for expansion to recursive     */
        /* searches of subdirectories. Right, re-write needed */
        /* in output facilities first...                      */
        if (Level++ == 0)
            directory(name);
#ifdef LENS
        if (mlen > Dirs.maxlen)
            Dirs.maxlen = mlen;
#endif
        break;

    case S_IFREG:
        /* do not print .files unless enviromental variable */
        /* or option set.                                   */
        if (!Allfiles && sav_str[0] == '.')
            break;
        if (Executables
            && (sbuf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
            *(sav_str + mlen) = '*';
            ++mlen;
            *(sav_str + mlen) = '\0';
#ifndef LENS
            if (mlen > Maxlen)
                Maxlen = mlen;
#endif
        }
        add_to_list(&Fls, sav_str);
#ifdef LENS
        if (mlen > Fls.maxlen)
            Fls.maxlen = mlen;
#endif
        break;

#ifdef S_IFCHR
    case S_IFCHR:
        if (!Allfiles && sav_str[0] == '.')
            break;
        add_to_list(&Chrs, sav_str);
#ifdef LENS
        if (mlen > Chrs.maxlen)
            Chrs.maxlen = mlen;
#endif
        break;
#endif

#ifdef S_IFBLK
    case S_IFBLK:
        if (!Allfiles && sav_str[0] == '.')
            break;
        add_to_list(&Blks, sav_str);
#ifdef LENS
        if (mlen > Blks.maxlen)
            Blks.maxlen = mlen;
#endif
        break;
#endif

#ifndef apollo
#ifdef S_IFIFO
    case S_IFIFO:
        if (!Allfiles && sav_str[0] == '.')
            break;
        add_to_list(&Fifos, sav_str);
#ifdef LENS
        if (mlen > Fifos.maxlen)
            Fifos.maxlen = mlen;
#endif
        break;
#endif
#endif

#ifdef S_IFLNK
    case S_IFLNK:
        if (!Allfiles && sav_str[0] == '.')
            break;
        if (display)
            add_to_list(&Lnks, sav_str);
        link = getlink(name);
        if (display)
            add_to_list(&Lnksn, link);
        if (link != (char *) NULL)
            free(link);
#ifdef LENS
        if (mlen > Lnks.maxlen && display)
            Lnks.maxlen = mlen;
#endif
        if (stat(name, &sbuf) < 0) {
            if (!Ignore) 
                (void) fprintf(stderr,
                               "%s: %s: can't resolve symbolic link\n",
                               Progname, name);
        }
        else {
            if (display && Mark_links) {
                *(sav_str + mlen) = '@';
                ++mlen;
                *(sav_str + mlen) = '\0';
#ifndef LENS
                if (mlen > Maxlen)
                    Maxlen = mlen;
#endif
            }

            if (display ||  (sbuf.st_mode & S_IFMT) == S_IFDIR)
                switch (sbuf.st_mode & S_IFMT) {

            case S_IFDIR:
                if (cnt != 1)        /*dont store the dir name on entry */
                    add_to_list(&Dirs, sav_str);
                /* never called - left for expansion to recursive */
                /* searches of subdirectories                     */
                if (Level++ == 0)
                    directory(name);
#ifdef LENS
                if (mlen > Dirs.maxlen)
                    Dirs.maxlen = mlen;
#endif
                break;

            case S_IFREG:
                if (Executables
                    && (sbuf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
                    *(sav_str + mlen) = '*';
                    ++mlen;
                    *(sav_str + mlen) = '\0';
#ifndef LENS
                    if (mlen > Maxlen)
                        Maxlen = mlen;
#endif
                }
                add_to_list(&Fls, sav_str);
#ifdef LENS
                if (mlen > Fls.maxlen)
                    Fls.maxlen = mlen;
#endif
                break;

#ifdef S_IFCHR
            case S_IFCHR:
                add_to_list(&Chrs, sav_str);
#ifdef LENS
                if (mlen > Chrs.maxlen)
                    Chrs.maxlen = mlen;
#endif
                break;
#endif

#ifdef S_IFBLK
            case S_IFBLK:
                add_to_list(&Blks, sav_str);
#ifdef LENS
                if (mlen > Blks.maxlen)
                    Blks.maxlen = mlen;
#endif
                break;
#endif

#ifndef apollo
#ifdef S_IFIFO
            case S_IFIFO:
                add_to_list(&Fifos, sav_str);
#ifdef LENS
                if (mlen > Fifos.maxlen)
                    Fifos.maxlen = mlen;
#endif
                break;
#endif
#endif

#ifdef S_IFSOCK
            case S_IFSOCK :
                add_to_list(&Socks, sav_str);
#ifdef LENS
                if (mlen > Socks.maxlen)
                    Socks.maxlen = mlen;
#endif
                break;
#endif
            }
        }
        break;
#endif

#ifdef S_IFSOCK
    case S_IFSOCK:
        if (!Allfiles && sav_str[0] == '.')
            break;
        add_to_list(&Socks, sav_str);
#ifdef LENS
        if (mlen > Socks.maxlen)
            Socks.maxlen = mlen;
#endif
        break;
#endif

#ifdef S_IFNAM
    case S_IFNAM:
        switch (sbuf.st_rdev) {

        case S_INSEM:
            if (!Allfiles && sav_str[0] == '.')
                break;
            add_to_list(&Sems, sav_str);
#ifdef LENS
            if (mlen > Sems.maxlen)
                Sems.maxlen = mlen;
#endif
            break;

        case S_INSHD:
            if (!Allfiles && sav_str[0] == '.')
                break;
            add_to_list(&Sds, sav_str);
#ifdef LENS
            if (mlen > Sds.maxlen)
                Sds.maxlen = mlen;
#endif
            break;
        }
        break;
#endif

    }
    return;
}

/* V A L I D _ O P T
 *
 * valid_opt() is used to translate user has supplied option
 * letters into something that this process can use. It sets
 * up the options and if a usage is requested, it sets up and
 * prints a usage message for the user.
 */

void valid_opt(char c, int usage)
{
    char up[7];

    up[0] = '\0';

    switch(c) {

    case 'a':
        Allfiles = TRUE;
        break;

    case 'b':
        Only |= BLOCK_ONLY;
        break;

    case 'B':
        Display_size = TRUE;
        break;
 
    case 'c':
        Only |= CHAR_ONLY;
        break;

    case 'C':
        Sort_down = TRUE;
        break;

    case 'd':
        Only |= DIR_ONLY;
        break;

    case 'D':
        Display_single = FALSE;
        break;

    case 'e':
        Executables = TRUE;
        break;

    case 'f':
        Only |= FILE_ONLY;
        break;

#ifndef apollo
# ifdef S_IFIFO
    case 'F':
        Only |= FIFO_ONLY;
        break;
# endif
#endif

    case 'i':
        Display_inode = TRUE;
        break;

#ifdef S_IFLNK
    case 's':
        Only |= LNK_ONLY;
        break;

    case 'l':
        Mark_links = TRUE;
        break;

    case 'I':
        Ignore = TRUE;
        break;

    case 'L':
        Disp_links = TRUE;
        break;
#endif

#ifdef S_IFNAM
    case 'm':
        Only |= SD_ONLY;
        break;

    case 'M':
        Only |= SEM_ONLY;
        break;
#endif

    case 'r':
        Sort_wanted = FALSE;
        break;

#ifdef S_IFSOCK
    case 'S':
        Only |= SOCK_ONLY;
        break;
#endif

    case '1':
        Single = TRUE;
        break;

    case 'v':
        (void) fprintf(stderr,"%s:\t%s\n\tRelease: %d\n\tPatch level: %d\n",
                               Progname, sccsid, RELEASE, PATCHLEVEL);
        exit(1);
	/*NOTREACHED*/

  case 'x':
        Display_accessable += ACCESSABLE_ONLY;
        break;
 
    case 'X':
        Display_accessable += INACCESSABLE_ONLY;
        break;

    default:
        if (usage == TRUE) {
#ifdef S_IFLNK
            (void) strcat(up, "IlLs");
#endif
#ifdef S_IFSOCK
            (void) strcat(up, "S");
#endif
#ifdef S_IFNAM
            (void) strcat(up, "mM");
#endif
            (void) fprintf(stderr,
                    "usage: %s [-abBcCdDefFivxX1%s] [directories or files]\n",
                     Progname, up);
            exit(1);
        }
    }

    return;
}


/* G E T _ W I N _ C O L S 
 *  
 * Get the number of columns in the current window.
 */ 
 
int get_win_cols(void) 
{
    int co = 0;
#ifdef TCAP
    char *term;
    char entree[1024];
  
    if ((term = getenv("TERM")) == NULL) 
        return(0);
  
    switch (tgetent(entree, term)) {
       case -1: /* "Cannot open termcap database." */
           return(0);
       case 0: /* "Cannot find %s in termcap database." */
           return(0);
    }
    if ((co = tgetnum("co")) == -1) {
       /* "Cannot find number of columns. " */
       return(0);
    }
#endif /* TCAP */
    return (co);
}


/* S E T _ E N V _ V A R S
 *
 * set_env_vars() is used get the environment variables that
 * lc uses. The environment variable LC can be used to setup
 * the default way in which the user likes to see a directory
 * listing. Command line options override those specified in
 * the environment.
 */

void set_env_vars(void)
{
    char *ep;

    if ((Screen_width = get_win_cols()) == 0) {
        if ((ep = getenv("COLS")) != (char *) NULL) {
           if (sscanf(ep, "%d", &Screen_width) == 0
               || (Screen_width != 80 && Screen_width != 132))
               Screen_width = DEFAULT_SCREEN_WIDTH;
        }
        else
           Screen_width = DEFAULT_SCREEN_WIDTH;
    }
    if ((ep = getenv("LC")) != (char *) NULL) {
        while (*ep != '\0') {
            valid_opt(*ep, FALSE);
            ep++;
        }
    }
    return;
}

/* S P D I S T:   return the distance between two names
 *
 * very rough spelling metric:
 *          0 if the strings are identical
 *          1 if two chars are transposed
 *          2 if 1 char wrong, added or deleted
 *          3 otherwise
 */
#define EQ(s, t) (strcmp(s, t) == 0)

int spdist(char *s, char *t)
{
    while (*s++ == *t) {
        if (*t++ == '\0')
            return 0;             /* exact match */
    }
    if (*--s) {
        if (*t) {
            if (s[1] && t[1] && *s == t[1] && *t == s[1] && EQ(s+2, t+2))
                return 1;             /* transposition */
            if (EQ(s+1, t+1))
                return 2;             /* 1 char mismatch */
        }
        if (EQ(s+1, t))
            return 2;                /* extra chacter */
    }
    if (*t && EQ(s, t+1))
        return 2;                   /* missing character */
    return 3;
}

/*    M I N D I S T       
 * 
 *  mindist() searches the directory for the best guess
 *  in the event the requested file was not located.
 */

#if (POSIX || BSD)
int mindist(char *dir, char *guess, char *best)    /* set best, return distance 0..3 */
{
    DIR *dfd;
    int d;
    int nd;
#ifdef DIRECT
    struct direct *dp;
#else
    struct dirent *dp;
#endif

    if (dir[0] == '\0')
        dir = ".";
    d = 3;                      /* minimum distance */

    if ((dfd = opendir(dir)) == NULL)
        return d;

    while ((dp = readdir(dfd)) != NULL) {
        if (dp->d_ino) {
            nd = spdist(dp->d_name, guess);
            if (nd <= d && nd != 3) {
                (void) strcpy(best, dp->d_name);
                d = nd;
                if (d == 0)    /* exact match */
                    break;
            }
        }
    }
    (void) closedir(dfd);
    return d;
}

#else /* not POSIX or BSD */

/*    M I N D I S T       
 * 
 *  mindist() searches the directory for the best guess
 *  in the event the requested file was not located.
 */

int mindist(char *dir, char *guess, char *best)    /* set best, return distance 0..3 */
{
    FILE *fd;
    int d;
    int nd;
    struct {
        ino_t ino;
        char name[NAME_MAX + 1];   /* 1 more than in dir.h */
    } nbuf;

    nbuf.name[NAME_MAX] = '\0';   /* +1 for terminal '\0' */
    if (dir[0] == '\0')
        dir = ".";
    d = 3;                      /* minimum distance */
    if ((fd = fopen(dir, "r")) == (FILE *) NULL)
        return d;
    while (fread((char *) &nbuf, sizeof(struct direct), 1, fd) > 0) {
        if (nbuf.ino) {
            nd = spdist(nbuf.name, guess);
            if (nd <= d && nd != 3) {
                (void) strcpy(best, nbuf.name);
                d = nd;
                if (d == 0)    /* exact match */
                    break;
            }
        }
    }
    (void) fclose(fd);
    return d;
}
#endif

/* S P N A M E:    return correctly spelled filename
 *
 * spname(oldname, newname) char *oldname, *newname;
 *      returns  -1 if no reasonable match to oldname,
 *                0 if exact match,
 *                1 if corrected.
 * stores corrected name in newname.
 */

int spname(char *oldname, char *newname)
{
    char *new = newname;
    char *old = oldname;
    char *p;
    char best[NAME_MAX + 1];
    char guess[NAME_MAX + 1];

    for (;;) {
        while (*old == '/')   /* skip slashes */
            *new++ = *old++;
        *new = '\0';
        if (*old == '\0')     /* exact or corrected */
            return (strcmp(oldname, newname) != 0);
        p = guess;            /* copy next component into guess */
        for (/* void */ ; *old != '/' && *old != '\0'; old++) {
            if (p < (guess + NAME_MAX))
                *p++ = *old;
        }
        *p = '\0';
        if (mindist(newname, guess, best) >= 3)
            return (-1);        /* hopeless */
        for (p = best; *new = *p++; new++)   /* add to end */
            /* void */;
    }
}

/*  I N _ C D P A T H
 *
 *  in_cdpath() searches $CDPATH variable stored in the sh/ksh/zsh environments
 *  and searches $cdpath variable in the csh environment for the filename 
 *  specified. If the filename is found, fill the storage area refered to 
 *  by buffer with the corrected path.
 *
 *  Return TRUE if located and FALSE if not located in the CDPATH.
 */
 
int in_cdpath(char *requested_dir, char *buffer, int check_spelling)
{
    static char *cdpath;
    static char cdsep;
    static int first = 1;

    char *cp;
    char *path;
    char patbuf[BUFSIZE + 1];
    int quit;

    if (first) {
        if ((cdpath = getenv("CDPATH")) != (char *) NULL) {
            cdpath = str_sav(cdpath);
            cdsep = ':';
        }
        if (cdpath == (char *) NULL || *cdpath == '\0') {
            /*
            ** No sh $CDPATH, check if csh cdpath defined.
            */
            if ((cdpath = getenv("cdpath")) != (char *) NULL) {
                cdpath = str_sav(cdpath);
                cdsep = ' ';
            }
        }
        first = 0;
    }   


    if (cdpath == (char *) NULL || *cdpath == '\0') 
        return (0);

    (void) strcpy(patbuf, cdpath);
    path = patbuf;

    quit = 0;

    while (!quit) {
        if ((cp = strchr(path, cdsep)) == (char *) NULL)
            quit++;
        else
            *cp = '\0';
 
        if (*(path + 1) == '\0' && *path == '/')
            (void) sprintf(buffer, "/%s", requested_dir);
        else
            (void) sprintf(buffer, "%s/%s",
                           (*path ? path : "."), requested_dir);
 
        if (access(buffer, 0) == 0)
            return (TRUE);
 
        if (check_spelling) {
            char bfr[BUFSIZ + 1];
            (void) strcpy(bfr, buffer);
            if (spname(bfr, buffer) == 1)
                return (TRUE);
        }
        path = ++cp;
    }
    return (FALSE);
}

/*  M A I N 
 * 
 *  Ye olde main();
 */

int main(int argc, char *argv[])
{
    char *argp;
#ifdef S_IFLNK
    char *link;
    int lnk_found;
#endif
    char buf[BUFSIZE + 1];
    int idx;
    int nl;
    struct stat sbuf;

    nl = idx = FALSE;

    /* get the base name of the command */

    if ((Progname = strrchr(argv[0], '/')) == (char *) NULL) 
        Progname = argv[0];
    else
        ++Progname;

    set_env_vars();                       /* get environment variables */

    /* All command line arguments must be */
    /* lumped together such as `lc -aef`  */

    if (argc > 1 && argv[1][0] == '-') {  /* if first parm is command */
        argp = argv[1];

        while (*(++argp))
            valid_opt(*argp, TRUE);

        ++argv;
        --argc;
    }

    /*
    ** The user has not specified a file or directory 
    ** to be examined so assume that the current directory
    ** is what the user is requesting.
    */
    if (argc == 1) {
        (void) strcpy(buf, ".");
        lc(buf, 1);
        print_info();
        return(0);
    }

    /*
    ** The user has specified at least one file or 
    ** directory to be examined.
    */
    if (argc > 2)
        nl = TRUE;
    while (--argc > 0) {
        ++argv;
        (void) strcpy(buf, *argv);
skipit:
#ifdef S_IFLNK
        if (lstat(buf, &sbuf) == -1) {
            lnk_found = 1;
#else
        if (stat(buf, &sbuf) == -1) {
#endif
            if (in_cdpath(*argv, buf, FALSE) ||   /* Look for it in CDPATH */
                (spname(*argv, buf) != -1)   ||   /* Look for it mispelled */
                in_cdpath(*argv, buf, TRUE)) {    /* Mispelled in CDPATH ? */
                /*
                 ** Check to see if the requested is in the CDPATH
                 ** and if not try to correct for typos. If that fails
                 ** then check for typos in the CDPOATH. Always print
                 ** the name of what was found...
                 */

                nl = TRUE;
                goto skipit;
            }
            else 
                (void)fprintf(stderr, "%s: can't find %s\n",
                              Progname, *argv);
        }
        else {
#ifdef S_IFLNK
            if ((sbuf.st_mode & S_IFMT) == S_IFLNK)
                (void) stat(buf, &sbuf);
            /*
            ** No need to check return code here: if stat() fails use
            ** the sbuf retrieved with lstat() earlier.
            */
#endif
            switch (sbuf.st_mode & S_IFMT) {

            case S_IFREG:
                if (Display_single)
                   (void) printf("%s: file\n", buf);
                break;

#ifdef S_IFCHR
            case S_IFCHR:
                if (Display_single)
                   (void) printf("%s: character special file\n", buf);
                break;
#endif

#ifdef S_IFBLK
            case S_IFBLK:
                if (Display_single)
                   (void) printf("%s: block special file\n", buf);
                break;
#endif

#ifndef apollo
#ifdef S_IFIFO
            case S_IFIFO:
                if (Display_single)
                   (void) printf("%s: fifo file\n", buf);
                break;
#endif
#endif

#ifdef S_IFSOCK
            case S_IFSOCK:
                if (Display_single)
                   (void) printf("%s: socket file\n", buf);
                break;
#endif

#ifdef S_IFLNK
            case S_IFLNK:
                if (Display_single) {
                   if ((link = getlink(buf)) != (char *) NULL) {
                       (void) printf("%s: symbolic link to %s\n", buf,link);
                       free(link);
                   }
                   else
                       (void) printf("%s: unresolved symbolic link\n", buf);
                }
                break;
#endif

#ifdef S_IFNAM
            case S_IFNAM:
                if (Display_single) {
                   if (sbuf.st_rdev == S_INSHD)
                       (void) printf("%s: shared memory file\n", buf);
                   if (sbuf.st_rdev == S_INSEM)
                       (void) printf("%s: semaphore file\n", buf);
                }
                break;
#endif

            case S_IFDIR:
                Maxlen = Level = 0;
#ifdef S_IFBLK
                Blks.num = 0;
#ifdef LENS
                Blks.maxlen = 0;
#endif
#endif

#ifdef S_IFCHR
                Chrs.num = 0;
#ifdef LENS
                Chrs.maxlen = 0;
#endif
#endif

                Dirs.num = Fls.num = 0;
#ifdef LENS
                Dirs.maxlen = Fls.maxlen = 0;
#endif

#ifndef apollo
#ifdef S_IFIFO
                Fifos.num = 0;
#ifdef LENS
                Fifos.maxlen = 0;
#endif
#endif
#endif

#ifdef S_IFLNK
                Lnks.num = Lnksn.num = 0;
#ifdef LENS
                Lnks.maxlen = Lnksn.maxlen = 0;
#endif
#endif

#ifdef S_IFSOCK
                Socks.num = 0;
#ifdef LENS
                Socks.maxlen = 0;
#endif
#endif

#ifdef S_IFNAM
                Sds.num = Sems.num = 0;
#ifdef LENS
                Sds.maxlen = Sems.maxlen = 0;
#endif
#endif

                if (nl == TRUE) {
                    if (idx > 0)
                        (void) puts("");
                    else
                        ++idx;
                    (void) fputs(": ", stdout);
                    (void) fputs(buf, stdout);
                    (void) puts(" :");
                }
                lc(buf, 1);
                print_info();
                break;

            default:
                (void) printf("%s: unknown file type\n", buf);
                break;
            }
        }
    }
    return(0);
}
