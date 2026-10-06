#ifndef MIDTERM_LS_H
#define MIDTERM_LS_H
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdint.h>
typedef struct { int all, almost, longfmt, numeric, inode, blocks, classify;
 int recursive, directory, reverse, unsorted, sort, timekind, quote, units;
 uintmax_t blocksize; } Options;
typedef struct { char *name, *path; struct stat st; } Entry;
typedef struct { Entry *data; size_t count, capacity; } Entries;
extern Options opt;
extern int exit_status;
#include "util.h"
#include "options.h"
#include "entries.h"
#include "display.h"
#include "traversal.h"
#endif
