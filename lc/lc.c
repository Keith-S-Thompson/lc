/*
** This software is 
**
** Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 by Kent Landfield.
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
**              kent@sparky.IMD.Sterling.COM
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
**               -c      List character special files only
**               -d      List directories only
**               -D      Do not display singular files
**               -e      Mark executable files with '*'
**               -f      List regular files only
**               -F      List fifo files only
**               -1      List files one per line instead of in columns
**               -r      Do not sort the filenames before displaying.
**               -m      List shared memory name space entry files only
**               -M      List semaphore name space entry files only
**               -S      List socket file only
**               -s      List symbolic links only
**               -L      Display symbolic links
**               -l      Mark symbolic links with '@'
**               -I      Suppress unresolved symbolic link messages.
**
** The "only" options can not be combined.
** If there is no 'directory' specified, the current directory is used.
** Not all options are supported on every system. (e.g. no symbolic links
** on your system ? Options -s, -L or -l won't be available..)
**
**  History:
**      Initially designed on an IBM-XT running Coherent in 1984.
**      Ported to XENIX on an IBM-AT in 1984.
**      Ported to System V on AT&T 3Bs in 1985.
**      Ported to DEC Vax 11/750 running System V in 1986.
**      Ported to BSD4.2 on a Sequent Balance 8000 in 1986.
**      Jeff Minnig added the initial support for links. 
**      Ported to SunOS 4.0 on a Sun 3/60 in 1988.
**      Rick Ohnemus did major surgery to remove static storage
**      and *greatly* enhanced the link support. Thanks rick!
**      Tested with Ultrix 3.0 & 3.1 on a DECstation 3100 in 1989.
**      Tested with Ultrix 3.0 & 3.1 on a VAXstation 3500 in 1989.
**      Tested with UTek on a Tektronix 4319 in 1989.
**      Tested with IRIX System V on a Silicon Graphics Iris 4D/210GTX in 1989.
**      Tested with AmigaDOS 1.3 on an Amiga 1000 in 1989.
**      Tested with SunOS 4.0.3 on a Sparkstation 1 in 1989.
**      Tested with AIX 3.+ on a Risc System/6000 in 1990. 
**                                                               
*/
#ifndef lint
static char *sccsid = "@(#)lc.c	1.23 9/7/90  Kent Landfield";
#endif

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

#define NODES_PER_HUNK  256

#define TRUE            1
#define FALSE           0

#ifndef S_IXUSR
#  define S_IXUSR       S_IEXEC
#endif

#ifndef S_IXGRP
#  define S_IXGRP       (S_IEXEC >> 3)
#endif

#ifndef S_IXOTH
#  define S_IXOTH       (S_IEXEC >> 6)
#endif

#define DIR_ONLY        1
#define FILE_ONLY       2
#ifdef S_IFCHR
#  define CHAR_ONLY     3
#endif
#ifdef S_IFBLK
#  define BLOCK_ONLY    4
#endif
#ifdef S_IFIFO
#  define FIFO_ONLY     5
#endif
#ifdef S_IFLNK
#  define LNK_ONLY      6
#endif
#ifdef S_IFSOCK
#  define SOCK_ONLY     7
#endif
#ifdef S_IFNAM
#  define SEM_ONLY      8
#  define SD_ONLY       9
#endif

#ifdef BSD
#  define strrchr       rindex
#  define strchr        index
#endif


struct list {
    int num;
    int max;
    char **names;
#ifdef LENS
    int maxlen;
#endif
};

#ifdef LENS

#ifdef S_IFBLK
struct list Blks = { 0, 0, (char **) NULL, 0 };
#endif

#ifdef S_IFCHR
struct list Chrs = { 0, 0, (char **) NULL, 0 };
#endif

struct list Dirs = { 0, 0, (char **) NULL, 0 };
struct list Fls = { 0, 0, (char **) NULL, 0 };

#ifdef S_IFIFO
struct list Fifos = { 0, 0, (char **) NULL, 0 };
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

#ifdef S_IFIFO
struct list Fifos = { 0, 0, (char **) NULL };
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

int Allfiles = FALSE;
int Display_single = TRUE;
int Executables = FALSE;
int Ignore = FALSE;
int Level = 0;
int Maxlen = 0;
int Only = FALSE;
int Screen_width = 80;
int Single = FALSE;
int Sort_wanted = TRUE;

#ifdef S_IFLNK
int Current = 0;
int Disp_links = FALSE;
int Mark_links = FALSE;
int lstat();
int readlink();
#endif

#ifndef _BSD
# ifdef BSD
    extern char *sprintf();
    extern int  exit();
    extern int  free();
    extern int  qsort();
