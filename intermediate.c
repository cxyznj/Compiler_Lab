#include "intermediate.h"

// 中间代码表头
struct InterCodes* codeshead = NULL;
// 临时变量编号
int tempno = 1;
// label编号
int templabelno = 1;
// 存储当前函数的参数，用于判断这个数组是否是一个参数
struct CharList* charlisthead = NULL;
// 存储当前函数的名字
char funcname[50];
// 文件指针
FILE* out;

// 模块的接口函数
void generate_intercodes(struct TreeNode* tn) {
    search_tree(tn, NULL, 1, 1);
    remedy_pre();
    // 优化开关，选择性打开
    //optimize_intercodes();
    print_intercodes(codeshead);
}

// 安全创建结构体函数
struct Operand* create_operand() {
    struct Operand* rt = malloc(sizeof(struct Operand));
    rt->kind = -1;
    rt->u.ivalue = 0;
    return rt;
}
struct InterCode* create_intercode() {
    struct InterCode* rt = malloc(sizeof(struct InterCode));
    rt->kind = -1;
}
struct InterCodes* create_intercodes() {
    struct InterCodes* rt = malloc(sizeof(struct InterCodes));
    rt->code = NULL;
    rt->pre = NULL;
    rt->next = NULL;
}

struct CharList* create_charlist() {
    struct CharList* rt = malloc(sizeof(struct CharList));
    rt->charname = malloc(sizeof(char) * 50);
    rt->next = NULL;
    return rt;
}

