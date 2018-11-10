#include "symbol.h"

// 初始化符号表（给符号表的头项分配空间）
int init_table() {
    vartablehead = create_vartable();
    functablehead = create_functiontable();
    return 1;
}

// 构建变量符号表
void build_vartable(struct TreeNode* tn) {

}

// 构建函数符号表
void build_functable(struct TreeNode* tn) {

}

// 安全地新建一个结构体项
struct Type* create_type() {
    struct Type* rt = malloc(sizeof(struct Type));
    rt->kind = BASIC;
    rt->u.basic = 1;
    return rt;
}
struct FieldList* create_fieldlist() {
    struct FieldList* rt = malloc(sizeof(struct FieldList));
    rt->name[0] = '\n';
    rt->type = NULL;
    rt->next = NULL;
    return rt;
}
struct VarTable* create_vartable() {
    struct VarTable* rt = malloc(sizeof(struct VarTable));
    rt->name[0] = '\n';
    rt->vartype = NULL;
    rt->next = NULL;
    return rt;
}
struct FunctionType* create_functiontype() {
    struct FunctionType* rt = malloc(sizeof(struct FunctionType));
    rt->rt_value = NULL;
    rt->name[0] = '\n';
    rt->paratable = NULL;
    return rt;
}
struct FunctionTable* create_functiontable() {
    struct FunctionTable* rt = malloc(sizeof(struct FunctionTable));
    rt->func = NULL;
    rt->next = NULL;
    return rt;
}