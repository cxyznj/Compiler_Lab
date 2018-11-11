#include "symbol.h"

// 初始化符号表（给符号表的头项分配空间）
int init_table() {
    vartablehead = NULL;
    strutablehead = NULL;
    emptyname = 1;
    functablehead = NULL;
    return 1;
}

// 构建变量符号表
void build_vartable(struct TreeNode* tn) {
    find_vartable(tn, NULL, NULL, NULL, 0);
    print_vartable();
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
    rt->next = NULL;
    return rt;
}
struct StructTable* create_structtable() {
    struct StructTable* rt = malloc(sizeof(struct StructTable));
    rt->name[0] = '\n';
    rt->next = NULL;
    return rt;
}
struct VarTable* create_vartable() {
    struct VarTable* rt = malloc(sizeof(struct VarTable));
    rt->name[0] = '\n';
    rt->next = NULL;
    return rt;
}
struct FunctionType* create_functiontype() {
    struct FunctionType* rt = malloc(sizeof(struct FunctionType));
    rt->name[0] = '\n';
    rt->paratable = NULL;
    return rt;
}
struct FunctionTable* create_functiontable() {
    struct FunctionTable* rt = malloc(sizeof(struct FunctionTable));
    rt->state = 0;
    rt->next = NULL;
    return rt;
}

void find_vartable(struct TreeNode* cur, struct TreeNode* father, char* type, int* arr, int arrdepth){
    // 错误检查
    assert(arrdepth < 10);

    if(cur == NULL || cur->ttype == EMPTY)
        return;

    printf("%s\n", cur->name);

    if(strcmp(cur->name, "Specifier") == 0) {
        // 基本类型
        if(strcmp(cur->child->name, "TYPE") == 0) {
            char* typename = malloc(sizeof(char) * 50);
            strcpy(typename, cur->child->val.strvalue);
            find_vartable(cur->sibling, cur, typename, arr, arrdepth);
        }
        // 结构体的引用
        else if(strcmp(cur->child->child->sibling->name, "Tag") == 0) {
            char* typename = malloc(sizeof(char) * 50);
            strcpy(typename, cur->child->child->sibling->child->val.strvalue);
            find_vartable(cur->sibling, cur, typename, arr, arrdepth);
        }
        // TODO:结构体的构造
        else {
            char* struname = malloc(sizeof(char) * 50);
            // 如果OptTag不推导到空
            if(cur->child->child->sibling->child != NULL)
                strcpy(struname, cur->child->child->sibling->child->val.strvalue);
            else {
                sprintf(struname, "%d", emptyname);
                emptyname++;
            }

        }
    }
    else if(strcmp(cur->name, "ExtDecList") == 0) {
        // VarDec
        find_vartable(cur->child, cur, type, arr, arrdepth);
        // 如果推导式是ExtDecList ::= VarDec COMMA ExtDecList
        if(cur->child->sibling != NULL) {
            find_vartable(cur->child->sibling->sibling, cur, type, arr, arrdepth);
        }
    }
    else if((strcmp(cur->name, "VarDec") == 0) && (strcmp(father->name, "ParamDec") != 0)) {
        if(strcmp(cur->child->name, "ID") == 0) {
            // TODO:检查符号表中是否有冲突表项

            // 向符号表中增加新表项
            printf("new a table. type = %s, arrdepth = %d\n", type, arrdepth);
            struct VarTable* nvartable = create_vartable();
            strcpy(nvartable->name, cur->child->val.strvalue);
            if(strcmp(type, "int") == 0) {
                if(arrdepth == 0) {
                    nvartable->vartype.kind = BASIC;
                    nvartable->vartype.u.basic = 1;
                }
                else {
                    nvartable->vartype.kind = ARRAY;
                    // TODO:处理数组情况
                    nvartable->vartype.u.array.size = arr[arrdepth - 1];
                    //nvartable->vartype.u.array.elem = create_type();
                    struct Type* elem = &(nvartable->vartype);
                    int i = arrdepth - 2;
                    for( ; i >= 0; i--) {
                        struct Type* ntype = create_type();
                        ntype->kind = ARRAY;
                        ntype->u.array.size = arr[i];
                        elem->u.array.elem = ntype;
                        elem = ntype;
                    }
                    elem->u.array.elem = create_type();
                    elem->u.array.elem->kind = BASIC;
                    elem->u.array.elem->u.basic = 1;
                }
            }
            else if(strcmp(type, "float") == 0) {
                if(arrdepth == 0) {
                    nvartable->vartype.kind = BASIC;
                    nvartable->vartype.u.basic = 2;
                }
                else {
                    nvartable->vartype.kind = ARRAY;
                    // TODO:处理数组情况
                    nvartable->vartype.u.array.size = arr[arrdepth - 1];
                    //nvartable->vartype.u.array.elem = create_type();
                    struct Type* elem = &(nvartable->vartype);
                    int i = arrdepth - 2;
                    for( ; i >= 0; i--) {
                        struct Type* ntype = create_type();
                        ntype->kind = ARRAY;
                        ntype->u.array.size = arr[i];
                        elem->u.array.elem = ntype;
                        elem = ntype;
                    }
                    elem->u.array.elem = create_type();
                    elem->u.array.elem->kind = BASIC;
                    elem->u.array.elem->u.basic = 2;                    
                }
            }
            // 用户struct的类型
            else {
                nvartable->vartype.kind = STRUCTURE;
                // TODO:在符号表中找到名为nvartable的struct项，然后将FieldList复制进来

            }
            if(vartablehead == NULL) {
                vartablehead = nvartable;
            }
            else {
                nvartable->next = vartablehead;
                vartablehead = nvartable;
            }
        }
        // VarDec ::= VarDec LB INT RB,这是一个数组类型
        else {
            // 新建数组项
            if(arr == NULL) {
                int* narr = malloc(sizeof(int) * 10);
                narr[arrdepth] = cur->child->sibling->sibling->val.intvalue;
                find_vartable(cur->child, cur, type, narr, arrdepth + 1);
            }
            // 已有数组项
            else {
                arr[arrdepth] = cur->child->sibling->sibling->val.intvalue;
                find_vartable(cur->child, cur, type, arr, arrdepth + 1);
            }
        }
    }
    else {
        find_vartable(cur->child, cur, type, arr, arrdepth);
        find_vartable(cur->sibling, father, type, arr, arrdepth);
    }
}
void print_vartable() {
    while(vartablehead != NULL) {
        printf("%s: ",vartablehead->name);
        print_type(&vartablehead->vartype);
        printf("\n");
        vartablehead = vartablehead->next;
    }
}
void print_type(struct Type* head) {
    if(head->kind == BASIC) {
        if(head->u.basic == 1)
            printf("int");
        else 
            printf("float");
    }
    else if(head->kind = ARRAY) {
        struct Type* t = head;
        while(t->kind == ARRAY) {
            printf("[%d]", t->u.array.size);
            t = t->u.array.elem;
        }
        if(t->u.basic == 1)
            printf("int");
        else 
            printf("float");
    }
    else if(head->kind = STRUCTURE) {
        printf("struct{ ");
        struct FieldList* fl = head->u.structure;
        while(fl != NULL) {
            printf("%s ", fl->name);
            print_type(&fl->type);
            printf("\n");
            fl = fl->next;
        }
        printf("}\n");
    }
}