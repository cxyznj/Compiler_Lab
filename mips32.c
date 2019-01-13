#include "mips32.h"

// 文件指针
FILE* out;

void generate_mips32code() {
    // 为变量分配栈空间
    int varoffset = generate_offset(vartablehead);
    // 检查是否正确为变量分配了栈空间
    printf("offset = %d\n", varoffset);
    print_varstack();
    // 直接将目标代码输出到文件
    out = fopen(filename, "w");
    // 输出首部信息
    fprintf(out, ".data\n_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n.globl main\n.text\n");
    fprintf(out, "read:\n\tli $v0, 4\n\tla $a0, _prompt\n\tsyscall\n\tli $v0, 5\n\tsyscall\n\tjr $ra\n\nwrite:\n\tli $v0, 1\n\tsyscall\n\tli $v0, 4\n\tla $a0, _ret\n\tsyscall\n\tmove $v0, $0\n\tjr $ra\n\n");
    fclose(out);

    // The number of parameters of this function
    int param_num = 0;
    int real_param_num = 0;
    // The number of arguments of this call
    int arg_num = 0;

    struct InterCodes* pic = codeshead;

    for(; pic != NULL; pic = pic->next) {
        struct InterCode* ic = pic->code;
        
        if(ic->kind == MFUNCDEC) {
            param_num = real_param_num = get_paramnum(ic->u.funcname);

            out = fopen(filename, "a");
            // print the name of function
            fprintf(out, "%s:\n", ic->u.funcname);

            //if(strcmp(ic->u.funcname, "main") == 0) {
                fprintf(out, "\taddi $sp, $sp, -4\n");
                // 记录栈开始位置
                fprintf(out, "\tmove $fp, $sp\n");            
                // 为变量（和临时变量）分配空间
                fprintf(out, "\taddi $sp, $sp, -%d\n", varoffset * 4);    
            //}

            // 保存s0-s7
            /*fprintf(out, "\taddi $sp, $sp, -32\n");
            fprintf(out, "\tsw $s0, 28($sp)\n");
            fprintf(out, "\tsw $s1, 24($sp)\n");
            fprintf(out, "\tsw $s2, 20($sp)\n");
            fprintf(out, "\tsw $s3, 16($sp)\n");
            fprintf(out, "\tsw $s4, 12($sp)\n");
            fprintf(out, "\tsw $s5, 8($sp)\n");
            fprintf(out, "\tsw $s6, 4($sp)\n");
            fprintf(out, "\tsw $s7, 0($sp)\n");*/
            fclose(out);
        }
        else if(ic->kind == MLABEL) {
            out = fopen(filename, "a");
            fprintf(out, "%s:\n", ic->u.labelname);
            fclose(out);
        }
        else if(ic->kind == MASSIGN) {
            if(ic->u.pair.right->kind == CONSTANT) {
                int loffset = get_var_offset(get_operand_name(ic->u.pair.left));
                //vartoreg(loffset * 4, "t0");

                out = fopen(filename, "a");
                fprintf(out, "\tli $t0, %d\n", ic->u.pair.right->u.ivalue);
                fclose(out);                
                regtovar(loffset * 4, "t0");
            }
            else {
                // kind == VARIABLE || TEMP
                int loffset = get_var_offset(get_operand_name(ic->u.pair.left));
                //vartoreg(loffset * 4, "t0");                
                int roffset = get_var_offset(get_operand_name(ic->u.pair.right));
                vartoreg(roffset * 4, "t1");                

                out = fopen(filename, "a");
                fprintf(out, "\tmove $t0, $t1\n");
                fclose(out);
                regtovar(loffset * 4, "t0");
            }
        }
        else if(ic->kind == MADD || ic->kind == MSUB) {
            //vartoreg(roffset * 4, "t0");            
            if(ic->u.binop.op1->kind == CONSTANT) {
                out = fopen(filename, "a");
                fprintf(out, "\tli $t1, %d\n", ic->u.binop.op1->u.ivalue);
                fclose(out);
                if(ic->u.binop.op2->kind == CONSTANT) {
                    out = fopen(filename, "a");
                    if(ic->kind == MADD)
                        fprintf(out, "\taddi $t0, $t1, %d\n", ic->u.binop.op2->u.ivalue);
                    else
                        fprintf(out, "\taddi $t0, $t1, %d\n", -ic->u.binop.op2->u.ivalue);
                    fclose(out);
                }
                else {
                    int op2offset = get_var_offset(get_operand_name(ic->u.binop.op2));                    
                    vartoreg(op2offset * 4, "t2");
                    out = fopen(filename, "a");
                    if(ic->kind == MADD)
                        fprintf(out, "\tadd $t0, $t1, $t2\n");
                    else
                        fprintf(out, "\tsub $t0, $t1, $t2\n");                    
                    fclose(out);
                }
            }
            else {
                // kind == VARIABLE || TEMP
                int op1offset = get_var_offset(get_operand_name(ic->u.binop.op1));
                vartoreg(op1offset * 4, "t1");
                if(ic->u.binop.op2->kind == CONSTANT) {
                    out = fopen(filename, "a");
                    if(ic->kind == MADD)
                        fprintf(out, "\taddi $t0, $t1, %d\n", ic->u.binop.op2->u.ivalue);
                    else
                        fprintf(out, "\taddi $t0, $t1, %d\n", -ic->u.binop.op2->u.ivalue);
                    fclose(out);
                }
                else {
                    int op2offset = get_var_offset(get_operand_name(ic->u.binop.op2));                    
                    vartoreg(op2offset * 4, "t2");
                    out = fopen(filename, "a");
                    if(ic->kind == MADD)
                        fprintf(out, "\tadd $t0, $t1, $t2\n");
                    else
                        fprintf(out, "\tsub $t0, $t1, $t2\n");                    
                    fclose(out);                    
                }
            }
            int roffset = get_var_offset(get_operand_name(ic->u.binop.result));
            regtovar(roffset * 4, "t0");
        }
        else if(ic->kind == MMUL || ic->kind == MDIV) {
            // 生成t0
            int roffset = get_var_offset(get_operand_name(ic->u.binop.result));
            //vartoreg(roffset * 4, "t0");
            // 生成t1
            if(ic->u.binop.op1->kind == CONSTANT) {
                out = fopen(filename, "a");
                fprintf(out, "\tli $t1, %d\n", ic->u.binop.op1->u.ivalue);
                fclose(out);
            }   
            else {
                int op1offset = get_var_offset(get_operand_name(ic->u.binop.op1));
                vartoreg(op1offset * 4, "t1");
            }   
            // 生成t2
            if(ic->u.binop.op2->kind == CONSTANT) {
                out = fopen(filename, "a");
                fprintf(out, "\tli $t2, %d\n", ic->u.binop.op2->u.ivalue);
                fclose(out);
            }   
            else {
                int op2offset = get_var_offset(get_operand_name(ic->u.binop.op2));
                vartoreg(op2offset * 4, "t2");
            }
            // 生成乘/除操作
            if(ic->kind == MMUL) {
                out = fopen(filename, "a");
                fprintf(out, "\tmul $t0, $t1, $t2\n");
                fclose(out);
            }
            else {
                out = fopen(filename, "a");
                fprintf(out, "\tdiv $t1, $t2\n\tmflo $t0\n");
                fclose(out);
            }
            regtovar(roffset * 4, "t0");            
        }
        else if(ic->kind == MGVALUE) {
            int xoffset = get_var_offset(get_operand_name(ic->u.getvalue.x));
            vartoreg(xoffset * 4, "t0");
            if(ic->u.getvalue.y->kind == CONSTANT) {
                out = fopen(filename, "a");
                fprintf(out, "\tli $t1, %d\n", ic->u.getvalue.y->u.ivalue);
                fclose(out);
            }
            else {
                int yoffset = get_var_offset(get_operand_name(ic->u.getvalue.y));
                vartoreg(yoffset * 4, "t1");            
            }

            if(ic->u.getvalue.type == 0) {
                out = fopen(filename, "a");
                fprintf(out, "\tlw $t0, 0($t1)\n");
                fclose(out);
                regtovar(xoffset * 4, "t0");
            }
            else {
                out = fopen(filename, "a");
                fprintf(out, "\tsw $t1, 0($t0)\n");
                fclose(out);
            }
        }
        else if(ic->kind == MGOTO) {
            out = fopen(filename, "a");
            fprintf(out, "\tj %s\n", ic->u.gotolabel);
            fclose(out);
        }
        else if(ic->kind == MRW || ic->kind == MFUNC || ic->kind == MRTFUNC) {
            // get function name
            char fname[50];
            if(ic->kind == MRW) {
                if(ic->u.rwfunc.rwflag) {
                    strcpy(fname, "write");

                    // put write arguments into $a1
                    if(ic->u.rwfunc.op1->kind == CONSTANT) {
                        out = fopen(filename, "a");
                        fprintf(out, "\tli $a0, %d\n", ic->u.rwfunc.op1->u.ivalue);
                        fclose(out);
                    }
                    else {
                        int toffset = get_var_offset(get_operand_name(ic->u.rwfunc.op1));
                        vartoreg(toffset * 4, "a0");
                    }
                }
                else
                    strcpy(fname, "read");
            }
            else if(ic->kind == MFUNC) {
                strcpy(fname, ic->u.funcname);
            }
            else {
                // ic->kind == MRTFUNC
                strcpy(fname, ic->u.rtfunc.funcname);
            }

            // call function
            out = fopen(filename, "a");
            // set the number of arguments
            fprintf(out, "\tli $a3, %d\n", arg_num);
            // save fp
            fprintf(out, "\taddi $sp, $sp, -4\n\tsw $fp, 0($sp)\n");
            fprintf(out, "\taddi $sp, $sp, -4\n\tsw $ra, 0($sp)\n\tjal %s\n\tlw $ra, 0($sp)\n\taddi $sp, $sp, 4\n", fname);
            fprintf(out, "\tlw $fp, 0($sp)\n\taddi $sp, $sp, 4\n");
            fclose(out);

            // get return value
            if(ic->kind == MRW) {
                if(!ic->u.rwfunc.rwflag) {
                    // read function
                    int op1offset = get_var_offset(get_operand_name(ic->u.rwfunc.op1));
                    //vartoreg(op1offset * 4, "t0");
                    out = fopen(filename, "a");
                    fprintf(out, "\tmove $t0, $v0\n");
                    fclose(out);
                    regtovar(op1offset * 4, "t0");
                }
            }
            else if(ic->kind == MRTFUNC) {
                int roffset = get_var_offset(get_operand_name(ic->u.rtfunc.result));
                //vartoreg(roffset * 4, "t0");
                out = fopen(filename, "a");
                fprintf(out, "\tmove $t0, $v0\n");
                fclose(out);
                regtovar(roffset * 4, "t0");
            }

            // remove the arguments which in the stack
            if(arg_num > 3) {
                out = fopen(filename, "a");
                fprintf(out, "\taddi $sp, $sp, %d\n", (arg_num-3)*4);
                fclose(out);
            }
            arg_num = 0;
        }
        else if(ic->kind == MRETURN) {
            // set return value
            if(ic->u.rtval->kind == CONSTANT) {
                out = fopen(filename, "a");
                fprintf(out, "\tli $t0, %d\n", ic->u.rtval->u.ivalue);
                fclose(out);
            }
            else {
                int roffset = get_var_offset(get_operand_name(ic->u.rtval));
                vartoreg(roffset * 4, "t0");
            }
            out = fopen(filename, "a");
            fprintf(out, "\tmove $v0, $t0\n");

            // recover stack
            /*fprintf(out, "\tlw $s0, 28($sp)\n");
            fprintf(out, "\tlw $s1, 24($sp)\n");
            fprintf(out, "\tlw $s2, 20($sp)\n");
            fprintf(out, "\tlw $s3, 16($sp)\n");
            fprintf(out, "\tlw $s4, 12($sp)\n");
            fprintf(out, "\tlw $s5, 8($sp)\n");
            fprintf(out, "\tlw $s6, 4($sp)\n");
            fprintf(out, "\tlw $s7, 0($sp)\n");
            fprintf(out, "\taddi $sp, $sp, 32\n");*/
            fprintf(out, "\taddi $sp, $sp, %d\n", (varoffset+1) * 4);    

            // return caller
            fprintf(out, "\tjr $ra\n");

            fclose(out);
        }
        else if(ic->kind == MRELOP) {
            // put x into reg t0
            if(ic->u.relopgoto.t1->kind == CONSTANT) {
                out = fopen(filename, "a");
                fprintf(out, "\tli $t0, %d\n", ic->u.relopgoto.t1->u.ivalue);
                fclose(out);                
            }
            else {
                int xoffset = get_var_offset(get_operand_name(ic->u.relopgoto.t1));
                vartoreg(xoffset * 4, "t0");
            }
            // put y into reg t1
            if(ic->u.relopgoto.t2->kind == CONSTANT) {
                out = fopen(filename, "a");
                fprintf(out, "\tli $t1, %d\n", ic->u.relopgoto.t2->u.ivalue);
                fclose(out);            
            }
            else {
                int yoffset = get_var_offset(get_operand_name(ic->u.relopgoto.t2));
                vartoreg(yoffset * 4, "t1");
            }
            // generate if relop goto
            out = fopen(filename, "a");
            if(strcmp(ic->u.relopgoto.relopsym, "==") == 0) {
                fprintf(out, "\tbeq ");
            }
            else if(strcmp(ic->u.relopgoto.relopsym, "!=") == 0) {
                fprintf(out, "\tbne ");
            }
            else if(strcmp(ic->u.relopgoto.relopsym, ">") == 0) {
                fprintf(out, "\tbgt ");
            }
            else if(strcmp(ic->u.relopgoto.relopsym, "<") == 0) {
                fprintf(out, "\tblt ");
            }
            else if(strcmp(ic->u.relopgoto.relopsym, ">=") == 0) {
                fprintf(out, "\tbge ");
            }
            else if(strcmp(ic->u.relopgoto.relopsym, "<=") == 0) {
                fprintf(out, "\tble ");
            }
            fprintf(out, "$t0, $t1, %s\n", ic->u.relopgoto.label);
            fclose(out);
        }
        else if(ic->kind == MARG) {
            // put argements into reg t0
            if(ic->u.arg->kind == CONSTANT) {
                out = fopen(filename, "a");
                fprintf(out, "\tli t0, %d\n", ic->u.arg->u.ivalue);
                fclose(out);
            }
            else {
                int aoffset = get_var_offset(get_operand_name(ic->u.arg));
                vartoreg(aoffset * 4, "t0");
            }

            if(arg_num < 3) {
                out = fopen(filename, "a");
                fprintf(out, "\tmove $a%d, $t0\n", arg_num);
                fclose(out);
            }
            else {
                // put arguments into stack
                out = fopen(filename, "a");
                fprintf(out, "\taddi $sp, $sp, -4\n\tsw $t0, 0($sp)\n");
                fclose(out);
            }
            arg_num++;
        }
        else if(ic->kind == MPARAM) {
            // put arguments into reg t0
            if(param_num < 4) {
                out = fopen(filename, "a");
                fprintf(out, "\tmove $t0, $a%d\n", param_num - 1);
                fclose(out);
            }
            else {
                out = fopen(filename, "a");
                fprintf(out, "\tlw $t0, %d($fp)\n", (real_param_num - param_num)*4 + 12);
                fclose(out);
            }

            int poffset = get_var_offset(ic->u.paramname);
            regtovar(poffset * 4, "t0");

            param_num--;
        }
        else if(ic->kind == MGADDRESS) {
            // 获取数组的首地址（以偏移量表示）
            int yoffset = get_var_offset(get_operand_name(ic->u.getaddress.y));
            out = fopen(filename, "a");
            // 将数组的首地址放在t0中            
            fprintf(out, "\taddi $t0, $fp, -%d\n", yoffset * 4);
            // 将t0中存放的地址放入x中
            fclose(out);
            int xoffset = get_var_offset(get_operand_name(ic->u.getaddress.x));
            regtovar(xoffset * 4, "t0");
        }
    }
    print_intercodes(codeshead);
}

