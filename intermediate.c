#include "intermediate.h"

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
    if(strcmp(Exp->child->name, "INT") == 0) {
        // 生成临时变量
        struct Operand* constant = create_operand();
        constant->kind = CONSTANT;
        constant->u.ivalue = Exp->child->val.intvalue;
        // 生成place ::= constant的中间代码
        struct InterCode* ic = create_intercode();
        ic->kind = ASSIGN;
        ic->u.assign.right = constant;
        ic->u.assign.left = place;
        // 插入到临时中间代码链表中
        ics = create_intercodes();
        ics->code = ic;
        ics->pre = ics->next = NULL;
    }
    return ics;
}

// 语法树遍历
void search_tree(struct TreeNode* cur, struct TreeNode* father) {
    if(cur == NULL)
        return;
    if(father == NULL)
        assert(strcmp(cur->name, "Program") == 0);
    else if(strcmp(cur->name, "Exp") == 0) {
        struct Operand* t = new_temp();
        struct InterCodes* ics;
        ics = translate_Exp(cur, t);
        // 将ics加到中间代码链表的尾部
        add_codes(ics, codeshead);
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
}

// 将一段新的ics插入到codeshead的链表中
void add_codes(struct InterCodes* ics, struct InterCodes* head) {
    if(head == NULL)
        head = ics;
    else {
        struct InterCodes* fic = head;
        while(fic->next != NULL) fic = fic->next;
        fic->next = ics;
    }
}

// 打印中间代码链表
void print_intercodes(struct InterCodes* head) {
    struct InterCodes* pic = head;
    while(pic != NULL) {
        struct InterCode* ic = pic->code;
        switch(ic->kind) {
            case ASSIGN: print_operand(ic->u.assign.left); printf(" = "); print_operand(ic->u.assign.right); break;
            default: assert(0); break;
        }
    }
}
// 打印操作数
void print_operand(struct Operand* op) {
    switch(op->kind) {
        case VARIABLE: printf("%s", op->u.var->name); break;
        case CONSTANT: printf("%d", op->u.ivalue); break;
        case TEMP: printf("t%d", op->u.tno); break;
        default: assert(0); break;
    }
}