#include "intermediate.h"

struct InterCodes* codeshead = NULL;
int tempno = 1;
int templabelno = 1;

// 模块的接口函数
void generate_intercodes(struct TreeNode* tn) {
    search_tree(tn, NULL);
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

// 翻译函数
struct InterCodes* translate_Exp(struct TreeNode* Exp, struct Operand* place) {
    struct InterCodes* ics = NULL;
    printf("%s%d\n", Exp->child->name, Exp->child->situation[0]);
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
        assert(0);
    }
    else if(strcmp(Exp->child->sibling->name, "DOT") == 0) {
        assert(0);
    }
    else if(strcmp(Exp->child->sibling->name, "LB") == 0) {
        assert(0);
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
        assert(strcmp(Exp->child->child->name, "ID") == 0);
        struct Operand* var = create_operand();
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
    else if(strcmp(Exp->child->sibling->name, "AND") == 0
            || strcmp(Exp->child->sibling->name, "OR") == 0
            || strcmp(Exp->child->sibling->name, "RELOP") == 0) {
        printf("relop(%d)", Exp->child->situation[0]);
        assert(0);
        //ics = translate_Cond(Exp);
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
    if(Args->child->sibling == NULL) {
        // Args ::= Exp
        struct Operand* t1 = new_temp();
        struct InterCodes* code1 = translate_Exp(Args->child, t1);
        args_list[*args_count] = t1;
        *args_count = *args_count + 1;
        return code1;
    }
    else {
        // Args ::= Exp COMMA Args
        struct Operand* t1 = new_temp();
        struct InterCodes* code1 = translate_Exp(Args->child, t1);
        args_list[*args_count] = t1;
        *args_count = *args_count + 1;
        struct InterCodes* code2 = translate_Args(Args->child->sibling->sibling, args_list, args_count);
        struct InterCodes* fcode = code1;
        while(fcode->next != NULL) fcode = fcode->next;
        fcode->next = code2;
        code2->pre = code1;
        return code1;
    }
}

struct InterCodes* translate_Param(struct TreeNode* VarList) {
    assert(strcmp(VarList->child->child->sibling->child->name, "ID") == 0);
    struct InterCodes* code1 = create_intercodes();
    code1->code = create_intercode();
    code1->code->kind = MPARAM;
    code1->code->u.paramname = malloc(sizeof(char) * 50);
    strcpy(code1->code->u.paramname, VarList->child->child->sibling->child->val.strvalue);
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
void search_tree(struct TreeNode* cur, struct TreeNode* father) {
    if(cur == NULL)
        return;
    if(father == NULL)
        assert(strcmp(cur->name, "Program") == 0);
    else if(strcmp(cur->name, "FunDec") == 0) {
        struct InterCodes* ics = create_intercodes();
        ics->code = create_intercode();
        ics->code->kind = MFUNCDEC;
        ics->code->u.funcname = malloc(sizeof(char) * 50);
        strcpy(ics->code->u.funcname, cur->child->val.strvalue);
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

        search_tree(cur->sibling->sibling->sibling->sibling, father);
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
        printf("i'm here\n");
        if(cur->sibling->sibling->sibling->sibling->sibling == NULL) {
            // Stmt ::= IF LP Exp RP Stmt
            char* label1 = new_label();
            char* label2 = new_label();
            struct InterCodes* code1 = translate_Cond(cur->sibling->sibling, label1, label2);
            printf("ok\n");
            struct InterCodes* icslabel1 = create_label(label1);
            struct InterCodes* icslabel2 = create_label(label2);
            add_codes(code1);
            add_codes(icslabel1);
            search_tree(cur->sibling->sibling->sibling->sibling, father);
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
            search_tree(cur->sibling->sibling->sibling->sibling, father);
            struct InterCodes* gotolabel3 = create_intercodes();
            gotolabel3->code = create_intercode();
            gotolabel3->code->kind = MGOTO;
            gotolabel3->code->u.gotolabel = label3;
            gotolabel3->next = icslabel2;
            icslabel2->pre = gotolabel3;
            add_codes(gotolabel3);
            search_tree(cur->sibling->sibling->sibling->sibling->sibling->sibling, father);
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
        search_tree(cur->sibling, father);
        return;
    }
    search_tree(cur->child, cur);
    search_tree(cur->sibling, father);
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

// 打印中间代码链表
void print_intercodes(struct InterCodes* head) {
    printf("\033[;33mIntermediate Codes:\033[0m\n");
    struct InterCodes* pic = head;
    while(pic != NULL) {
        struct InterCode* ic = pic->code;
        switch(ic->kind) {
            case MRELOP:     printf("IF ");
                            print_operand(ic->u.relopgoto.t1);
                            printf(" %s ", ic->u.relopgoto.relopsym);
                            print_operand(ic->u.relopgoto.t2);
                            printf(" GOTO %s", ic->u.relopgoto.label);
                            break;
            case MASSIGN:   print_operand(ic->u.pair.left);
                            printf(" := ");
                            print_operand(ic->u.pair.right); break;
            case MMINUS:    print_operand(ic->u.pair.left);
                            printf(" := - ");
                            print_operand(ic->u.pair.right); break;
            case MADD:  print_operand(ic->u.binop.result);
                        printf(" := ");
                        print_operand(ic->u.binop.op1);
                        printf(" + "); 
                        print_operand(ic->u.binop.op2); break;
            case MSUB:  print_operand(ic->u.binop.result);
                        printf(" := ");
                        print_operand(ic->u.binop.op1);
                        printf(" - "); 
                        print_operand(ic->u.binop.op2); break;
            case MMUL:  print_operand(ic->u.binop.result);
                        printf(" := ");
                        print_operand(ic->u.binop.op1);
                        printf(" * "); 
                        print_operand(ic->u.binop.op2); break;                        
            case MDIV:  print_operand(ic->u.binop.result);
                        printf(" := ");
                        print_operand(ic->u.binop.op1);
                        printf(" / "); 
                        print_operand(ic->u.binop.op2); break;                        
            case MRW:   if(ic->u.rwfunc.rwflag) printf("WRITE ");
                        else printf("READ ");
                        print_operand(ic->u.rwfunc.op1); break;

            case MFUNC: printf("CALL %s", ic->u.funcname); break;
            case MRTFUNC: print_operand(ic->u.rtfunc.result); printf(" := CALL %s", ic->u.rtfunc.funcname); break;
            case MARG: printf("ARG "); print_operand(ic->u.arg); break;
            case MFUNCDEC: printf("FUNCTION %s :", ic->u.funcname); break;
            case MRETURN: printf("RETURN "); print_operand(ic->u.rtval); break;
            case MPARAM: printf("PARAM %s", ic->u.paramname); break;
            case MGOTO: printf("GOTO %s", ic->u.gotolabel); break;
            case MLABEL: printf("LABEL %s :", ic->u.labelname); break;

            default: assert(0); break;
        }
        printf("\n");
        pic = pic->next;
    }
}
// 打印操作数
void print_operand(struct Operand* op) {
    switch(op->kind) {
        case VARIABLE: printf("%s", op->u.varname); break;
        case CONSTANT: printf("#%d", op->u.ivalue); break;
        case TEMP: printf("t%d", op->u.tno); break;
        default: assert(0); break;
    }
}