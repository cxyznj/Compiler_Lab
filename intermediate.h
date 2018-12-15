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
    enum { MASSIGN, MRELOP, MADD, MSUB, MMUL, \
            MDIV, MRW, MFUNC, MRTFUNC, MARG, \
            MFUNCDEC, MRETURN, MPARAM, MGOTO, MLABEL, \
            MGADDRESS, MGVALUE, MDEC } kind;
    union {
        struct { struct Operand* right; struct Operand* left; } pair;
        // MRELOP: IF t1 op t2 GOTO label_true
        struct { struct Operand* t1; char* relopsym; struct Operand* t2; char* label; } relopgoto;
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
        // goto的标号
        char* gotolabel;
        // label的标号
        char* labelname;
        // MGADDRESS
        struct { struct Operand* x; struct Operand* y; } getaddress;
        // type == 0: x := *y
        // type == 1: *x := y
        struct { struct Operand* x; struct Operand* y; int type; } getvalue;
        struct { char* decname; int decsize; } dec;
    } u;
};

struct InterCodes {
    struct InterCode* code;
    struct InterCodes* pre;
    struct InterCodes* next;
};

struct CharList {
    char* charname;
    struct CharList* next;
};

// 安全创建结构体函数
struct Operand* create_operand();
struct InterCode* create_intercode();
struct InterCodes* create_intercodes();
struct CharList* create_charlist();

// 模块的接口函数
void generate_intercodes(struct TreeNode* tn);

// 翻译函数
struct InterCodes* translate_Exp(struct TreeNode* Exp, struct Operand* place);
struct InterCodes* translate_Args(struct TreeNode* Args, struct Operand** args_list, int* args_count);
struct InterCodes* translate_Param(struct TreeNode* VarList);
struct InterCodes* translate_Cond(struct TreeNode* Exp, char* label_true, char* label_false);
//struct InterCodes* translate_Stmt(struct TreeNode* Stmt);
//struct InterCodes* translate_CompSt(struct TreeNode* CompSt);

// 语法树遍历
void search_tree(struct TreeNode* cur, struct TreeNode* father, int child_flag, int sibling_flag);

// 生成一个新的临时变量
struct Operand* new_temp();
char* new_label();

struct Operand* new_constant(int val);

// 将一段新的ics插入到codeshead的链表中
void add_codes(struct InterCodes* ics);

// 生成一个LABEL的中间代码
struct InterCodes* create_label(char* labelname);

// 获取一个数组变量的最终地址
struct InterCodes* get_addr(struct TreeNode* Exp, struct Operand* arraddr);

// 判断当前的数组是否是一个参数传来的数组
int source_of_array(char* arrname);

// 打印中间代码链表
void print_intercodes(struct InterCodes* head);
void print_intercode(struct InterCode* ic);
// 打印操作数
void print_operand(struct Operand* op);

// 补全可能遗漏的pre指针
void remedy_pre();

// 比较两个Operand指针指向的内容是否相等
int operandcmp(struct Operand* t1, struct Operand* t2);

// 优化中间代码
void optimize_intercodes();

#endif