// hjj: tbd, float num
#include "o_wrap.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "e_tac.h"
#include "o_riscv.h"
/* global var */
int tos; /* top of static */
int tof; /* top of frame */
int oof; /* offset of formal */
int oon; /* offset of next frame */

// 根据单条三地址码，生成汇编代码
void asm_code(struct tac *code) {
	int r;

	switch (code->type) {
		case TAC_UNDEF:
			perror("cannot translate TAC_UNDEF");
			return;

		case TAC_PLUS:
			asm_bin("add", code->id_1, code->id_2, code->id_3);
			return;

		case TAC_MINUS:
			asm_bin("sub", code->id_1, code->id_2, code->id_3);
			return;

		case TAC_MULTIPLY:
			asm_bin("mul", code->id_1, code->id_2, code->id_3);
			return;

		case TAC_DIVIDE:
			asm_bin("div", code->id_1, code->id_2, code->id_3);
			return;

		case TAC_EQ:
		case TAC_NE:
		case TAC_LT:
		case TAC_LE:
		case TAC_GT:
		case TAC_GE:
			asm_cmp(code->type, code->id_1, code->id_2, code->id_3);
			return;

		case TAC_ASSIGN:
		case TAC_VAR_REFER_INIT:
			r = reg_find(code->id_2);
			if (code->id_1->variable_type->data_type ==
			    code->id_2->variable_type->data_type)
				rdesc_fill(
				    r, code->id_1,
				    MODIFIED);  // 只有类型相同时覆盖寄存器描述符，防止未截断错误
			asm_store_var(code->id_1, reg_name[r]);
			return;

		case TAC_INPUT:
			asm_input(code->id_1);
			return;

		case TAC_OUTPUT:
			asm_output(code->id_1);
			return;

		case TAC_REFER:
			asm_refer(code->id_1, code->id_2);
			return;

		case TAC_DEREFER_GET:
			asm_derefer_get(code->id_1, code->id_2);
			return;

		case TAC_DEREFER_PUT:
			asm_derefer_put(code->id_1, code->id_2);
			return;

		case TAC_GOTO:
			asm_cond("j", NULL, code->id_1->name);
			return;

		case TAC_IFZ:
			asm_cond("beq", code->id_1, code->id_2->name);
			return;

		case TAC_LABEL:
			// for (int r = R_GEN; r < R_NUM; r++) asm_write_back(r);
			asm_label(code->id_1);
			return;

		case TAC_ARG:
			// r = reg_find(code->id_1);
			// input_str(obj_file, "\tSTO (R2+%d),R%u\n", tof + oon, r);
			// oon += 4;
			return;

		case TAC_PARAM:

			return;

		case TAC_CALL:
			asm_call(code, code->id_1, code->id_2);
			return;

		case TAC_BEGIN:
			// 栈帧迁移
			scope = LOCAL_TABLE;
			asm_stack_pivot(code);
			asm_param(code);
			return;

		case TAC_END:
			// asm_return(NULL);
			scope = GLOBAL_TABLE;
			return;

		case TAC_VAR:
			if (scope == LOCAL_TABLE) {
				if (code->id_1->pointer_info.index != NO_INDEX) {
					LOCAL_ARRAY_OFFSET(code->id_1, tof,
					                   code->id_1->pointer_info.index);
				} else {
					LOCAL_VAR_OFFSET(code->id_1, tof);
				}
			} else {
				asm_gvar(code->id_1);
			}
			return;

		case TAC_RETURN:
			asm_return(code->id_1);
			return;

		default:
			/* Don't know what this one is */
			perror("unknown TAC opcode to translate");
			printf("type: %d\n", code->type);
			return;
	}
}

// 根据source_to_tac生成的三地址码，生成汇编代码
void tac_to_obj() {
	tof = LOCAL_OFF; /* TOS allows space for link info */
	oof = FORMAL_OFF;
	oon = 0;

	for (int r = 0; r < R_NUM; r++) rdesc[r].var = NULL;

	asm_head();

	struct tac *cur;
	for (cur = tac_head; cur != NULL; cur = cur->next) {
#if DEBUG_PRINT == 1
		input_str(obj_file, "\t\t\t\t\t\t\t# ");
		output_tac(obj_file, cur);
#endif
		// input_str(obj_file, "\n");
		asm_code(cur);
	}
	for (struct id *gconst = id_global; gconst != NULL; gconst = gconst->next) {
		if (ID_IS_GCONST(gconst->id_type, gconst->variable_type->data_type)) {
			asm_lc(gconst);
		}
	}
	asm_tail();
	// asm_static();
}

// 生成开始段
void asm_head() {
	char head[] =
	    "\t\t\t\t\t\t\t# head\n"
	    "\t.text\n";
	input_str(obj_file, "%s", head);
}

// 生成结束段
void asm_tail() {
	char tail[] =
	    "\n\t\t\t\t\t\t\t# tail\n"
	    "\t.ident \"tiny-compiler\"\n";

	input_str(obj_file, "%s", tail);
}

void asm_lc(struct id *s) {
	const char *t = s->name; /* The text */
	int i;
	input_str(obj_file, "\t.section	.rodata\n");
	input_str(obj_file, "\t.align	%d\n", TYPE_ALIGN(s->variable_type));
	input_str(obj_file, ".LC%u:\n", s->label); /* Label for the string */
	if (s->id_type == ID_STRING) {
		input_str(obj_file, "\t.string	\"%s\"\n", s->name);
	} else {
#if DEBUG_PRINT == 1
		input_str(obj_file, "\t\t\t\t\t\t\t#	%s\n", t);
#endif
		for (int i = DATA_SIZE(DATA_DOUBLE) - TYPE_SIZE(s->variable_type);
		     i < DATA_SIZE(DATA_DOUBLE); i += DATA_SIZE(DATA_FLOAT)) {
			int temp;
			memcpy(&temp, (void *)(&s->number_info.num) + 0,
			       DATA_SIZE(DATA_FLOAT));
			input_str(obj_file, "\t.word	%d\n", temp);
		}
	}
}

// 生成静态数据段
// XXX:没用了
void asm_static(void) {
	int i;

	struct id *sl;

	// for (sl = id_global; sl != NULL; sl = sl->next) {
	// 	if (sl->id_type == ID_STRING) asm_str(sl);
	// }

	input_str(obj_file, "STATIC:\n");
	input_str(obj_file, "\tDBN 0,%u\n", tos);
	input_str(obj_file, "STACK:\n");
}
