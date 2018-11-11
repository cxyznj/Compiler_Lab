#ifndef symbol

#define symbol

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
    struct FieldList structure;
    // 结构体表下一项的指针
    struct StructTable* next;
};

// 变量（结构体）符号表
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
    struct FunctionType func;
    // 当前函数是否有定义的标识，state = 1表示有定义，0表示还没有定义
    int state;
    // 函数表下一项的指针
    struct FunctionTable* next;
};

// 变量符号表
struct VarTable* vartablehead;
// 结构体表
struct StructTable* strutablehead;
// 给无名结构体的命名种子
int emptyname;
// 函数符号表
struct FunctionTable* functablehead;

// 初始化符号表（给符号表的头项分配空间）
int init_table();
// 构建变量符号表
void build_vartable(struct TreeNode* tn);
// 构建函数符号表
void build_functable(struct TreeNode* tn);
// 安全地新建一个结构体项
struct Type* create_type();
struct FieldList* create_fieldlist();
struct StructTable* create_structtable();
struct VarTable* create_vartable();
struct FunctionType* create_functiontype();
struct FunctionTable* create_functiontable();

// 为vartable实现的树的遍历
void find_vartable(struct TreeNode* cur, struct TreeNode* father, char* type, int* arr, int arrdepth);
// 打印符号表
void print_vartable();
// 打印类型体
void print_type(struct Type* head);
#endif