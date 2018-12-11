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
            MDIV, MMINUS, MNOT } kind;
    union {
        struct { struct Operand* right; struct Operand* left; } pair;
        struct { struct Operand* result; struct Operand* op1; struct Operand* op2; } binop;
    } u;
    //InterCode(struct Operand* assign_right, struct Operand* assign_left) {
    //    this->kind = ASSIGN;
    //    this->u.assign.right = assign_right;
    //    this->u.assign.left = assign_left;
    //}
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

// 语法树遍历
void search_tree(struct TreeNode* cur, struct TreeNode* father);

// 生成一个新的临时变量
struct Operand* new_temp();

// 将一段新的ics插入到codeshead的链表中
void add_codes(struct InterCodes* ics);

// 打印中间代码链表
void print_intercodes(struct InterCodes* head);
// 打印操作数
void print_operand(struct Operand* op);

#endif