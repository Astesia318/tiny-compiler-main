#ifndef ASM_GENERATOR_H
#define ASM_GENERATOR_H
#include <stdio.h>
#include <stdlib.h>

#include "e_tac.h"
#include "o_reg.h"
/* register */
extern int tos;  // 栈顶偏移
extern int tof;  // 栈帧偏移
extern int oof;  // 参数偏移
extern int oon;  // 临时偏移

// 函数声明
void asm_bin(char *op, struct id *a, struct id *b, struct id *c);
void asm_cmp(int op, struct id *a, struct id *b, struct id *c);
void asm_cond(char *op, struct id *a, const char *l);
void asm_refer(struct id *pointer, struct id *var_pointed);
void asm_derefer(struct id *var, struct id *pointer);
void asm_stack_pivot(struct tac *code);
void asm_call(struct tac *code, struct id *a, struct id *b);
void asm_param(struct tac *code);
void asm_return(struct id *a);
void asm_label(struct id *a);
void asm_gvar(struct id *a);

#endif  // ASM_GENERATOR_H