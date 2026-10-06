#include "ls.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
Options opt;
/* BLOCKSIZE accepts bytes, or K/M/G suffixes; invalid values use 512. */
static uintmax_t environment_blocks(void) {
    const char *s = getenv("BLOCKSIZE"); char *end; unsigned long long n;
    uintmax_t mult = 1;
    if (!s || !*s || *s == '-') return 512;
    errno = 0; n = strtoull(s, &end, 10);
    if (end == s) n = 1;
    if (*end) {
        switch (*end++) {
        case 'k': case 'K': mult = 1024; break;
        case 'm': case 'M': mult = 1024*1024; break;
        case 'g': case 'G': mult = 1024ULL*1024*1024; break;
        default: return 512;
        }
        if (*end == 'b' || *end == 'B') end++;
    }
    if (errno || *end || !n || n > UINTMAX_MAX/mult) return 512;
    return n*mult;
}
int parse_options(int argc, char **argv) {
    int c;
    opt.almost = geteuid() == 0; opt.quote = isatty(STDOUT_FILENO);
    opt.blocksize = environment_blocks();
    while ((c = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (c) {
        case 'A': opt.almost=1; break;
        case 'a': opt.all=1; break;
        case 'c': opt.timekind=1; break;
        case 'u': opt.timekind=2; break;
        case 'd': opt.directory=1; opt.recursive=0; break;
        case 'R': opt.recursive=1; opt.directory=0; break;
        case 'F': opt.classify=1; break;
        case 'f': opt.unsorted=1; break;
        case 'h': opt.units=1; break;
        case 'k': opt.units=2; break;
        case 'i': opt.inode=1; break;
        case 's': opt.blocks=1; break;
        case 'l': opt.longfmt=1; opt.numeric=0; break;
        case 'n': opt.longfmt=1; opt.numeric=1; break;
        case 'q': opt.quote=1; break;
        case 'w': opt.quote=0; break;
        case 'r': opt.reverse=1; break;
        case 'S': opt.sort=1; break;
        case 't': opt.sort=2; break;
        default: fprintf(stderr,"usage: myls [-AacdFfhiklnqRrSstuw] [file ...]\n"); exit(1);
        }
    }
    return optind;
}