// 翻译函数
struct InterCodes* translate_Exp(struct TreeNode* Exp, struct Operand* place) {
    struct InterCodes* ics = NULL;
    //printf("%s%d\n", Exp->child->name, Exp->child->situation[0]);
    if(strcmp(Exp->child->name, "INT") == 0) {
        //printf("%d\n", Exp->child->val.intvalue);
        // 生成临时变量
        struct Operand* constant = create_operand();
        constant->kind = CONSTANT;
        constant->u.ivalue = Exp->child->val.intvalue;
        // 生成place ::= INT的中间代码
        struct InterCode* ic = create_intercode();
        ic->kind = MASSIGN;
        ic->u.pair.right = constant;
        ic->u.pair.left = place;
        // 插入到临时中间代码链表中
        ics = create_intercodes();
        ics->code = ic;
        ics->pre = ics->next = NULL;
    }
    else if(strcmp(Exp->child->name, "ID") == 0 && Exp->child->sibling == NULL) {
        // 生成临时变量
        struct  Operand* var = create_operand();
        var->kind = VARIABLE;
        var->u.varname = malloc(sizeof(char) * 50);
        strcpy(var->u.varname, Exp->child->val.strvalue);
        // 生成place ::= ID的中间代码
        struct InterCode* ic = create_intercode();
        ic->kind = MASSIGN;
        ic->u.pair.right = var;
        ic->u.pair.left = place;
        // 插入到临时中间代码链表中
        ics = create_intercodes();
        ics->code = ic;
        ics->pre = ics->next = NULL;
    }
    else if(strcmp(Exp->child->name, "FLOAT") == 0) {
        assert(0);
    }
    else if(strcmp(Exp->child->name, "NOT") == 0) {
        struct Operand* t1 = new_temp();
        struct InterCodes* code1 = translate_Exp(Exp->child->sibling, t1);

        struct Operand* constant0 = create_operand();
        constant0->kind = CONSTANT;
        constant0->u.ivalue = 0;
        struct Operand* constant1 = create_operand();
        constant1->kind = CONSTANT;
        constant1->u.ivalue = 1;

        char* label1 = new_label();
        struct InterCodes* labeltrue = create_intercodes();
        labeltrue->code = create_intercode();
        labeltrue->code->kind = MLABEL;
        labeltrue->code->u.labelname = label1;

        struct InterCodes* passign0 = create_intercodes();
        passign0->code = create_intercode();
        passign0->code->kind = MASSIGN;
        passign0->code->u.pair.left = place;
        passign0->code->u.pair.right = constant0;

        struct InterCodes* passign1 = create_intercodes();
        passign1->code = create_intercode();
        passign1->code->kind = MASSIGN;
        passign1->code->u.pair.left = place;
        passign1->code->u.pair.right = constant1;

        struct InterCodes* judge = create_intercodes();
        judge->code = create_intercode();
        judge->code->kind = MRELOP;
        judge->code->u.relopgoto.t1 = t1;
        judge->code->u.relopgoto.relopsym = malloc(sizeof(char) * 50);
        strcpy(judge->code->u.relopgoto.relopsym, "==");
        judge->code->u.relopgoto.t2 = constant0;
        judge->code->u.relopgoto.label = label1;

        // code1 + judge + passign0 + labeltrue + passign1
        ics = code1;
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = judge;
        judge->pre = fcode;
        judge->next = passign0;
        passign0->pre = judge;
        passign0->next = labeltrue;
        labeltrue->pre = passign0;
        labeltrue->next = passign1;
        passign1->pre = labeltrue;
    }
    else if(strcmp(Exp->child->sibling->name, "DOT") == 0) {
        printf("\033[;31mCannot translate: Code contains variables or parameters of structure type.\033[0m\n");
        assert(0);
    }
    else if(strcmp(Exp->child->sibling->name, "LB") == 0) {
        //assert(0);
        struct Operand* arraddr = new_temp();
        struct InterCodes* code1 = get_addr(Exp, arraddr);
        // place = *arraddr
        struct InterCodes* assignval = create_intercodes();
        assignval->code = create_intercode();
        assignval->code->kind = MGVALUE;
        assignval->code->u.getvalue.type = 0;
        assignval->code->u.getvalue.x = place;
        assignval->code->u.getvalue.y = arraddr;
        // 将code1, assignval加入ics
        ics = code1;
        struct InterCodes* fcode = ics;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = assignval;
        assignval->pre = fcode;
    }
    else if(strcmp(Exp->child->sibling->name, "LP") == 0) {
        if(Exp->child->sibling->sibling->sibling == NULL) {
            // Exp ::= ID LP RP
            struct InterCodes* code1 = create_intercodes();
            code1->code = create_intercode();
            if(strcmp(Exp->child->val.strvalue, "read") == 0) {
                code1->code->kind = MRW;
                code1->code->u.rwfunc.rwflag = 0;
                code1->code->u.rwfunc.op1 = place;
            }
            else {
                code1->code->kind = MRTFUNC;
                code1->code->u.rtfunc.funcname = malloc(sizeof(char) * 50);
                strcpy(code1->code->u.rtfunc.funcname, Exp->child->val.strvalue);
                code1->code->u.rtfunc.result = place;
            }
            ics = code1;
        }
        else {
            // Exp ::= ID LP Args RP
            struct InterCodes* codes;
            struct Operand* args_list[20];
            int* args_count = malloc(sizeof(int));
            *args_count = 0;
            struct InterCodes* code1 = create_intercodes();
            code1->code = create_intercode();
            strcpy(funcname, Exp->child->val.strvalue);
            code1 = translate_Args(Exp->child->sibling->sibling, args_list, args_count);
            if(strcmp(Exp->child->val.strvalue, "write") == 0) {
                assert(*args_count == 1);
                codes = code1;
                struct InterCodes* fcode = codes;
                while(fcode->next != NULL) fcode = fcode->next;
                fcode->next = create_intercodes();
                fcode->next->pre = fcode;
                fcode->next->code = create_intercode();
                fcode->next->code->kind = MRW;
                fcode->next->code->u.rwfunc.rwflag = 1;
                fcode->next->code->u.rwfunc.op1 = args_list[0];
            }
            else {
                struct InterCodes* code2 = NULL;
                for(int i = 0; i < *args_count; i++) {
                    struct InterCodes* ncode = create_intercodes();
                    ncode->code = create_intercode();
                    ncode->code->kind = MARG;
                    // TODO:考虑数组情况
                    ncode->code->u.arg = args_list[i];
                    if(code2 == NULL) code2 = ncode;
                    else {
                        // 插入到链表头部
                        code2->pre = ncode;
                        ncode->next = code2;
                        code2 = ncode;
                    }
                }
                struct InterCodes* fcode = code1;
                while(fcode->next != NULL) fcode = fcode->next;
                fcode->next = code2;
                code2->pre = fcode;
                while(fcode->next != NULL) fcode = fcode->next;
                fcode->next = create_intercodes();
                fcode->next->pre = fcode;
                fcode->next->code = create_intercode();
                fcode->next->code->kind = MRTFUNC;
                fcode->next->code->u.rtfunc.funcname = malloc(sizeof(char) * 50);
                strcpy(fcode->next->code->u.rtfunc.funcname, Exp->child->val.strvalue);
                fcode->next->code->u.rtfunc.result = place;
                codes = code1;
            }
            ics = codes;
        }
    }
    else if(strcmp(Exp->child->name, "MINUS") == 0) {
        struct Operand* t1 = new_temp();
        struct InterCodes* code1 = translate_Exp(Exp->child->sibling, t1);
        // 生成临时变量
        struct Operand* constant = create_operand();
        constant->kind = CONSTANT;
        constant->u.ivalue = 0;
        // 生成place :: 0 - t1的代码
        struct InterCode* code2 = create_intercode();
        code2->kind = MSUB;
        code2->u.binop.op1 = constant;
        code2->u.binop.op2 = t1;
        code2->u.binop.result = place;
        // 将code1和code2插入到临时中间代码链表中
        ics = code1;
        struct InterCodes* fcode1 = code1;
        while(fcode1->next != NULL) fcode1 = fcode1->next;
        fcode1->next = create_intercodes();
        fcode1->next->code = code2;
        fcode1->next->next = NULL;
        fcode1->next->pre = fcode1;
    }
    else if(strcmp(Exp->child->name, "LP") == 0) {
        struct Operand* t1 = new_temp();
        ics = translate_Exp(Exp->child->sibling, t1);
        struct InterCodes* code1 = create_intercodes();
        code1->code = create_intercode();
        code1->code->kind = MASSIGN;
        code1->code->u.pair.left = place;
        code1->code->u.pair.right = t1;
        struct InterCodes* fcode = ics;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code1;
        code1->pre = fcode;
    }
    else if(strcmp(Exp->child->sibling->name, "ASSIGNOP") == 0) {
        // 赋值号的左边只能是变量
        //assert(strcmp(Exp->child->child->name, "ID") == 0);
        struct Operand* var = create_operand();
        if(strcmp(Exp->child->child->name, "ID") == 0) {
            var->kind = VARIABLE;
            var->u.varname = malloc(sizeof(char) * 50);
            strcpy(var->u.varname, Exp->child->child->val.strvalue);
            struct Operand* t1 = new_temp();
            struct InterCodes* code1 = translate_Exp(Exp->child->sibling->sibling, t1);
            struct InterCodes* code2 = create_intercodes();
            code2->code = create_intercode();
            code2->code->kind = MASSIGN;
            code2->code->u.pair.left = var;
            code2->code->u.pair.right = t1;
            code2->next = create_intercodes();
            code2->next->code = create_intercode();
            code2->next->code->kind = MASSIGN;
            code2->next->code->u.pair.left = place;
            code2->next->code->u.pair.right = var;
            //print_intercodes(code2);

            struct InterCodes* fcode1 = code1;
            while(fcode1->next != NULL) fcode1 = fcode1->next;
            fcode1->next = code2;
            code2->pre = fcode1;
            ics = code1;
        }
        else {
            // 左边是数组类型的变量
            struct Operand* t0 = new_temp();
            struct InterCodes* code0 = get_addr(Exp->child, t0);
            struct Operand* t1 = new_temp();
            struct InterCodes* code1 = translate_Exp(Exp->child->sibling->sibling, t1);
            struct InterCodes* arrassign = create_intercodes();
            arrassign->code = create_intercode();
            arrassign->code->kind = MGVALUE;
            arrassign->code->u.getvalue.type = 1;
            arrassign->code->u.getvalue.x = t0;
            arrassign->code->u.getvalue.y = t1;
            struct InterCodes* code2 = create_intercodes();
            code2->code = create_intercode();
            code2->code->kind = MGVALUE;
            code2->code->u.getvalue.type = 0;
            code2->code->u.getvalue.x = place;
            code2->code->u.getvalue.y = t0;
            struct InterCodes* fcode = code0;
            while(fcode->next != NULL) fcode = fcode->next;
            fcode->next = code1;
            code1->pre = fcode;
            while(fcode->next != NULL) fcode = fcode->next;
            fcode->next = arrassign;
            arrassign->pre = fcode;
            arrassign->next = code2;
            code2->pre = arrassign;
            ics = code0;
        }
    }
    else if(strcmp(Exp->child->sibling->name, "AND") == 0
            || strcmp(Exp->child->sibling->name, "OR") == 0
            || strcmp(Exp->child->sibling->name, "RELOP") == 0) {
        char* label1 = new_label();
        char* label2 = new_label();

        struct InterCodes* labeltrue = create_intercodes();
        labeltrue->code = create_intercode();
        labeltrue->code->kind = MLABEL;
        labeltrue->code->u.labelname = label1;

        struct InterCodes* labelfalse = create_intercodes();
        labelfalse->code = create_intercode();
        labelfalse->code->kind = MLABEL;
        labelfalse->code->u.labelname = label2;

        struct Operand* constant0 = create_operand();
        constant0->kind = CONSTANT;
        constant0->u.ivalue = 0;

        struct Operand* constant1 = create_operand();
        constant1->kind = CONSTANT;
        constant1->u.ivalue = 1;

        struct InterCodes* code0 = create_intercodes();
        code0->code = create_intercode();
        code0->code->kind = MASSIGN;
        code0->code->u.pair.left = place;
        code0->code->u.pair.right = constant0;

        struct InterCodes* code1 = translate_Cond(Exp, label1, label2);

        struct InterCodes* code2 = create_intercodes();
        code2->code = create_intercode();
        code2->code->kind = MASSIGN;
        code2->code->u.pair.left = place;
        code2->code->u.pair.right = constant1;

        // 合并
        code0->next = code1;
        code1->pre = code0;
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = labeltrue;
        labeltrue->pre = fcode;
        labeltrue->next = code2;
        code2->pre = labeltrue;
        code2->next = labelfalse;
        labelfalse->pre = code2;

        ics = code0;
    }
    else {
        // PLUS/MINUS/STAR/DIV
        struct Operand* t1 = new_temp();
        struct Operand* t2 = new_temp();
        struct InterCodes* code1 = translate_Exp(Exp->child, t1);
        struct InterCodes* code2 = translate_Exp(Exp->child->sibling->sibling, t2);
        struct InterCodes* code3 = create_intercodes();
        code3->code = create_intercode();
        if(strcmp(Exp->child->sibling->name, "PLUS") == 0)
            code3->code->kind = MADD;
        else if(strcmp(Exp->child->sibling->name, "MINUS") == 0)
            code3->code->kind = MSUB;
        else if(strcmp(Exp->child->sibling->name, "STAR") == 0)
            code3->code->kind = MMUL;
        else if(strcmp(Exp->child->sibling->name, "DIV") == 0)
            code3->code->kind = MDIV;
        else
            assert(0);
        code3->code->u.binop.result = place;
        code3->code->u.binop.op1 = t1;
        code3->code->u.binop.op2 = t2;
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code2;
        code2->pre = fcode;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code3;
        code3->pre = fcode;
        ics = code1;
    }
    assert(ics != NULL);
    return ics;
}


