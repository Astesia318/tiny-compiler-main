// hjj: tbd, float num
#include "o_asm.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "riscv.h"
#include "e_tac.h"

// 生成二元运算对应的汇编代码
//XXX:默认寄存器a5用来接收结果
void asm_bin(char *op, struct id *a, struct id *b, struct id *c) {
	//bc都是立即数,直接计算
	if(b->id_type==ID_NUM&&c->id_type==ID_NUM){
		U_TYPE_UPPER_IMM("li", reg_name[R_a5], OP_TO_CAL(op, b->num.num_int, c->num.num_int)); // li rd, imm
	}
	//bc其中一个不是立即数
	else {
		int reg_b = -1, reg_c = -1;
		switch(op[0]) {
			case 'a'://add
				while (reg_b == reg_c) {//bc有一个为立即数均可使用addi处理
					if(b->id_type!=ID_NUM)reg_b = reg_find(b);
					if(c->id_type!=ID_NUM)reg_c = reg_find(c);
					if(b==c)break;
				}
			
				break;
			case 's'://sub
				while (reg_b == reg_c) {//c为立即数时可以使用addi处理
					reg_b = reg_find(b);
					if(c->id_type!=ID_NUM)reg_c = reg_find(c);
					if(b==c)break;
				}
				break;
			case 'm'://mul，XXX:选择将立即数储存到寄存器计算，但是gcc对于某些特殊用例是通过移位计算的
			case 'd'://div,XXX:选择将立即数储存到寄存器计算，但是gcc对于某些特殊用例是通过移位计算的
				while (reg_b == reg_c) {
					reg_b = reg_find(b);
					reg_c = reg_find(c);
					if(b==c)break;
				}
				break;
		}
		//op
		if(reg_b!=-1&&reg_c!=-1)input_str(obj_file, 
											"	%s a5,%s,%s\n", 
											op,
											reg_name[reg_b],
											reg_name[reg_c]);
		else {
			input_str(obj_file, 
						"	addi a5,%s,%d\n", 
						reg_b!=-1?
							reg_name[reg_b]:
							reg_name[reg_c],
						reg_b!=-1?
							(op[0]=='s'?
								-(c->num.num_int):
								c->num.num_int):
							b->num.num_int);
		}
	}
	//store a
	asm_store_var(a,"a5");
	rdesc_fill(R_a5,a,MODIFIED);
}