# else
    extern int  sprintf();
    extern void exit();
    extern void free();
    extern void qsort();
# endif
 extern int fprintf();
 extern int printf();
 extern int sscanf();
#endif

void lc();

extern char *getenv();
extern char *malloc();
extern char *realloc();
extern int access();
extern int fputs();
extern int puts();
extern int stat();

/* S T R _ S A V  
 *
 *   str_sav() returns a pointer to a new string which is a dupli-
 *   cate  of the string pointed to by s.  The space for the new
 *   string is obtained using malloc(3).  If the new string  can-
 *   not be created, the process prints an error message on stderr
 *   and terminates.
 */

char *str_sav(s)
    char *s;
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
void directory(dname)
    char *dname;
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
void directory(dname)
    char *dname;
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
char *getlink(fn)
    char *fn;
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

int print_line(files, ind)
    struct list *files;
    int ind;
{
    register char *frmt;
    char out_str[PATH_MAX + 3];
    int i;
    int prt_limit;

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
              ind++;
         }
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

int str_cmp(s1, s2)
    char **s1;
    char **s2;
{
    return strcmp(*s1, *s2);
}

/* P R _ I N F O
 *
 *  pr_info() is used to sort the data if required
 *  and then pass it to print_line to actually output
 *  the data.`
 */

int pr_info(strng, files, flg, sort_needed)
    char *strng;
    struct list *files;
    int flg;
    int sort_needed;
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
        qsort((char *) (files->names), files->num,
              sizeof(char *), str_cmp);

    do {
        pnum = print_line(files, pnum);
    } while (pnum < files->num);

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

    if (Lnks.num > 0 && (Disp_links == TRUE || Only == LNK_ONLY)) {
        ssing = Single;
        Single = TRUE;
        Current = LNK_ONLY;
        flag = pr_info("Symbolic Links: ", &Lnks, flag, 0);
        Single = ssing;
    }
#endif

#ifdef S_IFSOCK
    if (Socks.num > 0 && (Only == 0 || Only == SOCK_ONLY))
        flag = pr_info("Sockets: ", &Socks, flag, Sort_wanted);
#endif

#ifdef S_IFNAM
    if (Sems.num > 0 && (Only == 0 || Only == SEM_ONLY))
        flag = pr_info("Semaphore Files: ", &Sems, flag, Sort_wanted);

    if (Sds.num > 0 && (Only == 0 || Only == SD_ONLY))
        flag = pr_info("Shared Data Files: ", &Sds, flag, Sort_wanted);
#endif

#ifdef S_IFIFO
    if (Fifos.num > 0 && (Only == 0 || Only == FIFO_ONLY))
        flag = pr_info("Fifo Files: ", &Fifos, flag, Sort_wanted);
#endif

#ifdef S_IFCHR
    if (Chrs.num > 0 && (Only == 0 || Only == CHAR_ONLY))
        flag = pr_info("Character Special Files: ", &Chrs, flag, Sort_wanted);
#endif

#ifdef S_IFBLK
    if (Blks.num > 0 && (Only == 0 || Only == BLOCK_ONLY))
        flag = pr_info("Block Special Files: ", &Blks, flag, Sort_wanted);
#endif

    if (Dirs.num > 0 && (Only == 0 || Only == DIR_ONLY))
        flag = pr_info("Directories: ", &Dirs, flag, Sort_wanted);

    if (Fls.num > 0 && (Only == 0 || Only == FILE_ONLY))
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