struct InterCodes* translate_Args(struct TreeNode* Args, struct Operand** args_list, int* args_count) {
    int arr_flag = 0;
    struct TreeNode* find_id = Args->child;
    while(strcmp(find_id->name, "Exp") == 0) find_id = find_id->child;
    if(strcmp(find_id->name, "ID") == 0) {
        if(get_vartable(find_id->val.strvalue)->vartype.kind == ARRAY)
            arr_flag = 1;
    }
    if(strcmp(funcname, "write") == 0)
        arr_flag = 0;

    printf("This is %d\n", arr_flag);

    if(Args->child->sibling == NULL) {
        // Args ::= Exp
        struct Operand* t1 = new_temp();
        struct InterCodes* code1 = translate_Exp(Args->child, t1);
        if(arr_flag) {
            struct Operand* t2 = new_temp();
            struct InterCodes* getaddrics = create_intercodes();
            getaddrics->code = create_intercode();
            getaddrics->code->kind = MGADDRESS;
            getaddrics->code->u.getaddress.x = t2;
            getaddrics->code->u.getaddress.y = t1;
            struct InterCodes* fcode = code1;
            while(fcode->next != NULL) fcode = fcode->next;
            fcode->next = getaddrics;
            getaddrics->pre = fcode;

            args_list[*args_count] = t2;
            *args_count = *args_count + 1;
        }
        else {
            args_list[*args_count] = t1;
            *args_count = *args_count + 1;
        }
        return code1;
    }
    else {
        // Args ::= Exp COMMA Args
        struct Operand* t1 = new_temp();
        struct InterCodes* code1 = translate_Exp(Args->child, t1);
        if(arr_flag) {
            struct Operand* t2 = new_temp();
            struct InterCodes* getaddrics = create_intercodes();
            getaddrics->code = create_intercode();
            getaddrics->code->kind = MGADDRESS;
            getaddrics->code->u.getaddress.x = t2;
            getaddrics->code->u.getaddress.y = t1;
            struct InterCodes* fcode = code1;
            while(fcode->next != NULL) fcode = fcode->next;
            fcode->next = getaddrics;
            getaddrics->pre = fcode;

            args_list[*args_count] = t2;
            *args_count = *args_count + 1;            
        }
        else {
            args_list[*args_count] = t1;
            *args_count = *args_count + 1;
        }

        struct InterCodes* code2 = translate_Args(Args->child->sibling->sibling, args_list, args_count);
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code2;
        code2->pre = code1;
        return code1;
    }
}

