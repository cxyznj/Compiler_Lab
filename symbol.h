#ifndef _SYMBOL_H_

#define _SYMBOL_H_

#include "datastruct.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// 变量类型
struct Type {
    enum { BASIC, ARRAY, STRUCTURE } kind;
    union {
        // 基本类型: 1代表int, 2代表float
        int basic;
        // 数组类型信息：包括元素类型与数组大小
        struct { struct Type* elem; int size; } array;
        // 结构体信息（链表形式）
        struct FieldList* structure;
    } u;
    int constant;
};

// 结构体专用的域表
struct FieldList {
    // 域的名字
    char name[50];
    // 域的类型
    struct Type type;
    // 下一个域的指针
    struct FieldList* next;
};

// 结构体表
struct StructTable {
    // 结构体的名字
    char name[50];
    // 结构体的域
    struct FieldList* structure;
    // 结构体表下一项的指针
    struct StructTable* next;
};

// 变量符号表
struct VarTable {
    // 变量名
    char name[50];    
    // 变量类型
    struct Type vartype;
    // 符号表下一项的指针
    struct VarTable* next;
};

// 函数基本属性
struct FunctionType {
    // 函数的返回类型
    struct Type rt_value;
    // 函数名
    char name[50];
    // 参数表
    struct VarTable* paratable;
};

// 函数符号表
struct FunctionTable {
    // 当前函数的属性
    struct FunctionType* func;
    // 当前函数是否有定义的标识，state = 1表示有定义，0表示还没有定义
    int state;
    // 标识函数定义或声明（定义前）的行号
    int row;
    // 函数表下一项的指针
    struct FunctionTable* next;
};

// ----一些变量----
// 变量符号表
struct VarTable* vartablehead;
// 结构体表
struct StructTable* strutablehead;
// 给无名结构体的命名种子
int emptyname;
// 函数符号表
struct FunctionTable* functablehead;

// ----全局函数----
// 初始化符号表（给符号表的头项分配空间）
int init_table();
// 构建符号表
void build_vartable(struct TreeNode* tn);
// 检查程序中的语义错误
void check_program(struct TreeNode* tn);
// 检查是否有已声明但未定义的函数
void check_function();

// ----安全malloc辅助函数----
// 安全地新建一个结构体项
struct Type* create_type();
struct FieldList* create_fieldlist();
struct StructTable* create_structtable();
struct VarTable* create_vartable();
struct FunctionType* create_functiontype();
struct FunctionTable* create_functiontable();

// ----遍历----
// 为vartable实现的树的遍历
void find_vartable(struct TreeNode* cur, struct TreeNode* father, char* type, int* arr, int arrdepth);
// 为check_program实现的树的遍历
void check_error(struct TreeNode* cur, struct TreeNode* father);

// ----功能辅助函数----
// 处理结构体中的一条DecList
void add_onedeclist(char* type_name, struct FieldList** starfl, struct TreeNode* declist);
// 检查符号表中是否有重复定义
int check_varconflict(char* varname);
// 检查结构体中是否有重名的域
int check_struconflict(char* varname, struct FieldList* fl);

// ----查找函数----
// 返回对应名struct类型的域
struct FieldList* get_fieldlist(char* struname, int rownum);
// 查找符号表，若找到返回对应的类型，否则返回NULL
struct Type* search_vartable(char* varname);
// 查找结构体表，若找到返回1
int search_strutable(char* struname);
// 查找函数表，若找到返回对应的函数类型信息，找不到返回NULL
struct FunctionType* search_functable(char* funcname);
// 返回Exp节点的属性Type
struct Type* get_exp_type(struct TreeNode* exp);
// 在一个stmtlist中查找return语句，判断return的返回值是否与rttype相等，否则报错
void find_return_in_stmtlist(struct TreeNode* stmtlist, struct Type* rttype);
// 在一个stmt中查找return语句，判断return的返回值是否与rttype相等，否则报错
void find_return_in_stmt(struct TreeNode* stmt, struct Type* rttype);

// ----模式匹配检查函数----
// 检查两个操作数类型是否匹配
int match_variate(struct Type* type1, struct Type* type2);
// 检查两个参数表是否匹配
int match_parameter(struct VarTable* vt1, struct VarTable* vt2);

// ----打印表格函数
// 打印符号表
void print_vartable();
// 打印类型体
void print_type(struct Type* head);
// 打印结构体表
void print_strutable();
// 打印函数表
void print_functable();
// 打印一个exp
void print_exp(struct TreeNode* exp);

#endif