void lc(name, cnt)
    char *name;
    int cnt;
{
#ifdef S_IFLNK
    char *link;
#endif
    char sav_str[BUFSIZE + 2];
    int mlen;
    struct stat sbuf;

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

    basename(name, sav_str);
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

#ifdef S_IFLNK
    case S_IFLNK:
        if (!Allfiles && sav_str[0] == '.')
            break;
        add_to_list(&Lnks, sav_str);
        link = getlink(name);
        add_to_list(&Lnksn, link);
        if (link != (char *) NULL)
            free(link);
#ifdef LENS
        if (mlen > Lnks.maxlen)
            Lnks.maxlen = mlen;
#endif
        if (stat(name, &sbuf) < 0) {
            if (!Ignore) 
                (void) fprintf(stderr,
                               "%s: %s: can't resolve symbolic link\n",
                               Progname, name);
        }
        else {
            if (Mark_links) {
                *(sav_str + mlen) = '@';
                ++mlen;
                *(sav_str + mlen) = '\0';
#ifndef LENS
                if (mlen > Maxlen)
                    Maxlen = mlen;
#endif
            }

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

#ifdef S_IFIFO
            case S_IFIFO:
                add_to_list(&Fifos, sav_str);
#ifdef LENS
                if (mlen > Fifos.maxlen)
                    Fifos.maxlen = mlen;
#endif
                break;
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

void valid_opt(c, usage)
    char c;
    int usage;
{
    char up[7];

    up[0] = '\0';

    switch(c) {

    case 'a':
        Allfiles = TRUE;
        break;

    case 'b':
        Only = BLOCK_ONLY;
        break;

    case 'c':
        Only = CHAR_ONLY;
        break;

    case 'd':
        Only = DIR_ONLY;
        break;

    case 'D':
        Display_single = FALSE;
        break;

    case 'e':
        Executables = TRUE;
        break;

    case 'f':
        Only = FILE_ONLY;
        break;

    case 'r':
        Sort_wanted = FALSE;
        break;

#ifdef S_IFIFO
    case 'F':
        Only = FIFO_ONLY;
        break;
#endif

    case '1':
        Single = TRUE;
        break;

#ifdef S_IFLNK
    case 's':
        Only = LNK_ONLY;
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

#ifdef S_IFSOCK
    case 'S':
        Only = SOCK_ONLY;
        break;
#endif

#ifdef S_IFNAM
    case 'm':
        Only = SD_ONLY;
        break;

    case 'M':
        Only = SEM_ONLY;
        break;
#endif

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
                           "usage: %s [-abcdDefF1%s] [directories or files]\n",
                           Progname, up);
            exit(1);
        }
    }

    return;
}

/* S E T _ E N V _ V A R S
 *
 * set_env_vars() is used get the environment variables that
 * lc uses. The environment variable LC can be used to setup
 * the default way in which the user likes to see a directory
 * listing. Command line options override those specified in
 * the environment.
 */

void set_env_vars()
{
    char *ep;

    if ((ep = getenv("COLS")) != (char *) NULL) {
        if (sscanf(ep, "%d", &Screen_width) == 0
            || (Screen_width != 80 && Screen_width != 132))
            Screen_width = 80;
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

int spdist(s, t)
    char *s;
    char *t;
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
int mindist(dir, guess, best)    /* set best, return distance 0..3 */
    char *dir;
    char *guess;
    char *best;
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

int mindist(dir, guess, best)    /* set best, return distance 0..3 */
    char *dir;
    char *guess;
    char *best;
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

int spname(oldname, newname)
    char *oldname;
    char *newname;
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
 *  in_cdpath() searches the CDPATH stored in the environment
 *  for the filename specified. If it is found, fill the
 *  storage area refered to by buffer with the corrected path.
 *  Return TRUE if located and FALSE if not located in the CDPATH.
 */

int in_cdpath(requested_dir, buffer)
    char *requested_dir;
    char *buffer;
{
    static char *cdpath;
    static int first = 1;

    char *cp;
    char *path;
    char patbuf[BUFSIZE + 1];
    int quit;

    if (first) {
        if ((cdpath = getenv("CDPATH")) != (char *) NULL)
            cdpath = str_sav(cdpath);
        first = 0;
    }

    if (cdpath == (char *) NULL)
        return (0);

    (void) strcpy(patbuf, cdpath);
    path = patbuf;

    quit = 0;

    while (!quit) {
        cp = strchr(path, ':');
        if (cp == (char *) NULL)
            quit++;
        else
            *cp = '\0';

        if (*(path + 1) == '\0' && *path == '/')
            (void) sprintf(buffer, "/%s", requested_dir);
        else
            (void) sprintf(buffer, "%s/%s",
                           (*path ? path : "."), requested_dir);

        if (access(buffer, 1) == 0)
            return (TRUE);
        path = ++cp;
    }
    return (FALSE);
}

/*  M A I N 
 * 
 *  Ye olde main();
 */

int main(argc, argv)
    int argc;
    char *argv[];
{
    char *argp;
#ifdef S_IFLNK
    char *link;
#endif
    char buf[BUFSIZE + 1];
    int idx;
    int nl;
    struct stat sbuf;

    nl = idx = FALSE;

    Progname = argv[0];

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
        if (stat(buf, &sbuf) == -1) {
            if (in_cdpath(*argv, buf) || (spname(*argv, buf) != -1)) {
                /*
                 ** Check to see if the requested is in the CDPATH
                 ** and if not try to correct for typos. Always print
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

#ifdef S_IFIFO
            case S_IFIFO:
                if (Display_single)
                   (void) printf("%s: fifo file\n", buf);
                break;
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
                       (void) printf("%s: symbolic link to %s\n",
                                     buf, link);
                       free(link);
                   }
                   else
                       (void) printf("%s: unresolved symbolic link\n",
                                     buf);
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

#ifdef S_IFIFO
                Fifos.num = 0;
#ifdef LENS
                Fifos.maxlen = 0;
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