struct InterCodes* translate_Param(struct TreeNode* VarList) {
    //assert(strcmp(VarList->child->child->sibling->child->name, "ID") == 0);
    struct InterCodes* code1 = create_intercodes();
    code1->code = create_intercode();
    code1->code->kind = MPARAM;
    code1->code->u.paramname = malloc(sizeof(char) * 50);
    // 考虑一般情况和数组情况的区别
    if(strcmp(VarList->child->child->sibling->child->name, "ID") == 0)
        strcpy(code1->code->u.paramname, VarList->child->child->sibling->child->val.strvalue);
    else {
        struct TreeNode* VarDec = VarList->child->child->sibling;
        while(strcmp(VarDec->name, "VarDec") == 0) VarDec = VarDec->child;
        strcpy(code1->code->u.paramname, VarDec->val.strvalue);
    }

    // 更新参数表
    struct CharList* ncl = create_charlist();
    strcpy(ncl->charname, code1->code->u.paramname);
    if(charlisthead == NULL) charlisthead = ncl;
    else {
        struct CharList* fcl = charlisthead;
        while(fcl->next != NULL) fcl = fcl->next;
        fcl->next = ncl;
    }

    if(VarList->child->sibling == NULL) {
        // VarList ::= ParamDec
        // do nothing
        ;
    }
    else {
        // VarList ::= ParamDec COMMA VarList
        struct InterCodes* code2;
        code2 = translate_Param(VarList->child->sibling->sibling);
        code1->next = code2;
        code2->pre = code1;
    }
    return code1;
}

struct InterCodes* translate_Cond(struct TreeNode* Exp, char* label_true, char* label_false) {
    //printf("%s, %s\n", Exp->name, Exp->child->name);
    //assert(Exp->child->sibling != NULL);
    if(Exp->child->sibling == NULL) {
        //assert(0);
        goto OTHERS;
    }
    if(strcmp(Exp->child->sibling->name, "RELOP") == 0) {
        struct Operand* t1 = new_temp();
        struct Operand* t2 = new_temp();
        struct InterCodes* code1 = translate_Exp(Exp->child, t1);
        struct InterCodes* code2 = translate_Exp(Exp->child->sibling->sibling, t2);
 
        char* op = malloc(sizeof(char) * 50);
        strcpy(op, Exp->child->sibling->val.strvalue);
        struct InterCodes* code3 = create_intercodes();
        code3->code = create_intercode();
        code3->code->kind = MRELOP;
        code3->code->u.relopgoto.t1 = t1;
        code3->code->u.relopgoto.t2 = t2;
        code3->code->u.relopgoto.relopsym = malloc(sizeof(char) * 50);
        strcpy(code3->code->u.relopgoto.relopsym, op);
        code3->code->u.relopgoto.label = label_true;
        // 合并code1, code2, code3
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code2;
        code2->pre = fcode;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code3;
        code3->pre = fcode;
        while(fcode->next != NULL) fcode = fcode->next;
        struct InterCodes* gotolabelfalse = create_intercodes();
        gotolabelfalse->code = create_intercode();
        gotolabelfalse->code->kind = MGOTO;
        gotolabelfalse->code->u.gotolabel = label_false;
        fcode->next = gotolabelfalse;
        gotolabelfalse->pre = fcode;
        
        return code1;
    }
    else if(strcmp(Exp->child->name, "NOT") == 0) {
        return translate_Cond(Exp->child->sibling, label_false, label_true);
    }
    else if(strcmp(Exp->child->sibling->name, "AND") == 0) {
        char* label1 = new_label();
        struct InterCodes* code1 = translate_Cond(Exp->child, label1, label_false);
        struct InterCodes* code2 = translate_Cond(Exp->child->sibling->sibling, label_true, label_false);
        struct InterCodes* icslabel1 = create_label(label1);
        // code1 + icslabel1 + code2
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = icslabel1;
        icslabel1->pre = fcode;
        icslabel1->next = code2;
        code2->pre = icslabel1;
        return code1;
    }
    else if(strcmp(Exp->child->sibling->name, "OR") == 0) {
        char* label1 = new_label();
        struct InterCodes* code1 = translate_Cond(Exp->child, label_true, label1);
        struct InterCodes* code2 = translate_Cond(Exp->child->sibling->sibling, label_true, label_false);
        struct InterCodes* icslabel1 = create_label(label1);
        // code1 + icslabel1 + code2
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = icslabel1;
        icslabel1->pre = fcode;
        icslabel1->next = code2;
        code2->pre = icslabel1;
        return code1;
    }
    else {
        OTHERS: ; 
        struct Operand* t1 = new_temp();
        struct InterCodes* code1 = translate_Exp(Exp, t1);
        struct InterCodes* code2 = create_intercodes();
        code2->code = create_intercode();
        code2->code->kind = MRELOP;
        code2->code->u.relopgoto.t1 = t1;
        code2->code->u.relopgoto.t2 = new_constant(0);
        code2->code->u.relopgoto.relopsym = malloc(sizeof(char) * 50);
        strcpy(code2->code->u.relopgoto.relopsym, "!=");
        code2->code->u.relopgoto.label = label_true;
        struct InterCodes* code3 = create_intercodes();
        code3->code = create_intercode();
        code3->code->kind = MGOTO;
        code3->code->u.gotolabel = label_false;
        
        // code1 + code2 + code3
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code2;
        code2->pre = fcode;
        code2->next = code3;
        code3->pre = code2;
        return code1;
    }
}

