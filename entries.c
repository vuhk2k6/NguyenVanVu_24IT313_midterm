#include "ls.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
int add_entry(Entries *list, const char *name, const char *path, int operand) {
    struct stat st, target;
    if (lstat(path, &st)) { report_error(path); return 0; }
    /* Follow command-line links to directories only for ordinary listings. */
    if (operand && S_ISLNK(st.st_mode) && !opt.directory && !opt.longfmt &&
        !opt.classify && !stat(path, &target) && S_ISDIR(target.st_mode)) st=target;
    if (list->count == list->capacity) {
        size_t cap = list->capacity ? list->capacity*2 : 32;
        if (cap < list->capacity || cap > SIZE_MAX/sizeof(Entry)) exit(1);
        Entry *p = realloc(list->data, cap*sizeof(*p));
        if (!p) { perror("myls: realloc"); exit(1); }
        list->data=p; list->capacity=cap;
    }
    list->data[list->count++] = (Entry){copy_string(name),copy_string(path),st};
    return 1;
}
void free_entries(Entries *list) {
    for (size_t i=0;i<list->count;i++) { free(list->data[i].name); free(list->data[i].path); }
    free(list->data); *list=(Entries){0};
}
static struct timespec entry_time(const Entry *e) {
    if (opt.timekind==1) return e->st.st_ctim;
    if (opt.timekind==2) return e->st.st_atim;
    return e->st.st_mtim;
}
static int compare(const void *a, const void *b) {
    const Entry *x=a,*y=b; int result=0;
    if (opt.sort==1) result=(x->st.st_size < y->st.st_size)-(x->st.st_size > y->st.st_size);
    if (opt.sort==2) {
        struct timespec p=entry_time(x),q=entry_time(y);
        result=(p.tv_sec < q.tv_sec)-(p.tv_sec > q.tv_sec);
        if (!result) result=(p.tv_nsec < q.tv_nsec)-(p.tv_nsec > q.tv_nsec);
    }
    if (!result) { int n=strcmp(x->name,y->name); result=(n>0)-(n<0); }
    return opt.reverse ? -result : result;
}
void sort_entries(Entries *list) {
    if (!opt.unsorted && list->count>1) qsort(list->data,list->count,sizeof(Entry),compare);
}