int generate_offset(struct VarTable* vh) {
    varstackhead = NULL;
    int rtvalue = 0;

    struct VarTable* fvh = vh;
    for(; fvh != NULL; fvh = fvh->next) {
        if(fvh->vartype.kind == BASIC) {
            if(fvh->vartype.u.basic != 1) {
                printf("You can not use FLOAT type!\n");
                assert(0);
            }
            struct VarStack* nvs = create_varstack();
            strcpy(nvs->varname, fvh->name);
            nvs->offset = rtvalue;
            if(varstackhead == NULL)
                varstackhead = nvs;
            else {
                struct VarStack* fvs = varstackhead;
                while(fvs->next != NULL) fvs = fvs->next;
                fvs->next = nvs;
            }
            rtvalue++;
        }
        else if(fvh->vartype.kind == ARRAY) {
            // 默认为int类型的一维数组
            int arr_len = fvh->vartype.u.array.size;

            struct VarStack* nvs = create_varstack();
            strcpy(nvs->varname, fvh->name);
            nvs->offset = rtvalue;
            if(varstackhead == NULL)
                varstackhead = nvs;
            else {
                struct VarStack* fvs = varstackhead;
                while(fvs->next != NULL) fvs = fvs->next;
                fvs->next = nvs;
            }
            rtvalue += arr_len;
        }
        else {
            printf("You can not use STRUCTURE type!\n");
            assert(0);
        }
    }
    int tempno = get_tempno();
    for(int i = 1; i < tempno; i++) {
        struct VarStack* nvs = create_varstack();
        sprintf(nvs->varname, "t%d", i);
        nvs->offset = rtvalue;
        if(varstackhead == NULL)
            varstackhead = nvs;
        else {
            struct VarStack* fvs = varstackhead;
            while(fvs->next != NULL) fvs = fvs->next;
            fvs->next = nvs;
        }
        rtvalue++;
    }
    rtvalue--;
    return rtvalue;
}

