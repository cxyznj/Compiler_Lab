#include "intermediate.h"

struct InterCodes* codeshead = NULL;
int tempno = 1;

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
    printf("%s\n", Exp->child->name);
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
        assert(0);
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

// 语法树遍历
void search_tree(struct TreeNode* cur, struct TreeNode* father) {
    if(cur == NULL)
        return;
    if(father == NULL)
        assert(strcmp(cur->name, "Program") == 0);
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
    else if(strcmp(cur->name, "FunDec") == 0) {
        struct InterCodes* ics = create_intercodes();
        ics->code = create_intercode();
        ics->code->kind = MFUNCDEC;
        ics->code->u.funcname = malloc(sizeof(char) * 50);
        strcpy(ics->code->u.funcname, cur->child->val.strvalue);
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

// 将一段新的ics插入到codeshead的链表中
void add_codes(struct InterCodes* ics) {
    if(codeshead == NULL) {
        codeshead = ics;
    }
    else {
        struct InterCodes* fic = codeshead;
        while(fic->next != NULL) fic = fic->next;
        fic->next = ics;
    }
}

// 打印中间代码链表
void print_intercodes(struct InterCodes* head) {
    printf("\033[;33mIntermediate Codes:\033[0m\n");
    struct InterCodes* pic = head;
    while(pic != NULL) {
        struct InterCode* ic = pic->code;
        switch(ic->kind) {
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