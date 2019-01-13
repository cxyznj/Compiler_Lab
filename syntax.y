%locations

%expect 2

%{
    #define YYSTYPE struct TreeNode*
    #include <stdio.h>   
    #include "lex.yy.c"
    #include "datastruct.h"
    #include "symbol.h"
    #include "intermediate.h"
    #include "mips32.h"
    
    int yyerror(char* msg);
    int printflag = 1;
%}

%start Program

%nonassoc LOWER_THAN_RC LOWER_THAN_SEMI

%right ASSIGNOP

%left OR

%left AND

%left RELOP

%left PLUS MINUS

%left STAR DIV

%right NOT

%left LP RP LB RB DOT

%token INT FLOAT TYPE STRUCT RETURN IF ELSE WHILE ID
        SEMI COMMA LC RC

%nonassoc LOWER_THAN_ELSE

%nonassoc ELSE

%%

Program : ExtDefList {  // printf("Line %d, Column %d\n", yylloc.first_line, yylloc.first_column);
                        union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "Program", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1);
                        //if (printflag) printTree($$, 0);
                        build_vartable($$);
                        //check_program($$);
                        generate_intercodes($$);
                        generate_mips32code(); }
    ;
ExtDefList : ExtDef ExtDefList {    union Val v; v.intvalue = 0;
                                    $$ = CreatNode(NOTERMINAL, "ExtDefList", yyloc.first_line, yylloc.first_column, v);
                                    add_child($$, $1);
                                    add_sibling($1, $2); }
    | /* empty */ { union Val v; v.intvalue = 0;
                    $$ = CreatNode(EMPTY, "ExtDefList", yyloc.first_line, yylloc.first_column, v); }
    ;
ExtDef : Specifier ExtDecList SEMI {    union Val v; v.intvalue = 0;
                                        $$ = CreatNode(NOTERMINAL, "ExtDef", yyloc.first_line, yylloc.first_column, v);
                                        add_child($$, $1);
                                        add_sibling($1, $2);
                                        add_sibling($2, $3); }
    | Specifier SEMI {  union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "ExtDef", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1);
                        add_sibling($1, $2); }
    | Specifier FunDec CompSt { union Val v; v.intvalue = 0;
                                $$ = CreatNode(NOTERMINAL, "ExtDef", yyloc.first_line, yylloc.first_column, v);
                                add_child($$, $1);
                                add_sibling($1, $2);
                                add_sibling($2, $3); }
    | Specifier FunDec SEMI {   union Val v; v.intvalue = 0;
                                $$ = CreatNode(NOTERMINAL, "ExtDef", yyloc.first_line, yylloc.first_column, v);
                                add_child($$, $1);
                                add_sibling($1, $2);
                                add_sibling($2, $3);
                            }
    ;
ExtDecList : VarDec {   union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "ExtDecList", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1); }
    | VarDec COMMA ExtDecList { union Val v; v.intvalue = 0;
                                $$ = CreatNode(NOTERMINAL, "ExtDecList", yyloc.first_line, yylloc.first_column, v);
                                add_child($$, $1);
                                add_sibling($1, $2);
                                add_sibling($2, $3); }
    ;
Specifier : TYPE {  union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "Specifier", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1); }
    | StructSpecifier {     union Val v; v.intvalue = 0;
                            $$ = CreatNode(NOTERMINAL, "Specifier", yyloc.first_line, yylloc.first_column, v);
                            add_child($$, $1); }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC { union Val v; v.intvalue = 0;
                                                $$ = CreatNode(NOTERMINAL, "StructSpecifier", yyloc.first_line, yylloc.first_column, v);
                                                add_child($$, $1);
                                                add_sibling($1, $2);
                                                add_sibling($2, $3);
                                                add_sibling($3, $4);
                                                add_sibling($4, $5); }
    | STRUCT Tag {  union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "StructSpecifier", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1);
                    add_sibling($1, $2); }
    | STRUCT OptTag LC error RC { fprintf(stderr, "Error type B at line %d : %s\n", yylloc.first_line, "Missing \"}\""); }
    ;
OptTag : ID {   union Val v; v.intvalue = 0;
                $$ = CreatNode(NOTERMINAL, "OptTag", yyloc.first_line, yylloc.first_column, v);
                add_child($$, $1); }
    | /* empty */ { union Val v; v.intvalue = 0;
                    $$ = CreatNode(EMPTY, "OptTag", yyloc.first_line, yylloc.first_column, v); }
    ;
Tag : ID {  union Val v; v.intvalue = 0;
            $$ = CreatNode(NOTERMINAL, "Tag", yyloc.first_line, yylloc.first_column, v);
            add_child($$, $1); }
    ;