// 生成比较运算对应的汇编代码
//XXX:默认寄存器a5用来接收结果，写的太ex了，得想办法简洁些
void asm_cmp(int op, struct id *a, struct id *b, struct id *c) {
	//bc都是立即数,直接计算
	int res_reg = R_a5;
	if (b->id_type == ID_NUM && c->id_type == ID_NUM)
	{
		OP_TO_CMP(op, b->num.num_int, c->num.num_int) ? 
			U_TYPE_UPPER_IMM("li", reg_name[res_reg], 1) : //li rd,1
			(res_reg = R_zero);
	}
	//bc其中一个不是立即数
	else {
		int reg_b = -1, reg_c = -1;
			
		while (reg_b == reg_c) {
			if(b->id_type!=ID_NUM)reg_b = reg_find(b);
			if(c->id_type!=ID_NUM)reg_c = reg_find(c);
		}
		const char *b_n = reg_name[reg_b];
		const char *c_n = reg_name[reg_c];
		const char *res_n = reg_name[res_reg];

		if (reg_b != -1 && reg_c != -1) {
			switch (op) {
				case TAC_EQ: // a5 == a4  => sub a5, a5, a4; seqz a5, a5
					R_TYPE("sub", res_n, b_n, c_n);
					PSEUDO_2_REG("seqz", res_n, res_n);
					break;
				case TAC_NE: // a5 != a4  => sub a5, a5, a4; snez a5, a5
					R_TYPE("sub", res_n, b_n, c_n);
					PSEUDO_2_REG("snez", res_n, res_n);
					break;
				case TAC_LT: // a5 < a4   => slt a5, a5, a4
					R_TYPE("slt", res_n, b_n, c_n);
					break;
				case TAC_LE: // a5 <= a4  => slt a5, a4, a5; xori a5, a5, 1
					R_TYPE("slt", res_n, c_n, b_n); // res = (c < b)
					I_TYPE_ARITH("xori", res_n, res_n, 1); // res = !(c < b) => b <= c
					break;
				case TAC_GT: // a5 > a4   => slt a5, a4, a5
					R_TYPE("slt", res_n, c_n, b_n);
					break;
				case TAC_GE: // a5 >= a4  => slt a5, a5, a4; xori a5, a5, 1
					R_TYPE("slt", res_n, b_n, c_n); // res = (b < c)
					I_TYPE_ARITH("xori", res_n, res_n, 1); // res = !(b < c) => b >= c
					break;
			}
		} else {
			int imm_c = reg_b == -1 ? b->num.num_int : c->num.num_int;
			int reg_temp = reg_b == -1 ? reg_c : reg_b;
			const char *temp_n = reg_name[reg_temp];

			if (reg_b == -1) { // b 是立即数，op 需要反转
				if (op >= TAC_LE && op <= TAC_GE) {
					switch (op) {
						case TAC_LE: op = TAC_GE; break;
						case TAC_LT: op = TAC_GT; break;
						case TAC_GE: op = TAC_LE; break;
						case TAC_GT: op = TAC_LT; break;
					}
				}
			}

			switch (op) {
				case TAC_EQ:
					I_TYPE_ARITH("addi", res_n, temp_n, -imm_c);
					PSEUDO_2_REG("seqz", res_n, res_n);
					break;
				case TAC_NE: // a5 != imm_c => addi temp, a5, -imm_c; snez a5, temp
					I_TYPE_ARITH("addi", res_n, temp_n, -imm_c);
					PSEUDO_2_REG("snez", res_n, res_n);
					break;
				case TAC_LT: // a5 < imm_c  => slti a5, a5, imm_c
					I_TYPE_ARITH("slti", res_n, temp_n, imm_c);
					break;
				case TAC_LE: // a5 <= imm_c => slti a5, a5, imm_c + 1; xori a5, a5, 1
					I_TYPE_ARITH("slti", res_n, temp_n, imm_c + 1);
					I_TYPE_ARITH("xori", res_n, res_n, 1);
					break;
				case TAC_GT: // a5 > imm_c  => slti a5, a5, imm_c + 1; xori a5, a5, 1
					I_TYPE_ARITH("slti", res_n, temp_n, imm_c + 1);
					I_TYPE_ARITH("xori", res_n, res_n, 1);
					break;
				case TAC_GE: // a5 >= imm_c => slti a5, a5, imm_c; xori a5, a5, 1
					I_TYPE_ARITH("slti", res_n, temp_n, imm_c);
					I_TYPE_ARITH("xori", res_n, res_n, 1);
					break;
			}
		}
	I_TYPE_ARITH("andi", res_n, res_n, 0xff);
	asm_store_var(a,reg_name[res_reg]);
	if(res_reg==R_a5)rdesc_fill(R_a5,a,MODIFIED);
	}
}
// 生成条件跳转(ifz)对应的汇编代码
void asm_cond(char *op, struct id *a, const char *l) {
	//for (int r = R_GEN; r < R_NUM; r++) asm_write_back(r);

	if (a != NULL) {
		int r = reg_find(a);
		B_TYPE_BRANCH(op, reg_name[r], reg_name[R_zero], l); // 使用 B_TYPE_BRANCH 宏
	} else {
		J_TYPE_JUMP_PSEUDO(op, l); // 使用 J_TYPE_JUMP_PSEUDO 宏
	}
}
//XXX:需要考虑不同变量的大小，这里默认都是int
void asm_stack_pivot(struct tac* code) {
	oon = 0;
	int var_size = 0;
	int param_size = 0;
	struct tac *cur;
	for (cur = code; cur != NULL; cur = cur->next) {
		if(cur->type==TAC_VAR){
			var_size += ALIGN(TYPE_SIZE(cur->id_1->data_type));
		}
		else if(cur->type==TAC_PARAM){
			param_size += ALIGN(TYPE_SIZE(cur->id_1->data_type));
		}
		else if(cur->type==TAC_END){
			break;
		}
	}
	var_size = (var_size + 15) / 16*16;
	param_size = (param_size + 15) / 16*16;
	oon = var_size + param_size + 16;
	tof = LOCAL_OFF;
	oof = FORMAL_OFF-var_size;
	I_TYPE_ARITH("addi", "sp", "sp", -oon);
	S_TYPE_STORE("sw", "ra", "sp", oon - 4); // 使用 S_TYPE_STORE 宏
	S_TYPE_STORE("sw", "s0", "sp", oon - 8); // 使用 S_TYPE_STORE 宏
	I_TYPE_ARITH("addi", "s0", "sp", oon);   // 使用 I_TYPE_ARITH 宏
}
void asm_param(struct tac* code) {
	int cnt = 0;
	struct tac *cur = code->next;
	int data_size;
	while (cur->type == TAC_PARAM)
	{
		LOCAL_VAR_OFFSET(cur->id_1, oof);
		asm_store_var(cur->id_1, args_name[cnt]); // 使用 asm_store_var
		cur = cur->next;
		cnt++;
	}
}
// 生成函数调用对应的汇编代码
//XXX:没考虑大于8个参数如何传递
void asm_call(struct tac* code, struct id *a, struct id *b) {
	int r;
	int cnt = 0;
	// for (int r = R_GEN; r < R_NUM; r++) asm_write_back(r);
	for (int r = R_GEN; r < R_NUM; r++) rdesc_clear_all(r);
	
	struct tac *cur=code;
	//for (int i = 0; i < cop;i++)cur = cur->prev;
	while(cur->prev!=NULL&&cur->prev->type==TAC_ARG){
		cur = cur->prev;
		cnt++;
	}
	r = 0;
	while (cur->type==TAC_ARG)
	{
		r++;
		asm_load_var(cur->id_1, args_name[cnt - r]); // 使用 asm_load_var
		cur = cur->next;
	}
	J_TYPE_JUMP_PSEUDO("call", b->name); // 使用 J_TYPE_JUMP_PSEUDO 宏
	if (a != NULL) {
		asm_store_var(a, reg_name[R_a0]); // 使用 asm_store_var
		rdesc_fill(R_a0, a, MODIFIED);
	}
}
void asm_label(struct id *a) {
	for (int r = R_GEN; r < R_NUM; r++) rdesc_clear_all(r);
	if(a->id_type==ID_LABEL){
		input_str(obj_file, ".%s:\n", a->name);
	}
	else if(a->id_type==ID_FUNC){
		input_str(obj_file, "	.align	2\n");
		input_str(obj_file, "	.globl	%s\n", a->name);
		input_str(obj_file, "	.type	%s,@function\n", a->name);
		input_str(obj_file, "%s:\n", a->name);
	}
}

