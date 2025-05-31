// hjj: tbd, float num
#include "o_reg.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "e_tac.h"
#include "o_riscv.h"

/* global var */
struct rdesc rdesc[16];
const char *reg_name[] = {
    "a5",  "a4", "a3", "a2", "a1", "a0", "a6", "a7",
    "t0",  "t1", "t2", "t3", "t4", "t5", "t6",  // 可分配的
    "zero"};
const char *args_name[] = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};

// 清除某变量对应的所有非最新的寄存器描述符
void rdesc_clear_prev(int r) {
	rdesc[r].var = NULL;
	rdesc[r].mod = 0;
	rdesc[r].next = NULL;
	if (rdesc[r].prev != NULL) {
		int pre = RDESC_NUM(rdesc[r].prev);
		rdesc[r].prev = NULL;
		rdesc_clear_all(pre);
	}
}
// 清空寄存器描述符
// 由于reg_alloc不只是在bin和cmp里使用，所以不能删去clear_al
void rdesc_clear_all() {
	for (int i = R_GEN; i < R_NUM; i++) {
		rdesc_clear_prev(i);
	}
}
// 清除临时的寄存器描述符
void rdesc_clear_temp(int r) {
	rdesc[r].var = NULL;
	rdesc[r].mod = 0;
	if (rdesc[r].prev != NULL) {
		rdesc[r].prev->next = NULL;
	}
	rdesc[r].prev = NULL;
}

// 填充寄存器描述符
void rdesc_fill(int r, struct id *s, int mod) {
	// hjj: 需要允许同时有数个寄存器存储同一符号的情况，在asm_cmp会用到
	// hjj: 用链表存储某个var对应的rdesc的先后次序
	// 存在该var的链表
	int first_appear;
	for (first_appear = R_GEN; first_appear < R_NUM; first_appear++) {
		if (rdesc[first_appear].var == s) {
			FIND_LATEST_RDESC(first_appear, latest_appear);
			if (r != latest_appear) {
				rdesc[latest_appear].next = &rdesc[r];
				rdesc[r].prev = &rdesc[latest_appear];
				rdesc[r].var = s;
				rdesc[r].mod = mod;
			}
			return;  // hjj: 对每个var只有一条链表及头结点，也就是最小的rdesc
		}
	}
	// 不存在该var的链表，需要将r脱链
	struct rdesc *tp = rdesc[r].prev;
	if (tp != NULL) {
		tp->next = rdesc[r].next;
	}
	if (rdesc[r].next != NULL) {
		rdesc[r].next->prev = tp;
	}
	rdesc[r].var = s;
	rdesc[r].mod = mod;
	rdesc[r].next = NULL;
	rdesc[r].prev = NULL;
}
// 写回寄存器内容至内存
// XXX:没用了/有点用
// hjj: 不能直接用STO&LOD，先注释掉。
void asm_write_back(int r) {
	// if ((rdesc[r].var != NULL) && rdesc[r].mod) {
	// 	if (rdesc[r].var->scope == 1) /* local var */
	// 	{
	// 		input_str(obj_file, "\tSTO (R%u+%u),R%u\n", R_BP,
	// 		          rdesc[r].var->offset, r);
	// 	} else /* global var */
	// 	{
	// 		input_str(obj_file, "\tLOD R%u,STATIC\n", R_TP);
	// 		input_str(obj_file, "\tSTO (R%u+%u),R%u\n", R_TP,
	// 		          rdesc[r].var->offset, r);
	// 	}
	// 	rdesc[r].mod = UNMODIFIED;
	// }
	perror("tbd write back");
#ifndef HJJ_DEBUG
	exit(0);
#endif
}

