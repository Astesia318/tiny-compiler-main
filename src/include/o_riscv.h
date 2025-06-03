// riscv code used
#ifndef RISCV_H
#define RISCV_H

#include <stdio.h>
#include <stdlib.h>

#include "e_tac.h"
#include "o_reg.h"

// add sub
#define R_TYPE(op_mnemonic, dest_reg_name, rs1_reg_name, rs2_reg_name)   \
	input_str(obj_file, "\t%s %s, %s, %s\n", op_mnemonic, dest_reg_name, \
	          rs1_reg_name, rs2_reg_name)
// addi,subi
#define I_TYPE_ARITH(op_mnemonic, dest_reg_name, rs1_reg_name, immediate) \
	input_str(obj_file, "\t%s %s, %s, %d\n", op_mnemonic, dest_reg_name,  \
	          rs1_reg_name, immediate)
// lw lbu
#define I_TYPE_LOAD(op_mnemonic, dest_reg_name, rs1_base_reg_name,       \
                    offset_immediate)                                    \
	input_str(obj_file, "\t%s %s, %d(%s)\n", op_mnemonic, dest_reg_name, \
	          offset_immediate, rs1_base_reg_name)
// sw sb
#define S_TYPE_STORE(op_mnemonic, rs2_src_reg_name, rs1_base_reg_name,      \
                     offset_immediate)                                      \
	input_str(obj_file, "\t%s %s, %d(%s)\n", op_mnemonic, rs2_src_reg_name, \
	          offset_immediate, rs1_base_reg_name)
// beq bne
#define B_TYPE_BRANCH(op_mnemonic, rs1_reg_name, rs2_reg_name, label_name) \
	input_str(obj_file, "\t%s %s, %s, %s\n", op_mnemonic, rs1_reg_name,    \
	          rs2_reg_name, label_name)
// li
#define U_TYPE_UPPER_IMM(op_mnemonic, dest_reg_name, immediate) \
	input_str(obj_file, "\t%s %s, %d\n", op_mnemonic, dest_reg_name, immediate)
// la lla
#define U_TYPE_UPPER_SYM(op_mnemonic, dest_reg_name, symbol_no)              \
	op_mnemonic == "lla" ? input_str(obj_file, "\t%s %s, .LC%d\n",           \
	                                 op_mnemonic, dest_reg_name, symbol_no)  \
	                     : input_str(obj_file, "\t%s %s, %s\n", op_mnemonic, \
	                                 dest_reg_name, symbol_no)

// jr
#define J_TYPE_JUMP_REG(op_mnemonic, dest_reg_name) \
	input_str(obj_file, "\t%s %s\n", op_mnemonic, dest_reg_name)
// call j
#define J_TYPE_JUMP_PSEUDO(op_mnemonic, label_name)                     \
	op_mnemonic == "call"                                               \
	    ? input_str(obj_file, "\t%s %s@plt\n", op_mnemonic, label_name) \
	    : input_str(obj_file, "\t%s %s\n", op_mnemonic, label_name)
// mv seqz
#define PSEUDO_2_REG(op_mnemonic, dest_reg_name, src_reg_name)       \
	input_str(obj_file, "\t%s %s, %s\n", op_mnemonic, dest_reg_name, \
	          src_reg_name)

/* register */
extern int tos;  // 栈顶偏移
extern int tof;  // 栈帧偏移
extern int oof;  // 参数偏移
extern int oon;  // 临时偏移

// 函数声明
void asm_bin(char *op, struct id *a, struct id *b, struct id *c);
void asm_cmp(int op, struct id *a, struct id *b, struct id *c);
void asm_assign(struct id *a, struct id *b);
void asm_cond(char *op, struct id *a, const char *l);
void asm_refer(struct id *pointer, struct id *var_pointed);
void asm_derefer_get(struct id *var, struct id *pointer);
void asm_derefer_put(struct id *var, struct id *pointer);
void asm_stack_pivot(struct tac *code);
void asm_stack_restore();
void asm_call(struct tac *code, struct id *a, struct id *b);
void asm_param(struct tac *code);
void asm_return(struct id *a);
void asm_label(struct id *a);
void asm_gvar(struct id *a);
void asm_output(struct id *a);
void asm_input(struct id *a);
void asm_load_var(struct id *a, const char *reg);   // sw reg &a
void asm_store_var(struct id *a, const char *reg);  // sw reg &a
#endif