void asm_gvar(struct id *a) {
	int data_size = TYPE_SIZE(a->data_type);
	a->scope = 0; /* global var */
	input_str(obj_file, "	.globl	%s\n", a->name);
	input_str(obj_file, "	.bss\n");
	input_str(obj_file, "	.align	%d\n",TYPE_ALIGN(a->data_type));
	input_str(obj_file, "	.type	%s,@object\n", a->name);
	input_str(obj_file, "	.size %s, %d\n", a->name, data_size);
	input_str(obj_file, "%s:\n", a->name);
	input_str(obj_file, "	.zero	%d", data_size); 
}
// 生成函数返回对应的汇编代码
void asm_return(struct id *a) {
	//for (int r = R_GEN; r < R_NUM; r++) asm_write_back(r);
	//for (int r = R_GEN; r < R_NUM; r++) rdesc_clear_all(r);

	if (a != NULL) /* return value */
	{
		int r=reg_find(a);
		PSEUDO_2_REG("mv", reg_name[R_a0], reg_name[r]); // 使用 PSEUDO_2_REG 宏
	}

	I_TYPE_LOAD("lw", "ra", "sp", oon - 4); // 使用 I_TYPE_LOAD 宏
	I_TYPE_LOAD("lw", "s0", "sp", oon - 8); // 使用 I_TYPE_LOAD 宏
	I_TYPE_ARITH("addi", "sp", "sp", oon);  // 使用 I_TYPE_ARITH 宏
	J_TYPE_JUMP_REG("jr", "ra");           // 使用 J_TYPE_JUMP_REG 宏
}

void asm_load_var(struct id *s, const char *r) {
	if (s->id_type == ID_NUM) {
		U_TYPE_UPPER_IMM("li", r, s->num); // 使用 U_TYPE_UPPER_IMM 宏
	} else if (s->scope == GLOBAL_TABLE) {
		int addr_reg = reg_get();
		U_TYPE_UPPER_IMM("la", reg_name[addr_reg], s->name); // 使用 U_TYPE_UPPER_IMM 宏
		I_TYPE_LOAD(LOAD_OP(TYPE_SIZE(s->data_type)), r, reg_name[addr_reg], 0); // 使用 I_TYPE_LOAD 宏
	} else {
		I_TYPE_LOAD(LOAD_OP(TYPE_SIZE(s->data_type)), r, "s0", s->offset); // 使用 I_TYPE_LOAD 宏
	}
}

void asm_store_var(struct id *s, const char *r) {
	if (s->scope == GLOBAL_TABLE) {
		int addr_reg = reg_get();
		U_TYPE_UPPER_IMM("la", reg_name[addr_reg], s->name); // 使用 U_TYPE_UPPER_IMM 宏
		S_TYPE_STORE(STORE_OP(TYPE_SIZE(s->data_type)), r, reg_name[addr_reg], 0); // 使用 S_TYPE_STORE 宏
	} else {
		S_TYPE_STORE(STORE_OP(TYPE_SIZE(s->data_type)), r, "s0", s->offset); // 使用 S_TYPE_STORE 宏
	}
}