VarDec : ID {   union Val v; v.intvalue = 0;
                $$ = CreatNode(NOTERMINAL, "VarDec", yyloc.first_line, yylloc.first_column, v);
                add_child($$, $1); }
    | VarDec LB INT RB {    union Val v; v.intvalue = 0;
                            $$ = CreatNode(NOTERMINAL, "VarDec", yyloc.first_line, yylloc.first_column, v);
                            add_child($$, $1);
                            add_sibling($1, $2);
                            add_sibling($2, $3);
                            add_sibling($3, $4); }
    | VarDec LB error RB { fprintf(stderr, "Error type B at line %d : %s\n", yylloc.first_line, "Missing \"]\""); }                        
    ;
FunDec : ID LP VarList RP { union Val v; v.intvalue = 0;
                            $$ = CreatNode(NOTERMINAL, "FunDec", yyloc.first_line, yylloc.first_column, v);
                            add_child($$, $1);
                            add_sibling($1, $2);
                            add_sibling($2, $3);
                            add_sibling($3, $4); }
    | ID LP RP {    union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "FunDec", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1);
                    add_sibling($1, $2);
                    add_sibling($2, $3); }
    ;
VarList : ParamDec COMMA VarList {  union Val v; v.intvalue = 0;
                                    $$ = CreatNode(NOTERMINAL, "VarList", yyloc.first_line, yylloc.first_column, v);
                                    add_child($$, $1);
                                    add_sibling($1, $2);
                                    add_sibling($2, $3); }
    | ParamDec {    union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "VarList", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1); }
ParamDec : Specifier VarDec {   union Val v; v.intvalue = 0;
                                $$ = CreatNode(NOTERMINAL, "ParamDec", yyloc.first_line, yylloc.first_column, v);
                                add_child($$, $1);
                                add_sibling($1, $2); }
    ;
CompSt : LC DefList StmtList RC {   union Val v; v.intvalue = 0;
                                    $$ = CreatNode(NOTERMINAL, "CompSt", yyloc.first_line, yylloc.first_column, v);
                                    add_child($$, $1);
                                    add_sibling($1, $2);
                                    add_sibling($2, $3);
                                    add_sibling($3, $4); }
    | LC error RC %prec LOWER_THAN_RC { fprintf(stderr, "Error type B at line %d : %s\n", yylloc.first_line, "Missing \"}\""); }
    ;
StmtList : Stmt StmtList {  union Val v; v.intvalue = 0;
                            $$ = CreatNode(NOTERMINAL, "StmtList", yyloc.first_line, yylloc.first_column, v);
                            add_child($$, $1);
                            add_sibling($1, $2); }
    | /* empty */ { union Val v; v.intvalue = 0;
                    $$ = CreatNode(EMPTY, "StmtList", yyloc.first_line, yylloc.first_column, v); }
                    
    ;
Stmt : Exp SEMI {   union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "Stmt", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1); 
                    add_sibling($1, $2); }
    | CompSt {  union Val v; v.intvalue = 0;
                $$ = CreatNode(NOTERMINAL, "Stmt", yyloc.first_line, yylloc.first_column, v);
                add_child($$, $1); }
    | RETURN Exp SEMI { union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "Stmt", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1); 
                        add_sibling($1, $2);
                        add_sibling($2, $3); }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { union Val v; v.intvalue = 0;
                                                $$ = CreatNode(NOTERMINAL, "Stmt", yyloc.first_line, yylloc.first_column, v);
                                                add_child($$, $1); 
                                                add_sibling($1, $2);
                                                add_sibling($2, $3);
                                                add_sibling($3, $4);
                                                add_sibling($4, $5); }
    | IF LP Exp RP Stmt ELSE Stmt { union Val v; v.intvalue = 0;
                                    $$ = CreatNode(NOTERMINAL, "Stmt", yyloc.first_line, yylloc.first_column, v);
                                    add_child($$, $1); 
                                    add_sibling($1, $2);
                                    add_sibling($2, $3);
                                    add_sibling($3, $4);
                                    add_sibling($4, $5);
                                    add_sibling($5, $6);
                                    add_sibling($6, $7); }
    | WHILE LP Exp RP Stmt {    union Val v; v.intvalue = 0;
                                $$ = CreatNode(NOTERMINAL, "Stmt", yyloc.first_line, yylloc.first_column, v);
                                add_child($$, $1);
                                add_sibling($1, $2);
                                add_sibling($2, $3);
                                add_sibling($3, $4);
                                add_sibling($4, $5); }
    | error SEMI { fprintf(stderr, "Error type B at line %d : %s\n", yylloc.first_line, "Missing \";\""); }
    ;
DefList : Def DefList { union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "DefList", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1);
                        add_sibling($1, $2); }
    | /* empty */ { union Val v; v.intvalue = 0;
                    $$ = CreatNode(EMPTY, "DefList", yyloc.first_line, yylloc.first_column, v); }
    ;
