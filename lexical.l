%option yylineno
yylineno = 1;

%{
  #include <stdio.h>
  #include <math.h>
  #include "syntax.tab.h"
  #include "datastruct.h"
  
  int yycolumn = 1;
  extern int printflag;
  #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng -1; \
    yycolumn += yyleng;
%}

/*definitions*/

digit [0-9]
_digit [1-9]
symbol [+-]
letter [a-zA-Z]
_letter [_a-zA-Z]
Oct [0-7]
_Oct [1-7]
Hex [0-9a-fA-F]
_Hex [1-9a-fA-F]

Int10 (0|({_digit}{digit}{0,31}))
Int8 (0{_Oct}{Oct}{0,31})
Int16 (0(x|X)(0|({_Hex}{Hex}{0,31})))
Int ({Int10}|{Int8}|{Int16})

Id ([_a-zA-Z][_a-zA-Z0-9]{0,31})
Type ("int"|"float")

%%
{Int10} { union Val v;
          v.intvalue = atoi(yytext);
          yylval = (struct TreeNode*)CreatNode(TERMINAL, "INT", yylineno, yycolumn, v);
          return INT; }
        
{Int8}  { unsigned int key;
          sscanf(yytext, "%o", &key);
          union Val v;
          v.intvalue = key;
          yylval = (struct TreeNode*)CreatNode(TERMINAL, "INT", yylineno, yycolumn, v);            
          return INT; }

{Int16} { unsigned int key;
          sscanf(yytext, "%x", &key);
          union Val v;
          v.intvalue = key;
          yylval = (struct TreeNode*)CreatNode(TERMINAL, "INT", yylineno, yycolumn, v);   
          return INT; }

(({digit}*\.{digit}+)|({digit}+\.))([eE]{symbol}?{Int})?   {   union Val v;
                                                      v.floatvalue = atof(yytext);
                                                      yylval = (struct TreeNode*)CreatNode(TERMINAL, "FLOAT", yylineno, yycolumn, v);
                                                      return FLOAT; }

{Type} {    union Val v;
            strcpy(v.strvalue, yytext);
            yylval = (struct TreeNode*)CreatNode(TERMINAL, "TYPE", yylineno, yycolumn, v);
            return TYPE; }

"struct"  { union Val v;
            v.intvalue = 0;
            yylval = (struct TreeNode*)CreatNode(TERMINAL, "STRUCT", yylineno, yycolumn, v);
            return STRUCT; }

"return"  { union Val v;
            v.intvalue = 0;
            yylval = (struct TreeNode*)CreatNode(TERMINAL, "RETURN", yylineno, yycolumn, v);
            return RETURN; }

"if"  { union Val v;
        v.intvalue = 0;
        yylval = (struct TreeNode*)CreatNode(TERMINAL, "IF", yylineno, yycolumn, v);
        return IF; }

"else"  { union Val v;
          v.intvalue = 0;
          yylval = (struct TreeNode*)CreatNode(TERMINAL, "ELSE", yylineno, yycolumn, v);
          return ELSE; }

"while" { union Val v;
          v.intvalue = 0;
          yylval = (struct TreeNode*)CreatNode(TERMINAL, "WHILE", yylineno, yycolumn, v);
          return WHILE; }

{Id}  { union Val v;
        strcpy(v.strvalue, yytext);
        yylval = (struct TreeNode*)CreatNode(TERMINAL, "ID", yylineno, yycolumn, v);
        return ID; }

";" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "SEMI", yylineno, yycolumn, v);
      return SEMI; }

"," { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "COMMA", yylineno, yycolumn, v);
      return COMMA; }

"=" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "ASSIGNOP", yylineno, yycolumn, v);
      return ASSIGNOP; }

">"|"<"|">="|"<="|"=="|"!=" { union Val v;
                              strcpy(v.strvalue, yytext);
                              yylval = (struct TreeNode*)CreatNode(TERMINAL, "RELOP", yylineno, yycolumn, v);
                              return RELOP; }

"+" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "PLUS", yylineno, yycolumn, v);
      return PLUS; }

"-" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "MINUS", yylineno, yycolumn, v);
      return MINUS; }

"*" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "STAR", yylineno, yycolumn, v);
      return STAR; }

"/" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "DIV", yylineno, yycolumn, v);
      return DIV; }

"&&"  { union Val v;
        v.intvalue = 0;
        yylval = (struct TreeNode*)CreatNode(TERMINAL, "AND", yylineno, yycolumn, v);
        return AND; }

"||"  { union Val v;
        v.intvalue = 0;
        yylval = (struct TreeNode*)CreatNode(TERMINAL, "OR", yylineno, yycolumn, v);
        return OR; }

"." { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "DOT", yylineno, yycolumn, v);
      return DOT; }

"!" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "NOT", yylineno, yycolumn, v);
      return NOT; }

"(" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "LP", yylineno, yycolumn, v);
      return LP; }

")" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "RP", yylineno, yycolumn, v);
      return RP; }

"[" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "LB", yylineno, yycolumn, v);
      return LB; }

"]" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "RB", yylineno, yycolumn, v);
      return RB; }

"{" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "LC", yylineno, yycolumn, v);
      return LC; }

"}" { union Val v;
      v.intvalue = 0;
      yylval = (struct TreeNode*)CreatNode(TERMINAL, "RC", yylineno, yycolumn, v);
      return RC; }

"//"  { char c = input();
        while(c != '\n') c = input(); }

"/*"  { int flag = 0;
        while(1) {
          char c = input();
          if(flag == 1) {
            if(c == '/')
              break;
            else
              flag = 0;
          }
          else
            if(c == '*')
              flag = 1; 
        } }

"\n"    { yycolumn = 1; }

" "     { ; }

.		{ printf("Error type A at Line %d, Column %d: Mysterious characters \"%s\"\n", yylineno, yycolumn, yytext); printflag = 0; }

%%

//user subroutines