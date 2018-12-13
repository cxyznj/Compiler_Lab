#ifndef _INTERMEDIATE_H

#define _INTERMEDIATE_H

#include "symbol.h"
#include "datastruct.h"
#include <stdio.h>

struct Operand {
    enum { VARIABLE, CONSTANT, ADDRESS, TEMP } kind;
    union {
        char* varname;
        int ivalue;
        char* addname;
        int tno;
    } u;
};

struct InterCode {
    enum { MASSIGN, MAND, MOR, MRELOP, MADD, MSUB, MMUL, \
            MDIV, MMINUS, MNOT, MRW, MFUNC, MRTFUNC, MARG, \
            MFUNCDEC, MRETURN, MPARAM } kind;
    union {
        struct { struct Operand* right; struct Operand* left; } pair;
 
        struct { struct Operand* result; struct Operand* op1; struct Operand* op2; } binop;
        // MRW表示Read或Write函数
        struct { int rwflag; struct Operand* op1; } rwfunc;
        // 不带参数的函数的调用或函数的起始点，使用前记得分配内存！
        char* funcname;
        // 带参数的函数的调用
        struct { char* funcname; struct Operand* result; } rtfunc;
        // 参数的压栈
        struct Operand* arg;
        // 函数返回值
        struct Operand* rtval;
        // 参数的取出
        char* paramname;
    } u;
};

struct InterCodes {
    struct InterCode* code;
    struct InterCodes* pre;
    struct InterCodes* next;
};

// 安全创建结构体函数
struct Operand* create_operand();
struct InterCode* create_intercode();
struct InterCodes* create_intercodes();

// 模块的接口函数
void generate_intercodes(struct TreeNode* tn);

// 翻译函数
struct InterCodes* translate_Exp(struct TreeNode* Exp, struct Operand* place);
struct InterCodes* translate_Args(struct TreeNode* Args, struct Operand** args_list, int* args_count);
struct InterCodes* translate_Param(struct TreeNode* VarList);

// 语法树遍历
void search_tree(struct TreeNode* cur, struct TreeNode* father);

// 生成一个新的临时变量
struct Operand* new_temp();
char* new_label();

// 将一段新的ics插入到codeshead的链表中
void add_codes(struct InterCodes* ics);

// 打印中间代码链表
void print_intercodes(struct InterCodes* head);
// 打印操作数
void print_operand(struct Operand* op);

#endif