// 获取某变量在栈中的偏移量
int get_var_offset(char* varname) {
    struct VarStack* fvs = varstackhead;
    for(; fvs != NULL; fvs = fvs->next) {
        if(strcmp(fvs->varname, varname) == 0) {
            return fvs->offset;
        }
    }
    // 没有查找到以varname为名字的变量
    printf("Can not find varname = %s\n", varname);
    assert(0);
    return 0;
}

// 获取一个操作数的变量名（变量或临时变量）
char* get_operand_name(struct Operand* op) {
    if(op->kind == VARIABLE) {
        return op->u.varname;
    }
    else if(op->kind == TEMP) {
        char* retvalue = malloc(sizeof(char) * 50);        
        sprintf(retvalue, "t%d", op->u.ivalue);
        return retvalue;
    }
    else {
        assert(0);
    }
}

// 将某变量的值从栈中放入寄存器
void vartoreg(int offset, char* regname) {
    out = fopen(filename, "a");
    fprintf(out, "\tlw $%s, -%d($fp)\n", regname, offset);
    fclose(out);
}
// 将某变量的值从寄存器放入栈中
void regtovar(int offset, char* regname) {
    out = fopen(filename, "a");
    fprintf(out, "\tsw $%s, -%d($fp)\n", regname, offset);
    fclose(out);
}

// 输出函数
void print_varstack() {
    struct VarStack* fvs = varstackhead;
    for(; fvs != NULL; fvs = fvs->next) {
        printf("%s: offset = %d\n", fvs->varname, fvs->offset);
    }
}

// 安全创建结构体的函数
struct VarStack* create_varstack() {
    struct VarStack* rtvalue = malloc(sizeof(struct VarStack));
    strcpy(rtvalue->varname, "EMPTY");
    rtvalue->offset = 0;
    rtvalue->next = NULL;
    return rtvalue;
}