/*struct InterCodes* translate_Stmt(struct TreeNode* Stmt) {
    if(strcmp(Stmt->child->name, "Exp") == 0) {
        struct Operand* t1 = new_temp();
        return translate_Exp(Stmt->child, t1);
    }
    else if(strcmp(Stmt->child->name, "CompSt") == 0) {
        return translate_CompSt(Stmt->child);
    }
    else if(strcmp(Stmt->child->name, "RETURN") == 0) {
        struct Operand* t1 = new_temp();
        struct InterCodes* code1;
        code1 = translate_Exp(Stmt->child->sibling, t1);
        struct InterCodes* code2 = create_intercodes();
        code2->code = create_intercode();
        code2->code->kind = MRETURN;
        code2->code->u.rtval = t1;
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code2;
        code2->pre = fcode;
        return code1;
    }
    else {
        assert(0);
    }
}

struct InterCodes* translate_CompSt(struct TreeNode* CompSt) {
    struct InterCodes* codes = NULL;
    struct TreeNode* StmtList = CompSt->child->sibling->sibling;
    for( ; StmtList->ttype != EMPTY; StmtList = StmtList->child->sibling) {
        printf("This is Stmt %d\n", StmtList->child->situation[0]);
        struct InterCodes* code1 = translate_Stmt(StmtList->child);
        struct InterCodes* fcode = codes;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code1;
        code1->pre = codes;
    }
    assert(codes != NULL);
    return codes;
}*/

