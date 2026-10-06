#include "ls.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>
#include <string.h>
#ifdef __linux__
#include <sys/sysmacros.h>
#endif
/* Preserve printable multibyte names instead of replacing UTF-8 bytes. */
void print_name(const char *s) {
    mbstate_t state={0};
    if (!opt.quote) { fputs(s,stdout); return; }
    while (*s) {
        wchar_t wc; size_t n=mbrtowc(&wc,s,strlen(s),&state);
        if (n==(size_t)-1 || n==(size_t)-2) {
            putchar('?'); s++; memset(&state,0,sizeof(state));
        } else if (!n) break;
        else { if (iswprint(wc)) fwrite(s,1,n,stdout); else putchar('?'); s+=n; }
    }
}
static char type(mode_t m) {
    if (S_ISDIR(m)) return 'd';
    if (S_ISLNK(m)) return 'l';
    if (S_ISBLK(m)) return 'b';
    if (S_ISCHR(m)) return 'c';
    if (S_ISSOCK(m)) return 's';
    if (S_ISFIFO(m)) return 'p';
#ifdef S_ISWHT
    if (S_ISWHT(m)) return 'w';
#endif
#ifdef S_ISARCH1
    if (S_ISARCH1(m)) return 'a';
#endif
#ifdef S_ISARCH2
    if (S_ISARCH2(m)) return 'A';
#endif
    return '-';
}
static void mode_string(mode_t m, char out[11]) {
    const mode_t bits[]={S_IRUSR,S_IWUSR,S_IXUSR,S_IRGRP,S_IWGRP,S_IXGRP,S_IROTH,S_IWOTH,S_IXOTH};
    out[0]=type(m);
    for (int i=0;i<9;i++) out[i+1]=(m&bits[i]) ? "rwx"[i%3] : '-';
    if (m&S_ISUID) out[3]=(m&S_IXUSR)?'s':'S';
    if (m&S_ISGID) out[6]=(m&S_IXGRP)?'s':'S';
    if (m&S_ISVTX) out[9]=(m&S_IXOTH)?'t':'T';
    out[10]='\0';
}
static void human(uintmax_t bytes) {
    const char *units="BKMGTPE"; unsigned i=0; long double n=bytes;
    while (n>=1024 && i<6) { n/=1024; i++; }
    if (!i) printf("%juB",bytes);
    else if (n<10) printf("%.1Lf%c",n,units[i]);
    else printf("%.0Lf%c",n,units[i]);
}
static uintmax_t used(const Entry *e) {
    return e->st.st_blocks>0 ? (uintmax_t)e->st.st_blocks*512 : 0;
}
static void blocks(uintmax_t bytes) {
    uintmax_t unit=opt.units==2 ? 1024 : opt.blocksize;
    if (opt.units==1) human(bytes);
    else printf("%ju",bytes/unit+(bytes%unit!=0));
}
void print_total(const Entries *list) {
    uintmax_t sum=0;
    for (size_t i=0;i<list->count;i++) sum+=used(&list->data[i]);
    fputs("total ",stdout); blocks(sum); putchar('\n');
}
static char suffix(mode_t m) {
    if (S_ISDIR(m)) return '/';
    if (S_ISLNK(m)) return '@';
    if (S_ISSOCK(m)) return '=';
    if (S_ISFIFO(m)) return '|';
#ifdef S_ISWHT
    if (S_ISWHT(m)) return '%';
#endif
    return S_ISREG(m) && (m&0111) ? '*' : '\0';
}
static void link_target(const Entry *e) {
    size_t cap=128; char *buf;
    for (;;) {
        buf=allocate(cap+1); ssize_t n=readlink(e->path,buf,cap);
        if (n<0) { report_error(e->path); free(buf); return; }
        if ((size_t)n<cap) { buf[n]='\0'; break; }
        free(buf);
        if (cap>SIZE_MAX/2-1) exit(1);
        cap*=2;
    }
    fputs(" -> ",stdout); print_name(buf); free(buf);
}
void print_entry(const Entry *e) {
    if (opt.inode) printf("%ju ",(uintmax_t)e->st.st_ino);
    if (opt.blocks) { blocks(used(e)); putchar(' '); }
    if (opt.longfmt) {
        char mode[11],date[64]; struct tm tm;
        time_t stamp=opt.timekind==1 ? e->st.st_ctime : opt.timekind==2 ? e->st.st_atime : e->st.st_mtime;
        mode_string(e->st.st_mode,mode);
        printf("%s %ju ",mode,(uintmax_t)e->st.st_nlink);
        struct passwd *pw=opt.numeric?NULL:getpwuid(e->st.st_uid);
        if (pw) printf("%s ",pw->pw_name); else printf("%ju ",(uintmax_t)e->st.st_uid);
        struct group *gr=opt.numeric?NULL:getgrgid(e->st.st_gid);
        if (gr) printf("%s ",gr->gr_name); else printf("%ju ",(uintmax_t)e->st.st_gid);
        if (S_ISCHR(e->st.st_mode)||S_ISBLK(e->st.st_mode))
            printf("%ju, %ju",(uintmax_t)major(e->st.st_rdev),(uintmax_t)minor(e->st.st_rdev));
        else if (opt.units==1) human(e->st.st_size<0?0:(uintmax_t)e->st.st_size);
        else printf("%jd",(intmax_t)e->st.st_size);
        if (localtime_r(&stamp,&tm) && strftime(date,sizeof(date),"%b %e %H:%M",&tm)) printf(" %s ",date);
        else fputs(" ? ",stdout);
    }
    print_name(e->name);
    if (opt.classify) { char c=suffix(e->st.st_mode); if (c) putchar(c); }
    if (opt.longfmt && S_ISLNK(e->st.st_mode)) link_target(e);
    putchar('\n');
}