Def : Specifier DecList SEMI {  union Val v; v.intvalue = 0;
                                $$ = CreatNode(NOTERMINAL, "Def", yyloc.first_line, yylloc.first_column, v);
                                add_child($$, $1);
                                add_sibling($1, $2);
                                add_sibling($2, $3); }
    | error SEMI %prec LOWER_THAN_SEMI { fprintf(stderr, "Error type B at line %d : %s\n", yylloc.first_line, "Missing \";\""); }
    ;
DecList : Dec { union Val v; v.intvalue = 0;
                $$ = CreatNode(NOTERMINAL, "DecList", yyloc.first_line, yylloc.first_column, v);
                add_child($$, $1); }
    | Dec COMMA DecList {   union Val v; v.intvalue = 0;
                            $$ = CreatNode(NOTERMINAL, "DecList", yyloc.first_line, yylloc.first_column, v);
                            add_child($$, $1);
                            add_sibling($1, $2);
                            add_sibling($2, $3); }
    ;
Dec : VarDec {  union Val v; v.intvalue = 0;
                $$ = CreatNode(NOTERMINAL, "Dec", yyloc.first_line, yylloc.first_column, v);
                add_child($$, $1); }
    | VarDec ASSIGNOP Exp { union Val v; v.intvalue = 0;
                            $$ = CreatNode(NOTERMINAL, "Dec", yyloc.first_line, yylloc.first_column, v);
                            add_child($$, $1);
                            add_sibling($1, $2);
                            add_sibling($2, $3); }
    ;
Exp : Exp ASSIGNOP Exp {    union Val v; v.intvalue = 0;
                            $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                            add_child($$, $1);
                            add_sibling($1, $2);
                            add_sibling($2, $3); }
    | Exp AND Exp { union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1);
                    add_sibling($1, $2);
                    add_sibling($2, $3); }
    | Exp OR Exp {  union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1);
                    add_sibling($1, $2);
                    add_sibling($2, $3); }
    | Exp RELOP Exp {   union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1);
                        add_sibling($1, $2);
                        add_sibling($2, $3); }
    | Exp PLUS Exp {    union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1);
                        add_sibling($1, $2);
                        add_sibling($2, $3); }
    | Exp MINUS Exp {   union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1);
                        add_sibling($1, $2);
                        add_sibling($2, $3); }
    | Exp STAR Exp {    union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1);
                        add_sibling($1, $2);
                        add_sibling($2, $3); }
    | Exp DIV Exp { union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1);
                    add_sibling($1, $2);
                    add_sibling($2, $3); }
    | LP Exp RP {   union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1);
                    add_sibling($1, $2);
                    add_sibling($2, $3); }
    | MINUS Exp {   union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1);
                    add_sibling($1, $2); }
    | NOT Exp { union Val v; v.intvalue = 0;
                $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                add_child($$, $1);
                add_sibling($1, $2); }
    | ID LP Args RP {   union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1);
                        add_sibling($1, $2);
                        add_sibling($2, $3);
                        add_sibling($3, $4); }
    | ID LP RP {    union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1);
                    add_sibling($1, $2);
                    add_sibling($2, $3); }
    | Exp LB Exp RB {   union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1);
                        add_sibling($1, $2);
                        add_sibling($2, $3); 
                        add_sibling($3, $4); }
    | Exp DOT ID {  union Val v; v.intvalue = 0;
                    $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                    add_child($$, $1);
                    add_sibling($1, $2);
                    add_sibling($2, $3); }
    | ID {  union Val v; v.intvalue = 0;
            $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
            add_child($$, $1); }
    | INT { union Val v; v.intvalue = 0;
            $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
            add_child($$, $1); }
    | FLOAT {   union Val v; v.intvalue = 0;
                $$ = CreatNode(NOTERMINAL, "Exp", yyloc.first_line, yylloc.first_column, v);
                add_child($$, $1); }
    | LP error RP { fprintf(stderr, "Error type B at line %d : %s\n", yylloc.first_line, "Missing \")\""); }
    | Exp LB error RB { fprintf(stderr, "Error type B at line %d : %s\n", yylloc.first_line, "Missing \"]\""); }

    ;
Args : Exp COMMA Args { union Val v; v.intvalue = 0;
                        $$ = CreatNode(NOTERMINAL, "Args", yyloc.first_line, yylloc.first_column, v);
                        add_child($$, $1);
                        add_sibling($1, $2);
                        add_sibling($2, $3); }
    | Exp { union Val v; v.intvalue = 0;
            $$ = CreatNode(NOTERMINAL, "Args", yyloc.first_line, yylloc.first_column, v);
            add_child($$, $1); }
    ;
%%

int yyerror(char* msg) {
    printflag = 0;
    fprintf(stderr, "Error type B at line %d, column %d : %s\n", yylloc.first_line, yylloc.first_column, msg);
}