// 语法树遍历
void search_tree(struct TreeNode* cur, struct TreeNode* father, int child_flag, int sibling_flag) {
    if(cur == NULL)
        return;
    if(father == NULL)
        assert(strcmp(cur->name, "Program") == 0);
    else if(strcmp(cur->name, "VarDec") == 0) {
        // 作为参数的数组不需要DEC
        if(strcmp(father->name, "ParamDec") == 0) {
            search_tree(cur->sibling, father, 1, 1);
            return;
        }
        if(strcmp(cur->child->name, "VarDec") == 0) {
            // 遇到数组的定义
            while(strcmp(cur->name, "VarDec") == 0) cur = cur->child;
            struct VarTable* vt = get_vartable(cur->val.strvalue);
            int size = get_arrsize(0, &(vt->vartype)) * 4;
            struct InterCodes* ndec = create_intercodes();
            ndec->code = create_intercode();
            ndec->code->kind = MDEC;
            ndec->code->u.dec.decname = cur->val.strvalue;
            ndec->code->u.dec.decsize = size;
            add_codes(ndec);
            search_tree(cur->sibling, father, 1, 1);
            return;
        }
        if(cur->sibling != NULL) {
            // Dec ::= VarDec ASSIGNOP Exp
            assert(strcmp(cur->child->name, "ID") == 0);
            struct Operand* v1 = create_operand();
            v1->kind = VARIABLE;
            v1->u.varname = malloc(sizeof(char) * 50);
            strcpy(v1->u.varname, cur->child->val.strvalue);

            struct Operand* t1 = new_temp();
            struct InterCodes* code1 = translate_Exp(cur->sibling->sibling, t1);
            add_codes(code1);

            struct InterCodes* vassign = create_intercodes();
            vassign->code = create_intercode();
            vassign->code->kind = MASSIGN;
            vassign->code->u.pair.left = v1;
            vassign->code->u.pair.right = t1;
            add_codes(vassign);
            return;
        }
    }
    else if(strcmp(cur->name, "FunDec") == 0) {
        struct InterCodes* ics = create_intercodes();
        ics->code = create_intercode();
        ics->code->kind = MFUNCDEC;
        ics->code->u.funcname = malloc(sizeof(char) * 50);
        strcpy(ics->code->u.funcname, cur->child->val.strvalue);
        charlisthead = NULL;
        if(strcmp(cur->child->sibling->sibling->name, "VarList") == 0) {
            struct InterCodes* code2 = translate_Param(cur->child->sibling->sibling);
            ics->next = code2;
            code2->pre = ics;
        }
        /*assert(strcmp(cur->sibling->name, "CompSt") == 0);
        printf("xixi\n");
        struct InterCodes* code3 = translate_CompSt(cur->sibling);
        printf("haha\n");
        struct InterCodes* fcode = ics;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code3;
        code3->pre = fcode;*/
        add_codes(ics);
    }
    else if(strcmp(cur->name, "RETURN") == 0) {
        struct Operand* t1 = new_temp();
        struct InterCodes* code1;
        code1 = translate_Exp(cur->sibling, t1);
        struct InterCodes* code2 = create_intercodes();
        code2->code = create_intercode();
        code2->code->kind = MRETURN;
        code2->code->u.rtval = t1;
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code2;
        code2->pre = fcode;
        add_codes(code1);
        return;
    }
    else if(strcmp(cur->name, "WHILE") == 0) {
        char* label1 = new_label();
        char* label2 = new_label();
        char* label3 = new_label();
        struct InterCodes* code1 = translate_Cond(cur->sibling->sibling, label2, label3);
        struct InterCodes* icslabel1 = create_label(label1);
        struct InterCodes* icslabel2 = create_label(label2);
        struct InterCodes* icslabel3 = create_label(label3);

        icslabel1->next = code1;
        code1->pre = icslabel1;
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = icslabel2;
        icslabel2->pre = fcode;
        add_codes(icslabel1);

        search_tree(cur->sibling->sibling->sibling->sibling, father, 1, 0);
        struct InterCodes* gotolabel1 = create_intercodes();
        gotolabel1->code = create_intercode();
        gotolabel1->code->kind = MGOTO;
        gotolabel1->code->u.gotolabel = label1;
        gotolabel1->next = icslabel3;
        icslabel3->pre = gotolabel1;
        add_codes(gotolabel1);

        return;
    }
    else if(strcmp(cur->name, "IF") == 0) {
        if(cur->sibling->sibling->sibling->sibling->sibling == NULL) {
            // Stmt ::= IF LP Exp RP Stmt
            char* label1 = new_label();
            char* label2 = new_label();
            struct InterCodes* code1 = translate_Cond(cur->sibling->sibling, label1, label2);
            struct InterCodes* icslabel1 = create_label(label1);
            struct InterCodes* icslabel2 = create_label(label2);
            add_codes(code1);
            add_codes(icslabel1);
            search_tree(cur->sibling->sibling->sibling->sibling, father, 1, 0);
            add_codes(icslabel2);
            return;
        }
        else {
            // Stmt ::= IF LP Exp RP Stmt ELSE Stmt
            char* label1 = new_label();
            char* label2 = new_label();
            char* label3 = new_label();
            struct InterCodes* code1 = translate_Cond(cur->sibling->sibling, label1, label2);
            struct InterCodes* icslabel1 = create_label(label1);
            struct InterCodes* icslabel2 = create_label(label2);
            struct InterCodes* icslabel3 = create_label(label3);
            add_codes(code1);
            add_codes(icslabel1);
            search_tree(cur->sibling->sibling->sibling->sibling, father, 1, 0);
            struct InterCodes* gotolabel3 = create_intercodes();
            gotolabel3->code = create_intercode();
            gotolabel3->code->kind = MGOTO;
            gotolabel3->code->u.gotolabel = label3;
            gotolabel3->next = icslabel2;
            icslabel2->pre = gotolabel3;
            add_codes(gotolabel3);
            search_tree(cur->sibling->sibling->sibling->sibling->sibling->sibling, father, 1, 0);
            add_codes(icslabel3);
            return;
        }
    }
    else if(strcmp(cur->name, "Exp") == 0) {
        struct Operand* t1 = new_temp();
        struct InterCodes* ics;
        ics = translate_Exp(cur, t1);
        //print_intercodes(ics);
        // 将ics加到中间代码链表的尾部
        add_codes(ics);
        search_tree(cur->sibling, father, 1, 1);
        return;
    }
    if(child_flag)
        search_tree(cur->child, cur, 1, 1);
    if(sibling_flag)
        search_tree(cur->sibling, father, 1, 1);
}

// 生成一个新的临时变量
struct Operand* new_temp() {
    struct Operand* top = create_operand();
    top->kind = TEMP;
    top->u.tno = tempno;
    tempno++;
    return top;
}

char* new_label() {
    char* labelname = malloc(sizeof(char) * 50);
    sprintf(labelname, "label%d", templabelno);
    templabelno++;
    return labelname;
}

struct Operand* new_constant(int val) {
    struct Operand* nconstant = create_operand();
    nconstant->kind = CONSTANT;
    nconstant->u.ivalue = val;
    return nconstant;
}

// 将一段新的ics插入到codeshead的链表中
void add_codes(struct InterCodes* ics) {
    if(codeshead == NULL) {
        codeshead = ics;
    }
    else {
        struct InterCodes* fic = codeshead;
        while(fic->next != NULL) fic = fic->next;
        fic->next = ics;
        ics->pre = fic;
    }
}

// 生成一个LABEL的中间代码
struct InterCodes* create_label(char* labelname) {
    struct InterCodes* icslabel = create_intercodes();
    icslabel->code = create_intercode();
    icslabel->code->kind = MLABEL;
    icslabel->code->u.labelname = labelname;
    return icslabel;
}

// 获取一个数组变量的最终地址
struct InterCodes* get_addr(struct TreeNode* Exp, struct Operand* arraddr) {
        struct InterCodes* ics = NULL;