// 加载符号到寄存器
void asm_load(int r, struct id *s) {
	/* already in a reg */
	for (int first_appear = R_GEN; first_appear < R_NUM; first_appear++) {
		if (rdesc[first_appear].var == s) {
			// hjj: 应该找最近被修改的寄存器，找下一个
			FIND_LATEST_RDESC(first_appear, latest_appear);

			/* load from the reg */
			if (r != latest_appear) {
				PSEUDO_2_REG("mv", reg_name[r],
				             reg_name[latest_appear]);  // 使用 PSEUDO_2_REG 宏
				rdesc[latest_appear].next = &rdesc[r];
				rdesc[r].prev = &rdesc[latest_appear];
			}
			return;
		}
	}

	/* not in a reg */
	asm_load_var(s, reg_name[r]);
	// rdesc_fill(r, s, UNMODIFIED);
}
// 为符号分配寄存器
int reg_alloc(struct id *s) {
	int r = reg_get();
	asm_load(r, s);
	rdesc_fill(r, s, UNMODIFIED);
	return r;
}

int reg_get() {
	int r;

	/* empty register */
	for (r = R_GEN; r < R_NUM; r++) {
		if (rdesc[r].var == NULL) {
			return r;
		}
	}

	/* unmodifed register */
	for (r = R_GEN; r < R_NUM; r++) {
		if (!rdesc[r].mod) {
			return r;
		}
	}
	/* random register */
	srand(time(NULL));
	r = (rand() % (R_NUM - R_GEN)) + R_GEN;
	asm_write_back(r);
	return r;
}
// 寻找符号对应的最晚被修改的寄存器
int reg_find(struct id *s) {
	int first_appear;
	if (s->id_type == ID_NUM && strcmp(s->name, "0") == 0) return R_zero;
	/* already in a register */
	for (first_appear = R_GEN; first_appear < R_NUM; first_appear++) {
		if (rdesc[first_appear].var == s) {
			FIND_LATEST_RDESC(first_appear, latest_appear);
			return latest_appear;
		}
	}

	return reg_alloc(s);
}

void asm_load_var(struct id *s, const char *r) {
	if (ID_IS_GCONST(
	        s->id_type,
	        s->variable_type->data_type)) {    // XXX:不知道适不适配string
		U_TYPE_UPPER_SYM("lla", r, s->label);  // 使用 U_TYPE_UPPER_IMM 宏
		I_TYPE_LOAD(LOAD_OP(TYPE_SIZE(s->variable_type, NO_INDEX)), r, r, 0);
	} else if (ID_IS_INTCONST(s->id_type, s->variable_type->data_type)) {
		U_TYPE_UPPER_IMM("li", r,
		                 s->number_info.num);  // 使用 U_TYPE_UPPER_IMM 宏
	} else {                                   // TEMP or VAR
		if (s->scope == GLOBAL_TABLE) {
			U_TYPE_UPPER_SYM("la", r, s->name);  // 使用 U_TYPE_UPPER_IMM 宏
			I_TYPE_LOAD(LOAD_OP(TYPE_SIZE(s->variable_type, NO_INDEX)), r, r,
			            0);  // 使用 I_TYPE_LOAD 宏
		} else {
			I_TYPE_LOAD(LOAD_OP(TYPE_SIZE(s->variable_type, NO_INDEX)), r, "s0",
			            s->offset);  // 使用 I_TYPE_LOAD 宏
		}
	}
}

void asm_store_var(struct id *s, const char *r) {
	if (s->scope == GLOBAL_TABLE) {
		int addr_reg = reg_get();
		U_TYPE_UPPER_SYM("la", reg_name[addr_reg],
		                 s->name);  // 使用 U_TYPE_UPPER_IMM 宏
		S_TYPE_STORE(STORE_OP(TYPE_SIZE(s->variable_type, NO_INDEX)), r,
		             reg_name[addr_reg],
		             0);  // 使用 S_TYPE_STORE 宏
	} else {
		S_TYPE_STORE(STORE_OP(TYPE_SIZE(s->variable_type, NO_INDEX)), r, "s0",
		             s->offset);  // 使用 S_TYPE_STORE 宏
	}
}