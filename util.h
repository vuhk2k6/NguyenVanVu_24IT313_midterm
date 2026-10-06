#ifndef MIDTERM_UTIL_H
#define MIDTERM_UTIL_H
#include "ls.h"
void *allocate(size_t n);
char *copy_string(const char *s);
char *join_path(const char *a, const char *b);
void report_error(const char *path);
#endif
