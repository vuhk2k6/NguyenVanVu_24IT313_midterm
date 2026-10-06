#include "ls.h"
#include <stdio.h>
#include <locale.h>
int main(int argc,char **argv) {
    Entries files={0},dirs={0}; setlocale(LC_CTYPE,"");
    int start=parse_options(argc,argv), operands=argc-start;
    for (int i=start;i<argc || (!operands && i==start);i++) {
        const char *path=operands?argv[i]:".";
        Entries one={0};
        if (!add_entry(&one,path,path,1)) continue;
        Entries *dest=(!opt.directory && S_ISDIR(one.data[0].st.st_mode))?&dirs:&files;
        /* Reuse the normal collector, then release the temporary entry. */
        add_entry(dest,path,path,1); free_entries(&one);
    }
    sort_entries(&files); sort_entries(&dirs);
    for (size_t i=0;i<files.count;i++) print_entry(&files.data[i]);
    if (files.count && dirs.count) putchar('\n');
    for (size_t i=0;i<dirs.count;i++) list_directory(&dirs.data[i],operands>1 || opt.recursive);
    free_entries(&files); free_entries(&dirs);
    if (fflush(stdout)==EOF || ferror(stdout)) { perror("myls: stdout"); exit_status=1; }
    return exit_status;
}
