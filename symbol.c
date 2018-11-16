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
    print_strutable();
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
    rt->structure = NULL;
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
    // ----空结点的跳过处理----
    if(cur == NULL || cur->ttype == EMPTY)
        return;

    // ----错误检查----
    // 数组维度最高为10
    assert(arrdepth < 10);
    //printf("%s\n", cur->name);

    // ----遍历到特定结点时，执行操作----
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
        // 结构体的构造
        else {
            char* struname = malloc(sizeof(char) * 50);
            // 如果OptTag不推导到空
            if(cur->child->child->sibling->child != NULL)
                strcpy(struname, cur->child->child->sibling->child->val.strvalue);
            // OptTag推导到空，我们给其赋予一个独特的名字
            else {
                sprintf(struname, "%d", emptyname);
                emptyname++;
            }

            // TODO: 检查是否有重名结构体

            // 将这个结构体放入结构体表中
            // TODO:默认结构体没有嵌套
            struct StructTable* nst = create_structtable();
            strcpy(nst->name, struname);
            // 找到DefList
            struct TreeNode* deflist = cur->child->child->sibling->sibling->sibling;
            // 用指向指针的指针来在函数中改变fl指针的值            
            struct FieldList* fl = NULL;
            struct FieldList** starfl = &fl;

            // 选择DefList ::= Def DefList的推导式而不是DefList ::= /*empty*/的推导式
            while(deflist->ttype != EMPTY) {
                struct TreeNode* def = deflist->child;
                add_onedeclist(def->child->child->val.strvalue, starfl, def->child->sibling);
                deflist = def->sibling;
            }
            nst->structure = fl;
            // TODO:检查结构体中是否有重名的域
            printf("Add a new struct: %s\n", struname);
            if(strutablehead == NULL)
                strutablehead = nst;
            else {
                struct StructTable* st = strutablehead;
                while(st->next != NULL) st = st->next;
                st->next = nst;
            }

            find_vartable(cur->sibling, cur, struname, arr, arrdepth);
        }
    }
    else if((strcmp(cur->name, "VarDec") == 0) && (strcmp(father->name, "ParamDec") != 0)) {
        if(strcmp(cur->child->name, "ID") == 0) {
            // ----检查符号表中是否有冲突（已定义）表项----
            int ccflag = check_varconflict(cur->child->val.strvalue);
            if(ccflag == 1) {
                printf("\033[;31mError type 3 at Line %d: Redefined variable \"%s\".\033[0m\n", cur->situation[0], cur->child->val.strvalue);
                return;
            }

            // ----向符号表中增加新表项----
            //printf("new a table. type = %s, arrdepth = %d\n", type, arrdepth);
            struct VarTable* nvartable = create_vartable();
            strcpy(nvartable->name, cur->child->val.strvalue);

            if(strcmp(type, "int") == 0) {
                // 数组维度为0：基本类型
                if(arrdepth == 0) {
                    nvartable->vartype.kind = BASIC;
                    nvartable->vartype.u.basic = 1;
                }
                // 数组维度不为0：数组类型
                else {
                    nvartable->vartype.kind = ARRAY;
                    nvartable->vartype.u.array.size = arr[arrdepth - 1];
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
                // 数组维度为0：基本类型                
                if(arrdepth == 0) {
                    nvartable->vartype.kind = BASIC;
                    nvartable->vartype.u.basic = 2;
                }
                // 数组维度不为0：数组类型                
                else {
                    nvartable->vartype.kind = ARRAY;
                    nvartable->vartype.u.array.size = arr[arrdepth - 1];
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
                struct StructTable* fst = strutablehead;
                // 在符号表中找到名为nvartable的struct项，然后将FieldList复制进来
                while(fst != NULL && strcmp(fst->name, type) != 0) fst = fst->next;
                // TODO:处理找不到结构体定义的情况
                assert(fst != NULL);
                
                nvartable->vartype.u.structure = fst->structure;
            }
            // 给符号表添加一个新项（放在链尾）
            if(vartablehead == NULL) {
                vartablehead = nvartable;
            }
            else {
                struct VarTable *vt = vartablehead;
                while(vt->next != NULL) vt = vt->next;
                vt->next = nvartable;
            }
        }
        // VarDec ::= VarDec LB INT RB,这是一个数组类型
        else {
            // 如果还没为数组分配空间，分配空间（TODO:这里有可能有传参导致的问题，要谨慎处理）
            if(arr == NULL)
                arr = malloc(sizeof(int) * 10);
            arr[arrdepth] = cur->child->sibling->sibling->val.intvalue;
            find_vartable(cur->child, cur, type, arr, arrdepth + 1);
        }
    }
    // ----没有特殊情况，则做正常的深度遍历----
    else {
        find_vartable(cur->child, cur, type, arr, arrdepth);
        find_vartable(cur->sibling, father, type, arr, arrdepth);
    }
}

int check_varconflict(char* varname) {
    struct VarTable* vt = vartablehead;
    while(vt != NULL) {
        if(strcmp(varname, vt->name) == 0)
            return 1;
        vt = vt->next;
    }
    return 0;
}

void add_onedeclist(char* type_name, struct FieldList** starfl, struct TreeNode* declist) {
    while(1) {
        struct TreeNode* vardec = declist->child->child;

        int arr[10]; int arrdepth = 0;

        // 把VarDec ::= VarDec LB INT RB 的情况处理完
        while(strcmp(vardec->child->name, "VarDec") == 0) {
            arr[arrdepth] = vardec->child->sibling->sibling->val.intvalue;
            arrdepth++;
            vardec = vardec->child;
        }
        // 此时的推导式为VarDec ::= ID，开始添加构造域
        struct FieldList* nfl = create_fieldlist();
        strcpy(nfl->name, vardec->child->val.strvalue);
        // arrdepth为0表示这仅是个标准类型
        if(arrdepth == 0) {
            nfl->type.kind = BASIC;
            if(strcmp(type_name, "int") == 0)
                nfl->type.u.basic = 1;
            else
                nfl->type.u.basic = 2;
        }
        // arrdepth不为0表示这是一个数组类型
        else {
            nfl->type.kind = ARRAY;
            nfl->type.u.array.size = arr[arrdepth - 1];
            struct Type* elem = &(nfl->type);
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
            if(strcmp(type_name, "int") == 0)
                elem->u.array.elem->u.basic = 1;
            else
                elem->u.array.elem->u.basic = 2;
        }

        if(*starfl == NULL) {
            *starfl = nfl;
        }
        else {
            struct FieldList* ffl = *starfl;
            while(ffl->next != NULL) ffl = ffl->next;
            ffl->next = nfl;
        }
        // DecList ::= Dec
        if(declist->child->sibling == NULL)
            break;
        // DecList ::= Dec COMMA DecList
        else
            declist = declist->child->sibling->sibling;
    }
}

void print_vartable() {
    struct VarTable* vt = vartablehead;
    printf("\033[;33mName\t\tKind\t\tContent\033[0m\n");
    while(vt != NULL) {
        printf("%s\t\t",vt->name);
        print_type(&vt->vartype);
        printf("\n");
        vt = vt->next;
    }
}

void print_type(struct Type* head) {
    if(head->kind == BASIC) {
        printf("BASIC\t\t");
        if(head->u.basic == 1)
            printf("int");
        else 
            printf("float");
    }
    else if(head->kind == ARRAY) {
        printf("ARRAY\t\t");
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
    else if(head->kind == STRUCTURE) {
        printf("STRUCT\t\t{");
        struct FieldList* fl = head->u.structure;
        for(; fl != NULL; fl = fl->next) {
            printf("%s: ", fl->name);
            if(fl->type.kind == BASIC) {
                if(fl->type.u.basic == 1)
                    printf("int");
                else 
                    printf("float");
            }
            else if(fl->type.kind = ARRAY) {
                struct Type* t = &(fl->type);
                while(t->kind == ARRAY) {
                    printf("[%d]", t->u.array.size);
                    t = t->u.array.elem;
                }
                if(t->u.basic == 1)
                    printf("int");
                else
                    printf("float");
            }
            printf("; ");
        }
        printf("}");
    }
    else
        assert(0);
}
void print_strutable() {
    printf("\033[;33mName\tField\033[0m\n");
    struct StructTable* st = strutablehead;
    for(; st != NULL; st = st->next) {
        printf("%s\t{", st->name);
        struct FieldList* fl = st->structure;
        for(; fl != NULL; fl = fl->next) {
            printf("%s: ", fl->name);
            if(fl->type.kind == BASIC) {
                if(fl->type.u.basic == 1)
                    printf("int");
                else 
                    printf("float");
            }
            else if(fl->type.kind = ARRAY) {
                struct Type* t = &(fl->type);
                while(t->kind == ARRAY) {
                    printf("[%d]", t->u.array.size);
                    t = t->u.array.elem;
                }
                if(t->u.basic == 1)
                    printf("int");
                else
                    printf("float");
            }
            printf("; ");
        }
        printf("}\n");
    }
}