#ifndef MIDTERM_ENTRIES_H
#define MIDTERM_ENTRIES_H
#include "ls.h"
int add_entry(Entries *list, const char *name, const char *path, int operand);
void free_entries(Entries *list);
void sort_entries(Entries *list);
#endif