        struct Operand* t[20];
        int tcount = 0;
        for(int i = 0; i < 20; i++) t[i] = new_temp();
        //printf("test1\n");
        while(strcmp(Exp->child->name, "ID") != 0) {
            struct InterCodes* code1 = translate_Exp(Exp->child->sibling->sibling, t[tcount++]);
            if(ics == NULL)
                ics = code1;
            else {
                struct InterCodes* fcode = ics;
                while(fcode->next != NULL) fcode = fcode->next;
                fcode->next = code1;
                code1->pre = fcode;
            }
            Exp = Exp->child;
        }
        //printf("test2\n");
        //printf("%s\n", Exp->child->val.strvalue);
        struct VarTable* arr = get_vartable(Exp->child->val.strvalue);
        // 生成代码
        // arraddr = 首地址
        struct InterCodes* code2 = create_intercodes();
        code2->code = create_intercode();
        // 处理奇怪的传参数组问题
        if(source_of_array(Exp->child->val.strvalue) == 0) {
            code2->code->kind = MGADDRESS;
            code2->code->u.getaddress.x = arraddr;
            struct Operand* arrname = create_operand();
            arrname->kind = VARIABLE;
            arrname->u.varname = Exp->child->val.strvalue;
            code2->code->u.getaddress.y = arrname;
        }
        else {
            code2->code->kind = MGVALUE;
            code2->code->u.getvalue.type = 0;
            code2->code->u.getvalue.x = arraddr;
            struct Operand* arrname = create_operand();
            arrname->kind = VARIABLE;
            arrname->u.varname = Exp->child->val.strvalue;
            code2->code->u.getvalue.y = arrname;
        }
        //printf("test3\n");
        // arraddr = arraddr + 第i维的地址
        for(int i = 0; i < tcount; i++) {
            // t[i] = t[i] * (第i维的数组大小 * 4)
            //printf("test31\n");
            int size = get_arrsize(i+1, &(arr->vartype));
            //printf("test311\n");
            size = size * 4;
            struct Operand* opsize = create_operand();
            opsize->kind = CONSTANT;
            opsize->u.ivalue = size;
            //printf("test312\n");
            struct InterCodes* code3 = create_intercodes();
            code3->code = create_intercode();
            code3->code->kind = MMUL;
            //printf("test32\n");
            code3->code->u.binop.result = t[tcount - i - 1];
            code3->code->u.binop.op1 = t[tcount - i - 1];
            code3->code->u.binop.op2 = opsize;
            // arraddr = arraddr + t[i]
            //printf("test33\n");
            code3->next = create_intercodes();
            code3->next->pre = code3;
            code3->next->code = create_intercode();
            code3->next->code->kind = MADD;
            code3->next->code->u.binop.result = arraddr;
            code3->next->code->u.binop.op1 = arraddr;
            code3->next->code->u.binop.op2 = t[tcount - i - 1];
            //printf("test34\n");
            // 合并代码
            struct InterCodes* fcode = code2;
            while(fcode->next != NULL) fcode = fcode->next;
            fcode->next = code3;
            code3->pre = fcode;
        }
        //printf("test4\n");
        struct InterCodes* fcode = ics;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code2;
        code2->pre = fcode;
        return ics;
}

// 判断当前的数组是否是一个参数传来的数组
int source_of_array(char* arrname) {
    struct CharList* fcl = charlisthead;
    for(; fcl != NULL; fcl = fcl->next) {
        if(strcmp(fcl->charname, arrname) == 0)
            return 1;
    }
    return 0;
}

// 打印中间代码链表
void print_intercodes(struct InterCodes* head) {
    out = fopen("test.ir", "wt");
    //printf("\033[;33mIntermediate Codes:\033[0m\n");
    struct InterCodes* pic = head;
    while(pic != NULL) {
        struct InterCode* ic = pic->code;
        print_intercode(ic);
        fprintf(out, "\n");
        pic = pic->next;
    }
    fclose(out);
}

void print_intercode(struct InterCode* ic) {
    switch(ic->kind) {
            case MRELOP:    fprintf(out, "IF ");
                            print_operand(ic->u.relopgoto.t1);
                            fprintf(out, " %s ", ic->u.relopgoto.relopsym);
                            print_operand(ic->u.relopgoto.t2);
                            fprintf(out, " GOTO %s", ic->u.relopgoto.label);
                            break;
            case MASSIGN:   print_operand(ic->u.pair.left);
                            fprintf(out, " := ");
                            print_operand(ic->u.pair.right); break;
            case MADD:  print_operand(ic->u.binop.result);
                        fprintf(out, " := ");
                        print_operand(ic->u.binop.op1);
                        fprintf(out, " + "); 
                        print_operand(ic->u.binop.op2); break;
            case MSUB:  print_operand(ic->u.binop.result);
                        fprintf(out, " := ");
                        print_operand(ic->u.binop.op1);
                        fprintf(out, " - "); 
                        print_operand(ic->u.binop.op2); break;
            case MMUL:  print_operand(ic->u.binop.result);
                        fprintf(out, " := ");
                        print_operand(ic->u.binop.op1);
                        fprintf(out, " * "); 
                        print_operand(ic->u.binop.op2); break;                        
            case MDIV:  print_operand(ic->u.binop.result);
                        fprintf(out, " := ");
                        print_operand(ic->u.binop.op1);
                        fprintf(out, " / "); 
                        print_operand(ic->u.binop.op2); break;                        
            case MRW:   if(ic->u.rwfunc.rwflag) fprintf(out, "WRITE ");
                        else fprintf(out, "READ ");
                        print_operand(ic->u.rwfunc.op1); break;

            case MFUNC: fprintf(out, "CALL %s", ic->u.funcname); break;
            case MRTFUNC: print_operand(ic->u.rtfunc.result); fprintf(out, " := CALL %s", ic->u.rtfunc.funcname); break;
            case MARG: fprintf(out, "ARG "); print_operand(ic->u.arg); break;
            case MFUNCDEC: fprintf(out, "FUNCTION %s :", ic->u.funcname); break;
            case MRETURN: fprintf(out, "RETURN "); print_operand(ic->u.rtval); break;
            case MPARAM: fprintf(out, "PARAM %s", ic->u.paramname); break;
            case MGOTO: fprintf(out, "GOTO %s", ic->u.gotolabel); break;
            case MLABEL: fprintf(out, "LABEL %s :", ic->u.labelname); break;
            case MGADDRESS: print_operand(ic->u.getaddress.x); fprintf(out, " := &"); print_operand(ic->u.getaddress.y); break;
            case MGVALUE:   if(ic->u.getvalue.type == 0) { print_operand(ic->u.getvalue.x); fprintf(out, " := *"); print_operand(ic->u.getvalue.y); }
                            else { fprintf(out, "*"); print_operand(ic->u.getvalue.x); fprintf(out, " := "); print_operand(ic->u.getvalue.y); } break;
            case MDEC: fprintf(out, "DEC %s %d", ic->u.dec.decname, ic->u.dec.decsize); break;
            default: assert(0); break;
        }
}

