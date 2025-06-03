#include "e_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_proc.h"
#include "e_tac.h"

struct op *__deref(struct op *exp) {
	struct op *deref_stat = new_op();

	struct id *t_1 = exp->addr;
	if (!t_1->variable_type->pointer_level) {
		perror("data is not a pointer");
#ifndef HJJ_DEBUG
		exit(0);
#endif
	}
	while (exp->deref_count) {
		struct id *t_2 =
		    new_temp(new_var_type(t_1->variable_type->data_type,
		                          t_1->variable_type->pointer_level - 1,
		                          t_1->variable_type->is_reference));
		cat_tac(deref_stat, NEW_TAC_1(TAC_VAR, t_2));
		cat_tac(deref_stat, NEW_TAC_2(TAC_DEREFER_GET, t_2, t_1));
		t_1 = t_2;
		exp->deref_count--;
	}
	deref_stat->addr = t_1;
	return deref_stat;
}

struct op *process_derefer(struct op *id_op) {
	struct op *content_stat = id_op;

	content_stat->deref_count++;

	return content_stat;
}

// 处理被解引用并向内赋值的指针
struct op *process_derefer_put(struct op *id_op) {
	struct op *content_stat = new_op();

	struct id *var = id_op->addr;
	if (!var->variable_type->pointer_level) {
		perror("data is not a pointer");
		printf("data name: %s\n", var->name);
		printf("data type: %d\n", var->variable_type->data_type);
#ifndef HJJ_DEBUG
		exit(0);
#endif
	}

	// struct id *t = new_temp(var->variable_type);
	// t->pointer_info.temp_deref_count = id_op->deref_count;
	content_stat->addr = var;
	content_stat->deref_count = id_op->deref_count;

	cat_op(content_stat, id_op);
	// cat_tac(content_stat, NEW_TAC_1(TAC_VAR, t));
	// cat_tac(content_stat, NEW_TAC_2(TAC_ASSIGN, t, var));

	return content_stat;
}

// 处理被解引用并从内取值的指针
struct op *process_derefer_get(struct op *id_op) { return __deref(id_op); }
// cur=t19 一级指针，
//  处理指针对变量的引用
struct op *process_reference(struct op *id_op) {
	struct id *var = id_op->addr;
	if (id_op->deref_count) {
		return id_op;
	}
	struct op *pointer_stat = new_op();

	struct id *t =
	    new_temp(new_var_type(var->variable_type->data_type,
	                          var->variable_type->pointer_level + 1, 0));
	pointer_stat->addr = t;

	cat_op(pointer_stat, id_op);
	cat_tac(pointer_stat, NEW_TAC_1(TAC_VAR, t));
	if (var->variable_type->is_reference) {
		cat_tac(pointer_stat, NEW_TAC_2(TAC_ASSIGN, t, var));
	} else {
		cat_tac(pointer_stat, NEW_TAC_2(TAC_REFER, t, var));
	}

	return pointer_stat;
}
// 分配整数型数字符号
struct op *process_int(int integer) {
	struct op *int_op = new_op();

	BUF_ALLOC(buf);  // 声明一个char数组变量buf，储存符号名
	sprintf(buf, "%d", integer);
	struct id *var = add_const_identifier(
	    buf, ID_NUM,
	    new_const_type(DATA_INT, 0));  // 向符号表添加以buf为名的符号
	var->number_info.num.num_int = integer;
	int_op->addr = var;

	return int_op;
}

// 分配浮点型数字符号
struct op *process_float(double floatnum) {
	struct op *float_op = new_op();

	BUF_ALLOC(buf);
	sprintf(buf, "%f", floatnum);
	struct id *var =
	    add_const_identifier(buf, ID_NUM, new_const_type(DATA_FLOAT, 0));
	var->number_info.num.num_float = floatnum;
	float_op->addr = var;

	return float_op;
}

// 分配字符型数字符号
struct op *process_char(char character) {
	struct op *char_op = new_op();

	BUF_ALLOC(buf);
	sprintf(buf, "%c", character);
	struct id *var =
	    add_const_identifier(buf, ID_NUM, new_const_type(DATA_CHAR, 0));
	var->number_info.num.num_char = character;
	char_op->addr = var;

	return char_op;
}
