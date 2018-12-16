#include "symbol.h"

// 初始化符号表（给符号表的头项分配空间）
int init_table() {
    vartablehead = NULL;
    strutablehead = NULL;
    emptyname = 1;
    functablehead = NULL;

    // 预添加read和write两个函数
    struct FunctionType* func_read = create_functiontype(); struct FunctionType* func_write = create_functiontype();
    strcpy(func_read->name, "read"); strcpy(func_write->name, "write");
    func_read->paratable = NULL; func_write->paratable = NULL;
    struct Type* type = create_type();
    type->kind = BASIC;
    type->u.basic = 1;
    func_read->rt_value = func_write->rt_value = *type;
    functablehead = create_functiontable();
    functablehead->func = func_read;
    functablehead->state = 1;
    functablehead->next = create_functiontable();
    functablehead->next->func = func_write;
    functablehead->next->state = 1;

    return 1;
}

// 构建符号表
void build_vartable(struct TreeNode* tn) {
    find_vartable(tn, NULL, NULL, NULL, 0);
    // 打印变量表/结构体表及函数表
    //print_vartable();
    //print_strutable();
    //print_functable();
}

void check_program(struct TreeNode* tn) {
    check_error(tn, NULL);
    check_function();
}

// 检查是否有已声明但未定义的函数
void check_function() {
    struct FunctionTable* ft = functablehead;
    for(; ft != NULL; ft = ft->next) {
        if(ft->state == 0) {
            printf("\033[;31mError type 18 at Line %d: Undefined function \"%s\".\033[0m\n", ft->row, ft->func->name);
        }
    }
}

// 安全地新建一个结构体项
struct Type* create_type() {
    struct Type* rt = malloc(sizeof(struct Type));
    rt->kind = BASIC;
    rt->u.basic = 1;
    rt->constant = 0;
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
    rt->func = NULL;
    rt->state = 0;
    rt->row = 0;
    rt->next = NULL;
    return rt;
}

