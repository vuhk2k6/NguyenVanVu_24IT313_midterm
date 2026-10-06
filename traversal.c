#include "ls.h"
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
typedef struct Ancestor { dev_t dev; ino_t ino; const struct Ancestor *parent; } Ancestor;
static int printed;
static void walk(const Entry *e,int heading,const Ancestor *parent) {
    for (const Ancestor *p=parent;p;p=p->parent)
        if (p->dev==e->st.st_dev && p->ino==e->st.st_ino) {
            fprintf(stderr,"myls: %s: directory cycle\n",e->path); exit_status=1; return;
        }
    Ancestor current={e->st.st_dev,e->st.st_ino,parent};
    DIR *dir=opendir(e->path); Entries entries={0}; struct dirent *de;
    if (!dir) { report_error(e->path); return; }
    if (heading) {
        if (printed) putchar('\n');
        print_name(e->path); puts(":");
    }
    printed=1;
    for (;;) {
        errno=0; de=readdir(dir);
        if (!de) { if (errno) report_error(e->path); break; }
        const char *n=de->d_name;
        if (n[0]=='.' && !opt.all && (!opt.almost || !strcmp(n,".") || !strcmp(n,".."))) continue;
        char *path=join_path(e->path,n);
        add_entry(&entries,n,path,0); free(path);
    }
    if (closedir(dir)) report_error(e->path);
    sort_entries(&entries);
    if (opt.longfmt || (opt.blocks && isatty(STDOUT_FILENO))) print_total(&entries);
    for (size_t i=0;i<entries.count;i++) print_entry(&entries.data[i]);
    if (opt.recursive) for (size_t i=0;i<entries.count;i++) {
        Entry *child=&entries.data[i];
        if (S_ISDIR(child->st.st_mode) && strcmp(child->name,".") && strcmp(child->name,"..")) walk(child,1,&current);
    }
    free_entries(&entries);
}
void list_directory(const Entry *e,int heading) { walk(e,heading,NULL); }
