#include <stdio.h>
#include "syntax.tab.h"
#include "symbol.h"
#include "intermediate.h"
#include "mips32.h"

extern FILE* f;
int yyparse (void);
int yyrestart(FILE*);

int main(int argc, char** argv) {
    if (argc <= 2) return 1;
    FILE* f = fopen(argv[1], "r");
    //printf("%s\n", argv[2]);
    get_filename(argv[2]);
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    init_table();
    yyrestart(f);
    yyparse();
    return 0;
}