void find_vartable(struct TreeNode* cur, struct TreeNode* father, char* type, int* arr, int arrdepth){
    // ----空结点的跳过处理----
    if(cur == NULL)
        return;

    // ----错误检查----
    // 数组维度最高为10
    assert(arrdepth < 10);

    // ----遍历到特定结点时，执行操作----
    if(strcmp(cur->name, "Specifier") == 0) {
        // 基本类型
        if(strcmp(cur->child->name, "TYPE") == 0) {
            char* typename = malloc(sizeof(char) * 50);
            strcpy(typename, cur->child->val.strvalue);
            find_vartable(cur->sibling, father, typename, arr, arrdepth);
        }
        // 结构体的引用
        else if(strcmp(cur->child->child->sibling->name, "Tag") == 0) {
            char* typename = malloc(sizeof(char) * 50);
            strcpy(typename, cur->child->child->sibling->child->val.strvalue);
            find_vartable(cur->sibling, father, typename, arr, arrdepth);
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

            // 检查是否有重名结构体
            struct StructTable* fst = strutablehead;
            for(; fst != NULL; fst = fst->next) {
                if(strcmp(fst->name, struname) == 0) {
                    // 出现重名
                    printf("\033[;31mError type 16 at Line %d: Duplicated name \"%s\".\033[0m\n", cur->situation[0], struname);
                    find_vartable(cur->sibling, father, struname, arr, arrdepth);
                    return;
                }
            }

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
            // 检查结构体中是否有重名的域
            int cstflag = 0;
            struct FieldList* cst = nst->structure;
            for(; cst != NULL; cst = cst->next) {
                if(check_struconflict(cst->name, cst->next) == 1) {
                    printf("\033[;31mError type 15 at Line %d: Redefined field \"%s\".\033[0m\n", cur->situation[0], cst->name);
                    cstflag = 1;
                }
            }
            // 如果有重名，则这个结构体不放入结构体表
            if(cstflag == 1) {
                find_vartable(cur->sibling, father, struname, arr, arrdepth);
                return;
            }

            //printf("Add a new struct: %s\n", struname);
            if(strutablehead == NULL)
                strutablehead = nst;
            else {
                struct StructTable* st = strutablehead;
                while(st->next != NULL) st = st->next;
                st->next = nst;
            }

            find_vartable(cur->sibling, father, struname, arr, arrdepth);
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
                nvartable->vartype.u.structure = get_fieldlist(type, cur->situation[0]);
                /*struct FieldList* varfieldlist = get_fieldlist(type, cur->situation[0]);
                if(varfieldlist == NULL) {
                    // TODO:不将这个变量放入变量表
                    find_vartable(cur->sibling, father, NULL, NULL, 0);
                    return;
                }*/
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
    else if(strcmp(cur->name, "FunDec") == 0) {
        struct FunctionType* nfunctype = create_functiontype();
        // ----函数名----
        strcpy(nfunctype->name, cur->child->val.strvalue);
        // ----函数返回值----
        // 注：根据C--语法，返回值不能是指针
        if(strcmp(type, "int") == 0) {
            nfunctype->rt_value.kind = BASIC;
            nfunctype->rt_value.u.basic = 1;
        }
        else if(strcmp(type, "float") == 0) {
            nfunctype->rt_value.kind = BASIC;
            nfunctype->rt_value.u.basic = 2;
        }
        else {
            // struct类型
            nfunctype->rt_value.kind = STRUCTURE;
            nfunctype->rt_value.u.structure = get_fieldlist(type, cur->situation[0]);
        }
        // ----函数的参数表----
        // FunDec ::= ID LP VarList RP
        if(strcmp(cur->child->sibling->sibling->name, "VarList") == 0) {
            struct VarTable* paramtable = NULL;
            struct TreeNode* varlist = cur->child->sibling->sibling;
            while(1) {
                struct TreeNode* paramdec = varlist->child;
                //printf("This is paramdec: %s\n", paramdec->name);
                // 获取参数类型
                char* paramtype = malloc(sizeof(char) * 50);
                if(strcmp(paramdec->child->child->name, "TYPE") == 0)
                    strcpy(paramtype, paramdec->child->child->val.strvalue);
                else {
                    // 在参数表中只能出现StructSpecifier ::= STRUCT Tag
                    assert(strcmp(paramdec->child->child->child->sibling->name, "Tag") == 0);
                    strcpy(paramtype, paramdec->child->child->child->sibling->child->val.strvalue);
                }

                // 获取参数名
                // 为了将参数表中的数组加到变量表中，不得以出此下策
                //assert(strcmp(paramdec->child->sibling->child->name, "ID") == 0);
                if(strcmp(paramdec->child->sibling->child->name, "ID") != 0) {
                    strcpy(paramdec->name, "Def");
                    find_vartable(paramdec->child, paramdec, NULL, NULL, 0);
                    strcpy(paramdec->name, "ParamDec");
                }
                else {
                    char* paramname = malloc(sizeof(char) * 50);
                    strcpy(paramname, paramdec->child->sibling->child->val.strvalue);
                    //printf("The parameter is: %s(%s)\n", paramname, paramtype);

                    struct VarTable* npt = create_vartable();
                    strcpy(npt->name, paramname);
                    if(strcmp(paramtype, "int") == 0) {
                        npt->vartype.kind = BASIC;
                        npt->vartype.u.basic = 1;
                    }
                    else if (strcmp(paramtype, "float") == 0) {
                        npt->vartype.kind = BASIC;
                        npt->vartype.u.basic = 2;
                    }
                    else {
                        npt->vartype.kind = STRUCTURE;
                        npt->vartype.u.structure = get_fieldlist(paramtype, cur->situation[0]);
                    }
                    // 将新的参数添加到参数表中
                    if(paramtable == NULL)
                        paramtable = npt;
                    else {
                        struct VarTable* fpt = paramtable;
                        while(fpt->next != NULL) fpt = fpt->next;
                        fpt->next = npt;
                    }
                }

                //printf("Add a new param %s(%s)in function:%s\n", paramname, paramtype, nfunctype->name);

                // VarList ::= ParamDec
                if(varlist->child->sibling == NULL)
                    break;
                // VarList ::= ParamDec COMMA VarList
                else
                    varlist = paramdec->sibling->sibling;
            }
            nfunctype->paratable = paramtable;
        }
        // FunDec ::= ID LP RP
        else {
            nfunctype->paratable = NULL;
        }
        // ----将该函数加入函数表----
        // 函数的定义
        if(strcmp(cur->sibling->name, "CompSt") == 0) {
            // 重复定义检查
            struct FunctionTable* ft = functablehead;
            // 判断该函数是否应该加入到函数表中
            int add_flag = 1;
            for(; ft != NULL; ft = ft->next) {
                if(strcmp(ft->func->name, nfunctype->name) == 0) {
                    if(ft->state == 0) {
                        // 比较参数表是否相同，相同则将state改为1,否则报错
                        // 参数表相同
                        if(match_parameter(nfunctype->paratable, ft->func->paratable) == 1) {
                            ft->state = 1;
                            add_flag = 0;
                        }
                        // 参数表不同
                        else {
                            printf("\033[;31mError type 4 at Line %d: Redefined function \"%s\".\033[0m\n", cur->situation[0], nfunctype->name);
                            find_vartable(cur->sibling, father, NULL, NULL, 0);
                            return;
                        }
                    }
                    // 已定义过，报错
                    else {
                        printf("\033[;31mError type 4 at Line %d: Redefined function \"%s\".\033[0m\n", cur->situation[0], nfunctype->name);
                        find_vartable(cur->sibling, father, NULL, NULL, 0);
                        return;
                    }
                }
            }
            struct VarTable* fpt = nfunctype->paratable;
            for(; fpt != NULL; fpt = fpt->next) {
                // TODO:由于这里不考虑作用域，所以我们将新的参数添加到符号表中
                if(vartablehead == NULL) {
                    vartablehead = create_vartable();
                    strcpy(vartablehead->name, fpt->name);
                    vartablehead->vartype = fpt->vartype;
                    vartablehead->next = NULL;
                }
                else {
                    struct VarTable* fvt = vartablehead;
                    while(fvt->next != NULL) fvt = fvt->next;
                    fvt->next = create_vartable();
                    strcpy(fvt->next->name, fpt->name);
                    fvt->next->vartype = fpt->vartype;
                    fvt->next->next = NULL;
                }
            }

            if(add_flag == 1) {
                struct FunctionTable* nfunctable = create_functiontable();
                nfunctable->func = nfunctype;
                nfunctable->state = 1;
                nfunctable->row = cur->situation[0];
                if(functablehead == NULL) {
                    functablehead = nfunctable;
                }
                else {
                    struct FunctionTable* fft = functablehead;
                    while(fft->next != NULL) fft = fft->next;
                    fft->next = nfunctable;
                }
            }
            find_vartable(cur->sibling, father, NULL, NULL, 0);
        }
        // 函数的声明
        else {
            // 重复声明检查
            struct FunctionTable* ft = functablehead;
            for(; ft != NULL; ft = ft->next) {
                if(strcmp(ft->func->name, nfunctype->name) == 0) {
                    if(match_parameter(nfunctype->paratable, ft->func->paratable) != 1) {
                        printf("\033[;31mError type 19 at Line %d: Inconsistent declaration of function \"%s\".\033[0m\n", cur->situation[0], nfunctype->name);
                        find_vartable(cur->sibling, father, NULL, NULL, 0);
                        return;
                    }
                }
            }
            struct FunctionTable* nfunctable = create_functiontable();
            nfunctable->func = nfunctype;
            nfunctable->state = 0;
            nfunctable->row = cur->situation[0];
            if(functablehead == NULL) {
                functablehead = nfunctable;
            }
            else {
                struct FunctionTable* fft = functablehead;
                while(fft->next != NULL) fft = fft->next;
                fft->next = nfunctable;
            }
        }
    }
    // ----没有特殊情况，则做正常的深度遍历----
    else {
        find_vartable(cur->child, cur, type, arr, arrdepth);
        find_vartable(cur->sibling, father, type, arr, arrdepth);
    }
}

void check_error(struct TreeNode* cur, struct TreeNode* father) {
    // ----空结点的跳过处理----
    if(cur == NULL)
        return;

    // if(cur != NULL && father != NULL) {
    //    printf("cur = %s, father = %s\n", cur->name, father->name);
    //}

    // ----遍历到特定结点时，执行操作----
    if(father == NULL) {
        // do nothing
        assert(strcmp(cur->name, "Program") == 0);
    }
    // ----检查未定义就使用的变量----
    else if(strcmp(cur->name, "ID") == 0 && strcmp(father->name, "Exp") == 0 && cur->sibling == NULL && strcmp(father->child->name, "Exp") != 0) {
        struct Type* vartype = search_vartable(cur->val.strvalue);
        if(vartype == NULL) {
            printf("\033[;31mError type 1 at Line %d: Undefined variable \"%s\".\033[0m\n", cur->situation[0], cur->val.strvalue);
        }
    }
    // ----检查函数的引用发生的一系列的问题----
    else if(strcmp(cur->name, "ID") == 0 && strcmp(father->name, "Exp") == 0 && cur->sibling != NULL) {
        struct FunctionType* functype = search_functable(father->child->val.strvalue);
        if(functype == NULL) {
            struct Type* vtype = search_vartable(father->child->val.strvalue);
            if(vtype == NULL)
                printf("\033[;31mError type 2 at Line %d: Undefined function \"%s\".\033[0m\n", father->situation[0], father->child->val.strvalue);
            else
                printf("\033[;31mError type 11 at Line %d: \"%s\" is not a function.\033[0m\n", father->situation[0], father->child->val.strvalue);            
        }
        else {
            char* fname = father->child->val.strvalue;
            // 获取这个函数应有的参数表
            struct VarTable* vt = functype->paratable;
            struct Type* rtvalue = &(functype->rt_value);
            char* rttype = malloc(sizeof(char) * 50);
            if(rtvalue->kind == BASIC) {
                if(rtvalue->u.basic == 1)
                    strcpy(rttype, "int");
                else
                    strcpy(rttype, "float");
            }
            else if(rtvalue->kind == STRUCTURE) {
                strcpy(rttype, "structure");
            }
            else {
                // 返回值不能是数组类型
                assert(0);
            }

            if(strcmp(cur->sibling->sibling->name, "RP") == 0) {
                // Exp ::= ID LP RP
                if(vt != NULL) {
                    printf("\033[;31mError type 9 at Line %d: Function \"%s(%s)\" is not applicable for arguments \"()\".\033[0m\n", cur->situation[0], fname, rttype);
                }
            }
            else {
                // Exp ::= ID LP Args RP
                struct TreeNode* args = cur->sibling->sibling;
                struct VarTable* vthead = NULL;
                while(1) {
                    struct VarTable* nvt = create_vartable();
                    nvt->vartype = *(get_exp_type(args->child));

                    if(vthead == NULL)
                        vthead = nvt;
                    else {
                        struct VarTable* fvt = vthead;
                        while(fvt->next != NULL) fvt = fvt->next;
                        fvt->next = nvt;
                    }

                    if(args->child->sibling == NULL)
                        break;
                    else
                        args = args->child->sibling->sibling;
                }
                if(match_parameter(vt, vthead) == 0) {
                    printf("\033[;31mError type 9 at Line %d: Function \"%s(%s)\" is not applicable for arguments \"(", cur->situation[0], fname, rttype);
                    struct VarTable* fvt = vthead;
                    for(; fvt != NULL; fvt = fvt->next) {
                        if(fvt->vartype.kind == BASIC) {
                            if(fvt->vartype.u.basic == 1)
                                printf("int");
                            else
                                printf("float");
                        }
                        else if(fvt->vartype.kind == ARRAY)
                            printf("array");
                        else
                            printf("structure");
                        if(fvt->next != NULL)
                            printf(", ");
                    }
                    printf(")\".\033[0m\n");
                }
            }
        }
    }
    // ----检查赋值时类型不匹配的问题----
    else if(strcmp(cur->name, "ASSIGNOP") == 0 && strcmp(father->name, "Exp") == 0) {
        struct Type* type1 = get_exp_type(father->child);
        struct Type* type2 = get_exp_type(cur->sibling);
        if(type1 == NULL || type2 == NULL || match_variate(type1, type2) == 0) {
            printf("\033[;31mError type 5 at Line %d: Type mismatched for assignment.\033[0m\n", cur->situation[0]);
        }
        if(type1 != NULL && type1->constant == 1) {
            // constant为1表示这不是个变量，而是一个常量
            printf("\033[;31mError type 6 at Line %d: The left-hand side of an assignment must be a variable.\033[0m\n", cur->situation[0]);
        }
    }
    // ----检查两个变量进行操作时类型不匹配的问题----
    else if(strcmp(father->name, "Exp") == 0 \
            && (strcmp(cur->name, "AND") == 0 \
                || strcmp(cur->name, "OR") == 0 \
                || strcmp(cur->name, "RELOP") == 0 \
                || strcmp(cur->name, "PLUS") == 0 \
                || strcmp(cur->name, "MINUS") == 0 \
                || strcmp(cur->name, "STAR") == 0 \
                || strcmp(cur->name, "DIV") == 0 \
                )) {
        struct Type* type1 = get_exp_type(father->child);
        struct Type* type2 = get_exp_type(cur->sibling);
        if(type1 == NULL || type2 == NULL || match_variate(type1, type2) == 0) {
            printf("\033[;31mError type 7 at Line %d: Type mismatched for operands.\033[0m\n", cur->situation[0]);
        }
    }
    // ----检查函数返回值与return值不匹配的问题
    else if(strcmp(cur->name, "CompSt") == 0 && strcmp(father->name, "ExtDef") == 0) {
        // ExtDef ::= Specifier FunDec CompSt
        struct TreeNode* stmtlist = cur->child->sibling->sibling;
        struct FunctionType* functype = search_functable(father->child->sibling->child->val.strvalue);
        if(functype != NULL) {
            struct Type* rttype = &(functype->rt_value);
            find_return_in_stmtlist(stmtlist, rttype);
        }
    }
    // ----检查数组使用时发生的问题----
    else if(strcmp(cur->name, "LB") == 0 && strcmp(father->name, "Exp") == 0) {
        // Exp ::= Exp LB Exp RB
        if(get_exp_type(father->child)->kind != ARRAY) {
            printf("\033[;31mError type 10 at Line %d: \"", cur->situation[0]);
            print_exp(father->child);
            printf("\" is not an array.\033[0m\n");
        }
        struct Type* fval = get_exp_type(father->child->sibling->sibling);
        if(fval->kind == BASIC && fval->u.basic == 1) 
            // do nothing
            ;
        else {
            printf("\033[;31mError type 12 at Line %d: \"", cur->situation[0]);
            print_exp(father->child->sibling->sibling);
            printf("\" is not an integer.\033[0m\n");
        }
    }
    // ----检查结构体使用时发生的问题----
    else if(strcmp(cur->name, "DOT") == 0 && strcmp(father->name, "Exp") == 0) {
        // Exp ::= Exp DOT ID
        struct Type* strutype = get_exp_type(father->child);
        if(strutype == NULL || strutype->kind != STRUCTURE) {
            printf("\033[;31mError type 13 at Line %d: Illegal use of \".\".\033[0m\n", cur->situation[0]);
        }
    }
    check_error(cur->child, cur);
    check_error(cur->sibling, father);
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

// 检查结构体中是否有重名的域
int check_struconflict(char* varname, struct FieldList* fl) {
    struct FieldList* ffl = fl;
    while(ffl != NULL) {
        if(strcmp(ffl->name, varname) == 0)
            return 1;
        ffl = ffl->next;
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

struct FieldList* get_fieldlist(char* struname, int rownum) {
    struct StructTable* fst = strutablehead;
    // 在符号表中找到名为struname的struct项，然后将FieldList复制进来
    while(fst != NULL && strcmp(fst->name, struname) != 0) fst = fst->next;
    // 处理找不到结构体定义的情况
    //assert(fst != NULL);
    if(fst == NULL) {
        printf("\033[;31mError type 17 at Line %d: Undefined structure \"%s\".\033[0m\n", rownum, struname);
        return NULL;
    }
    return (fst->structure);
}

// 查找符号表，若找到返回对应的类型，否则返回NULL
struct Type* search_vartable(char* varname) {
    struct VarTable* vt = vartablehead;
    for(; vt != NULL; vt = vt->next) {
        if(strcmp(vt->name, varname) == 0)
            return &(vt->vartype);
    }
    return NULL;
}

// 查找函数表，若找到返回对应的函数类型信息，找不到返回NULL
struct FunctionType* search_functable(char* funcname) {
    struct FunctionTable* ft = functablehead;
    for(; ft != NULL; ft = ft->next) {
        if((strcmp(ft->func->name, funcname) == 0) && (ft->state == 1)) {
            return ft->func;
        }
    }
    return NULL;
}

// 返回Exp节点的属性Type
struct Type* get_exp_type(struct TreeNode* exp) {
    while(1) {
        if(strcmp(exp->child->name, "ID") == 0) {
            if(exp->child->sibling == NULL)
                // Exp ::= ID 变量引用
                return search_vartable(exp->child->val.strvalue);
            else {
                // Exp ::= ID LP Args RP | ID LP RP 函数引用
                struct FunctionType* ft = search_functable(exp->child->val.strvalue);
                if(ft == NULL)
                    return NULL;
                else
                    return &(ft->rt_value);
            }
        }
        else if(strcmp(exp->child->name, "INT") == 0){
            struct Type* tempint = create_type();
            tempint->kind = BASIC;
            tempint->u.basic = 1;
            tempint->constant = 1;
            return tempint;
        }
        else if(strcmp(exp->child->name, "FLOAT") == 0){
            struct Type* tempfloat = create_type();
            tempfloat->kind = BASIC;
            tempfloat->u.basic = 2;
            tempfloat->constant = 1;
            return tempfloat;
        }
        else if(strcmp(exp->child->name, "LP") == 0)
            exp = exp->child->sibling;
        else if(strcmp(exp->child->name, "MINUS") == 0)
            exp = exp->child->sibling;
        else if(strcmp(exp->child->name, "NOT") == 0)
            exp = exp->child->sibling;
        else if(strcmp(exp->child->name, "ID") == 0 && strcmp(exp->child->sibling->name, "LP") == 0) {
            struct FunctionType* ft = search_functable(exp->child->val.strvalue);
            if(ft == NULL)
                return NULL;
            else
                return &(ft->rt_value);
        }
        else if(strcmp(exp->child->name, "Exp") == 0 && strcmp(exp->child->sibling->name, "LB") == 0){
            // 处理数组情况
            struct Type* arrexp = get_exp_type(exp->child);
            assert(arrexp->kind == ARRAY);
            return arrexp->u.array.elem;
        }
        else if(strcmp(exp->child->name, "Exp") == 0 && strcmp(exp->child->sibling->name, "DOT") == 0) {
            // 处理结构体情况
            struct Type* strutype = get_exp_type(exp->child);
            if(strutype == NULL)
                return NULL;
            assert(strutype->kind == STRUCTURE);
            struct Type* rttype = NULL;
            char* fieldname = exp->child->sibling->sibling->val.strvalue;
            int searchflag = 0;
            struct FieldList* strufield = strutype->u.structure;
            for(; strufield != NULL; strufield = strufield->next) {
                if(strcmp(strufield->name, fieldname) == 0) {
                    searchflag = 1;
                    rttype = &(strufield->type);
                    break;
                }
            }
            if(searchflag == 0) {
                printf("\033[;31mError type 14 at Line %d: Non-existent field \"%s\".\033[0m\n", exp->child->situation[0], fieldname);
            }
            return rttype;
        }
        else if(strcmp(exp->child->name, "Exp") == 0)
            // 一系列乱七八糟的推导
            exp = exp->child;
    }
}

// 在一个stmtlist中查找return语句，判断return的返回值是否与rttype相等，否则报错
int find_return_in_stmtlist(struct TreeNode* stmtlist, struct Type* rttype) {
    int rtflag = 0;
    if(stmtlist->ttype == EMPTY) {
        printf("\033[;31mError type 8 at Line %d: Type mismatched for return.\033[0m\n", stmtlist->situation[0]);
        return 0;
    }
    for(; stmtlist->ttype != EMPTY; stmtlist = stmtlist->child->sibling) {
        struct TreeNode* stmt = stmtlist->child;
        // 查找所有stmt语句
        if(find_return_in_stmt(stmt, rttype) == 1)
            rtflag = 1;
        if(stmtlist->child->sibling->ttype == EMPTY){
            if(rtflag == 0)
                printf("\033[;31mError type 8 at Line %d: Type mismatched for return.\033[0m\n", stmt->situation[0]);
        }
    }
    return rtflag;
}

// 在一个stmt中查找return语句，判断return的返回值是否与rttype相等，否则报错
int find_return_in_stmt(struct TreeNode* stmt, struct Type* rttype) {
    if(strcmp(stmt->child->name, "CompSt") == 0) {
            return find_return_in_stmtlist(stmt->child->child->sibling->sibling, rttype);
    }
    else if(strcmp(stmt->child->name, "RETURN") == 0) {
        struct Type* curtype = get_exp_type(stmt->child->sibling);
        if(match_variate(rttype, curtype) == 0) {
            printf("\033[;31mError type 8 at Line %d: Type mismatched for return.\033[0m\n", stmt->situation[0]);
        }
        return 1;
    }
    else if(strcmp(stmt->child->name, "IF") == 0) {
        if(stmt->child->sibling->sibling->sibling->sibling->sibling == NULL)
            // Stmt ::= IF LP Exp RP Stmt
            return find_return_in_stmt(stmt->child->sibling->sibling->sibling->sibling, rttype);
        else {
            // Stmt ::= IF LP Exp RP Stmt ELSE Stmt
            if(find_return_in_stmt(stmt->child->sibling->sibling->sibling->sibling, rttype) == 1 ||
            find_return_in_stmt(stmt->child->sibling->sibling->sibling->sibling->sibling->sibling, rttype) == 1)
                return 1;
        }
    }
    else if(strcmp(stmt->child->name, "WHILE") == 0) {
        // Stmt ::= WHILE LP Exp RP Stmt
        return find_return_in_stmt(stmt->child->sibling->sibling->sibling->sibling, rttype);
    }
    return 0;
}

// ----模式匹配检查函数----
// 检查两个操作数类型是否匹配
int match_variate(struct Type* type1, struct Type* type2) {
    if(type1 == NULL || type2 == NULL) {
        return 0;
    }
    if(type1->kind == type2->kind) {
        if(type1->kind == BASIC) {
            if(type1->u.basic == type2->u.basic)
                return 1;
        }
        else if(type1->kind == ARRAY) {
            while(type1->kind != BASIC) {
                if(type1->u.array.size == type2->u.array.size) {
                    type1 = type1->u.array.elem;
                    type2 = type2->u.array.elem;
                }
                else
                    return 0;
            }
            return match_variate(type1, type2);
        }
        else if(type1->kind == STRUCTURE) {
            struct FieldList* fl1 = type1->u.structure;
            struct FieldList* fl2 = type2->u.structure;
            while(fl1 != NULL && fl2 != NULL) {
                if(match_variate(&fl1->type, &fl2->type) == 0) {
                    return 0;
                }
                fl1 = fl1->next; fl2 = fl2->next;
            }
            if(fl1 == NULL && fl2 != NULL || fl1 != NULL && fl2 == NULL)
                return 0;
            return 1;
        }
    }
    return 0;
}
// 检查两个参数表是否匹配
int match_parameter(struct VarTable* vt1, struct VarTable* vt2) {
    if(vt1== NULL || vt2 == NULL)
        return 0;
    while(vt1 != NULL && vt2 != NULL) {
        if(match_variate(&vt1->vartype, &vt2->vartype) == 0)
            return 0;
        vt1 = vt1->next;
        vt2 = vt2->next;
    }
    if((vt1 == NULL && vt2 != NULL) || (vt1 != NULL && vt2 == NULL))
        return 0;
    return 1;
}

void print_vartable() {
    struct VarTable* vt = vartablehead;
    printf("\033[;33mVariate Table:\nName\t\tKind\t\tContent\033[0m\n");
    while(vt != NULL) {
        printf("%s\t\t",vt->name);
        print_type(&vt->vartype);
        printf("\n");
        vt = vt->next;
    }
    printf("\n");
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
    printf("\033[;33mStructure Table:\nName\t\tField\033[0m\n");
    struct StructTable* st = strutablehead;
    for(; st != NULL; st = st->next) {
        printf("%s\t\t{", st->name);
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
    printf("\n");
}

void print_functable() {
    struct FunctionTable* ft = functablehead;
    printf("\033[;33mFunction Table:\nName\tState\tReturn\tParameters\033[0m\n");
    for(; ft != NULL; ft = ft->next) {
        printf("%s\t%d\t", ft->func->name, ft->state);
        if(ft->func->rt_value.kind == BASIC) {
            if(ft->func->rt_value.u.basic == 1)
                printf("int\t");
            else
                printf("float\t");
        }
        else {
            // TODO:为了方便，只打印STRUCT，下同
            printf("STRUCT\t");
        }
        struct VarTable* vt = ft->func->paratable;
        for(; vt != NULL; vt = vt->next) {
            printf("%s: ", vt->name);
            if(vt->vartype.kind == BASIC) {
                if(vt->vartype.u.basic == 1)
                    printf("int; ");
                else
                    printf("float; ");
            }
            else {
                printf("STRUCT; ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

// 打印一个exp
void print_exp(struct TreeNode* exp) {
    if(strcmp(exp->child->name, "ID") == 0) {
        printf("%s", exp->child->val.strvalue);
    }
    else if(strcmp(exp->child->name, "INT") == 0) {
        printf("%d", exp->child->val.intvalue);
    }
    else if(strcmp(exp->child->name, "FLOAT") == 0) {
        printf("%f", exp->child->val.floatvalue);
    }    
    else if(strcmp(exp->child->sibling->name, "ASSIGNOP") == 0) {
        print_exp(exp->child);
        printf(" = ");
        print_exp(exp->child->sibling->sibling);
    }
    else if(strcmp(exp->child->sibling->name, "AND") == 0) {
        print_exp(exp->child);
        printf(" && ");
        print_exp(exp->child->sibling->sibling);
    }
    else if(strcmp(exp->child->sibling->name, "OR") == 0) {
        print_exp(exp->child);
        printf(" || ");
        print_exp(exp->child->sibling->sibling);
    }
    else if(strcmp(exp->child->sibling->name, "RELOP") == 0) {
        print_exp(exp->child);
        printf(" RELOP ");
        print_exp(exp->child->sibling->sibling);
    }
    else if(strcmp(exp->child->sibling->name, "PLUS") == 0) {
        print_exp(exp->child);
        printf(" + ");
        print_exp(exp->child->sibling->sibling);
    }
    else if(strcmp(exp->child->sibling->name, "MINUS") == 0) {
        print_exp(exp->child);
        printf(" - ");
        print_exp(exp->child->sibling->sibling);
    }
    else if(strcmp(exp->child->sibling->name, "STAR") == 0) {
        print_exp(exp->child);
        printf(" * ");
        print_exp(exp->child->sibling->sibling);
    }
    else if(strcmp(exp->child->sibling->name, "DIV") == 0) {
        print_exp(exp->child);
        printf(" / ");
        print_exp(exp->child->sibling->sibling);
    }
    else if(strcmp(exp->child->name, "LP") == 0) {
        printf("(");
        print_exp(exp->child->sibling);
        printf(")");
    }
    else if(strcmp(exp->child->name, "MINUS") == 0) {
        printf("-");
        print_exp(exp->child->sibling);
    }
    else if(strcmp(exp->child->name, "NOT") == 0) {
        printf("!");
        print_exp(exp->child->sibling);
    }
    else if(strcmp(exp->child->sibling->name, "LB") == 0) {
        print_exp(exp->child);
        printf("[");
        print_exp(exp->child->sibling->sibling);
        printf("]");
    }
    else if(strcmp(exp->child->sibling->name, "DOT") == 0) {
        print_exp(exp->child);
        printf(".");
        printf("%s", exp->child->sibling->sibling->val.strvalue);
    }
}

// 为其他文件使用的：搜索变量及结构体
struct VarTable* get_vartable(char* name) {
    struct VarTable* fvt = vartablehead;
    for(; fvt != NULL; fvt = fvt->next) {
        if(strcmp(fvt->name, name) == 0) {
            return fvt;
        }
    }
    return NULL;
}
struct StructTable* get_structtable(char* name) {
    struct StructTable* fvt = strutablehead;
    for(; fvt != NULL; fvt = fvt->next) {
        if(strcmp(fvt->name, name) == 0) {
            return fvt;
        }
    }
    return NULL;
}

// 获取数组第i维的大小
int get_arrsize(int dim, struct Type* arrtype) {
    assert(arrtype != NULL);
    //printf("gtest1\n");
    //printf("%d\n", arrtype->kind);
    for(; dim > 0; dim--) {
        arrtype = arrtype->u.array.elem;
    }
    //printf("%d\n", arrtype->kind);
    //printf("gtest2\n");
    int result = 1;
    while(arrtype->kind == ARRAY) {
        //printf("loop1\n");
        result = result * arrtype->u.array.size;
        arrtype = arrtype->u.array.elem;
    }
    //printf("gtest3\n");
    return result;
}