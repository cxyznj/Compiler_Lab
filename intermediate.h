#ifndef _INTERMEDIATE_H_

#define _INTERMEDIATE_H_

#include "symbol.h"
#include "datastruct.h"
#include <stdio.h>

struct Operand {
    enum { VARIABLE, CONSTANT, ADDRESS, TEMP } kind;
    union {
        struct VarTable* var;
        int ivalue;
        int tno;
    } u;
};

struct InterCode {
    enum { ASSIGN, AND, OR, RELOP, ADD, SUB, MUL, \
            DIV, MINUS, NOT } kind;
    union {
        struct { struct Operand* right; struct Operand* left; } assign;
        struct { struct Operand* result, op1, op2; } binop;
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

struct InterCodes* codeshead = NULL;
int tempno = 1;

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
void add_codes(struct InterCodes* ics, struct InterCodes* head);

// 打印中间代码链表
void print_intercodes(struct InterCodes* head);
// 打印操作数
void print_operand(struct Operand* op);

#endif