// 打印操作数
void print_operand(struct Operand* op) {
    switch(op->kind) {
        case VARIABLE: fprintf(out, "%s", op->u.varname); break;
        case CONSTANT: fprintf(out, "#%d", op->u.ivalue); break;
        case TEMP: fprintf(out, "t%d", op->u.tno); break;
        default: assert(0); break;
    }
}

// 补全可能遗漏的pre指针
void remedy_pre() {
    struct InterCodes* pic = codeshead;
    for(; pic->next != NULL; pic = pic->next) {
        pic->next->pre = pic;
    }
}

// 比较两个Operand指针指向的内容是否相等
int operandcmp(struct Operand* t1, struct Operand* t2) {
    if(t1->kind == t2->kind) {
        if(t1->kind == CONSTANT || t1->kind == TEMP) {
            if(t1->u.ivalue == t2->u.ivalue)
                return 1;
        }
        else {
            if(strcmp(t1->u.varname, t2->u.varname) == 0)
                return 1;
        }
    }
    return 0;
}

// 优化中间代码
void optimize_intercodes() {
    struct InterCodes* optimize = codeshead;
    // 死代码消除
    for(; optimize != NULL; optimize = optimize->next) {
        // 如果该条指令对一个操作数进行了赋值，但后面没有用到这个值的话，可以去除
        //print_intercode(optimize->code);
        //printf(":\n");
        struct Operand* opleft;
        struct InterCode* opticode = optimize->code;
        switch(opticode->kind) {
            case MASSIGN: opleft = opticode->u.pair.left; break;
            case MADD: case MSUB: case MMUL: case MDIV: opleft = opticode->u.binop.result; break;
            case MGADDRESS: opleft = opticode->u.getaddress.x; break;
            case MGVALUE: opleft = opticode->u.getvalue.x; break;
            default: continue; break;
        }
        struct InterCodes* searchuse = optimize->next;
        // 在下一次赋值（或遍历结束）之前未被使用，则可认为是死代码
        int globaluseflag = 0;
        for(; searchuse != NULL; searchuse = searchuse->next) {
            int useflag = 0;
            int assignflag = 0;
            struct InterCode* scode = searchuse->code;
            switch(scode->kind) {
                case MASSIGN: assignflag = operandcmp(scode->u.pair.left, opleft); useflag = operandcmp(scode->u.pair.right, opleft); break;
                case MRELOP: useflag = (operandcmp(scode->u.relopgoto.t1, opleft) || operandcmp(scode->u.relopgoto.t2, opleft)); break;
                case MADD: case MSUB: case MMUL: case MDIV: assignflag = operandcmp(scode->u.binop.result, opleft); useflag = (operandcmp(scode->u.binop.op1, opleft) || operandcmp(scode->u.binop.op2, opleft)); break;
                case MRW: if(scode->u.rwfunc.rwflag) useflag = operandcmp(scode->u.rwfunc.op1, opleft); else assignflag = operandcmp(scode->u.rwfunc.op1, opleft); break;
                case MRTFUNC: assignflag = operandcmp(scode->u.rtfunc.result, opleft); break;
                case MARG: useflag = operandcmp(scode->u.arg, opleft); break;
                case MRETURN: useflag = operandcmp(scode->u.rtval, opleft); break;
                case MGADDRESS: assignflag = operandcmp(scode->u.getaddress.x, opleft); useflag = operandcmp(scode->u.getaddress.y, opleft); break;
                case MGVALUE: operandcmp(scode->u.getvalue.x, opleft); useflag = operandcmp(scode->u.getvalue.y, opleft); break;
                default: break;
            }
            //printf("\t%d %d ", assignflag, useflag);
            //print_intercode(scode);
            //printf("\n");
            if(useflag == 1)
                globaluseflag = 1;
            if(assignflag == 1)
                break;
        }
        if(globaluseflag == 0) {
            // TODO:死代码
            optimize->pre->next = optimize->next;
            optimize->next->pre = optimize->pre;
            free(optimize);
        }
    }
}