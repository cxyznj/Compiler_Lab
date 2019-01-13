#ifndef _MIPS32_H

#define _MIPS32_H

#include "symbol.h"
#include "datastruct.h"
#include <stdio.h>
#include "intermediate.h"

struct VarStack {
    char varname[50];
    // 偏移量为整数，在栈中实际上为该整数的倍数
    int offset;
    struct VarStack *next;
};

// 接口函数
void generate_mips32code();

// 计算各变量（和临时变量）存放在栈中的偏移量，返回存放变量的总偏移量
int generate_offset(struct VarTable* vh);

// 获取某变量在栈中的偏移量
int get_var_offset(char* varname);

// 获取一个操作数的变量名（变量或临时变量）
char* get_operand_name(struct Operand* op);

// 将某变量的值从栈中放入寄存器
void vartoreg(int offset, char* regname);
// 将某变量的值从寄存器放入栈中
void regtovar(int offset, char* regname);

// 安全创建结构体的函数
struct VarStack* create_varstack();

// 输出函数
void print_varstack();

// 全局变量
struct VarStack* varstackhead;

// 外部引用变量
struct InterCodes* codeshead;
char* filename;

#endif