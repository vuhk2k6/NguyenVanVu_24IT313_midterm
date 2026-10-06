#include "ls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
int exit_status;
void *allocate(size_t n) {
    void *p = malloc(n ? n : 1);
    if (!p) { perror("myls: malloc"); exit(1); }
    return p;
}
char *copy_string(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = allocate(n); memcpy(p, s, n); return p;
}
char *join_path(const char *a, const char *b) {
    size_t x = strlen(a), y = strlen(b);
    if (x > SIZE_MAX - y - 2) { fprintf(stderr, "myls: path too long\n"); exit(1); }
    char *p = allocate(x + y + 2);
    memcpy(p, a, x);
    if (x && a[x-1] != '/') p[x++] = '/';
    memcpy(p+x, b, y+1); return p;
}
void report_error(const char *path) {
    fprintf(stderr, "myls: %s: %s\n", path, strerror(errno)); exit_status = 1;
}
