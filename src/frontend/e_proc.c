#include "e_proc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_tac.h"
#include "o_reg.h"

struct tac *tac_head;
static struct tac *arg_list_head;
static struct block *block_top;

/**************************************/
/************ expression **************/
/**************************************/
// 处理形如"a=a op b"的表达式
struct op *process_calculate(struct op *exp_l, struct op *exp_r, int cal) {
	struct op *exp = new_op();
	if (exp_l == NUM_ZERO) exp_l = process_int(0);
	if (exp_l == NUM_ONE) exp_l = process_int(1);

	struct id *exp_l_addr = exp_l->addr;
	struct id *exp_r_addr = exp_r->addr;

	struct id *t = new_temp(exp_l->addr->variable_type);  // 分配临时变量
	exp->addr = t;
	cat_tac(exp, NEW_TAC_1(TAC_VAR, t));
	cat_op(exp, exp_l);  // 拼接exp和exp_l的code
	cat_op(exp, exp_r);  // 拼接exp和exp_r的code

	if (exp_l_addr->variable_type->is_reference) {
		struct id *tl =
		    new_temp(new_var_type(exp_l_addr->variable_type->data_type,
		                          exp_l_addr->variable_type->pointer_level, 0));

		cat_tac(exp, NEW_TAC_1(TAC_VAR, tl));
		cat_tac(exp, NEW_TAC_2(TAC_DEREFER_GET, tl, exp_l_addr));

		exp_l_addr = tl;
		t->variable_type->data_type = tl->variable_type->data_type;
	}
	if (exp_r_addr->variable_type->is_reference) {
		struct id *tr =
		    new_temp(new_var_type(exp_r_addr->variable_type->data_type,
		                          exp_r_addr->variable_type->pointer_level, 0));

		cat_tac(exp, NEW_TAC_1(TAC_VAR, tr));
		cat_tac(exp, NEW_TAC_2(TAC_DEREFER_GET, tr, exp_r_addr));

		exp_r_addr = tr;
	}

	if (exp_l_addr->variable_type->data_type == DATA_FLOAT ||
	    exp_r_addr->variable_type->data_type == DATA_FLOAT) {
		// 对于浮点数的运算，生成调用内部函数的三地址码

		if ((TYPE_CHECK(exp_l_addr, exp_r_addr)) == 0) {
			if (exp_l_addr->variable_type->data_type == DATA_FLOAT) {
				struct op *cast_exp = type_casting(exp_l_addr, exp_r_addr);
				exp_r_addr = cast_exp->addr;
				cat_op(exp, cast_exp);
			} else if (exp_r_addr->variable_type->data_type == DATA_FLOAT) {
				struct op *cast_exp = type_casting(exp_r_addr, exp_l_addr);
				exp_l_addr = cast_exp->addr;
				cat_op(exp, cast_exp);
			}
		}
		struct id *func;

		NEW_BUILT_IN_FUNC_ID(func, TAC_TO_FUNC(cal), DATA_FLOAT);

		cat_tac(exp, NEW_TAC_1(TAC_ARG, exp_r_addr));  // 生成 arg b
		cat_tac(exp, NEW_TAC_1(TAC_ARG, exp_l_addr));  // 生成 arg a
		cat_tac(exp, NEW_TAC_2(TAC_CALL, t, func));    // 生成 t1=call func
		if (TAC_IS_CMP(cal)) {
			struct id *label_1 = new_label();
			struct id *label_2 = new_label();

			struct id *const_0 = add_const_identifier(
			    "0", ID_NUM, new_const_type(DATA_INT, 0));
			const_0->number_info.num.num_int = 0;
			struct id *const_1 = add_const_identifier(
			    "1.0", ID_NUM, new_const_type(DATA_FLOAT, 0));
			const_1->number_info.num.num_float = 1.0;

			cat_tac(exp, NEW_TAC_2(TAC_IFZ, t, label_1));
			cat_tac(exp, NEW_TAC_2(TAC_ASSIGN, t, const_1));
			cat_tac(exp, NEW_TAC_1(TAC_GOTO, label_2));
			cat_tac(exp, NEW_TAC_1(TAC_LABEL, label_1));
			cat_tac(exp, NEW_TAC_2(TAC_ASSIGN, t, const_0));
			cat_tac(exp, NEW_TAC_1(TAC_LABEL, label_2));
		}
	} else {
		cat_tac(
		    exp,
		    NEW_TAC_3(
		        cal, exp->addr, exp_l_addr,
		        exp_r_addr));  // 生成代表目标表达式的三地址码，并拼接至exp的code末尾
	}
	return exp;
}

// 处理数组标识符
struct op *process_array_identifier(struct op *array_op, int index) {
	struct op *id_op = new_op();

	struct id *array = array_op->addr;
	if (!array->variable_type->pointer_level) {
		perror("id is not from an array");
#ifndef HJJ_DEBUG
		exit(0);
#endif
	}

	struct id *t = new_temp(array->variable_type);
	struct op *num_op = process_int(index * TYPE_SIZE(array->variable_type));
	id_op->addr = t;
	cat_tac(id_op, NEW_TAC_1(TAC_VAR, t));
	cat_tac(id_op, NEW_TAC_3(TAC_PLUS, t, array, num_op->addr));

	return id_op;
}

struct op *process_add_identifier(char *name, int index) {
	struct op *id_op = new_op();

	struct id *var = add_identifier(name, ID_VAR, NO_TYPE, index);

	cat_tac(id_op, NEW_TAC_1(TAC_VAR, var));

	return id_op;
}

// 处理标识符
struct op *process_identifier(char *name) {
	struct op *id_op = new_op();

	struct id *var = find_identifier(name);
	id_op->addr = var;

	return id_op;
}

struct op *process_instance_member(char *instance_name, char *member_name) {
	struct op *instance_op = new_op();

	struct id *instance = find_identifier(instance_name);
	struct member_def *member = find_member(instance, member_name);
	struct id *t_member =
	    new_temp(new_var_type(member->variable_type->data_type,
	                          member->variable_type->pointer_level, 1));
	struct id *t_member_addr =
	    new_temp(new_var_type(member->variable_type->data_type,
	                          member->variable_type->pointer_level + 1, 0));
	struct id *t_instance_addr =
	    new_temp(new_var_type(instance->variable_type->data_type,
	                          member->variable_type->pointer_level + 1, 0));
	instance_op->addr = t_member;

	cat_tac(instance_op, NEW_TAC_1(TAC_VAR, t_member));
	cat_tac(instance_op, NEW_TAC_1(TAC_VAR, t_member_addr));
	cat_tac(instance_op, NEW_TAC_1(TAC_VAR, t_instance_addr));
	cat_tac(instance_op, NEW_TAC_2(TAC_REFER, t_instance_addr, instance));
	cat_tac(instance_op, NEW_TAC_3(TAC_PLUS, t_member_addr, t_instance_addr,
	                               process_int(member->member_offset)->addr));
	cat_tac(instance_op,
	        NEW_TAC_2(TAC_VAR_REFER_INIT, t_member, t_member_addr));

	return instance_op;
}

struct op *process_pointer_instance_member(char *instance_ptr_name,
                                           char *member_name) {
	struct op *instance_op = new_op();

	struct id *instance_ptr = find_identifier(instance_ptr_name);
	struct member_def *member = find_member(instance_ptr, member_name);
	struct id *t_member =
	    new_temp(new_var_type(member->variable_type->data_type,
	                          member->variable_type->pointer_level, 1));
	struct id *t_member_addr =
	    new_temp(new_var_type(member->variable_type->data_type,
	                          member->variable_type->pointer_level + 1, 0));
	instance_op->addr = t_member;

	cat_tac(instance_op, NEW_TAC_1(TAC_VAR, t_member));
	cat_tac(instance_op, NEW_TAC_1(TAC_VAR, t_member_addr));
	cat_tac(instance_op, NEW_TAC_3(TAC_PLUS, t_member_addr, instance_ptr,
	                               process_int(member->member_offset)->addr));
	cat_tac(instance_op,
	        NEW_TAC_2(TAC_VAR_REFER_INIT, t_member, t_member_addr));

	return instance_op;
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

// 处理形如"a++"和"++a"的表达式
struct op *process_inc(struct op *id_op, int pos) {
	struct op *inc_op = new_op();
	struct op *inc_temp = new_op();

	struct id *var = id_op->addr;
	inc_temp->addr = var;

	if (var->variable_type->data_type == DATA_FLOAT ||
	    var->variable_type->data_type == DATA_DOUBLE) {
		perror("wrong type");
	}
	if (pos == INC_HEAD) {
		struct op *inc_done = process_calculate(NUM_ONE, inc_temp, TAC_PLUS);
		inc_op->addr = var;

		cat_op(inc_op, inc_done);
		cat_tac(inc_op, NEW_TAC_2(TAC_ASSIGN, var, inc_done->addr));
	} else {
		struct id *temp_pre = new_temp(var->variable_type);
		struct op *inc_done = process_calculate(NUM_ONE, inc_temp, TAC_PLUS);
		inc_op->addr = temp_pre;

		cat_tac(inc_op, NEW_TAC_2(TAC_ASSIGN, inc_done->addr, var));
		cat_op(inc_op, inc_done);
		cat_tac(inc_op, NEW_TAC_2(TAC_ASSIGN, var, inc_done->addr));
	}

	return inc_op;
}

// 处理形如"a--"和"--a"的表达式
struct op *process_dec(struct op *id_op, int pos) {
	struct op *dec_op = new_op();
	struct op *dec_temp = new_op();

	struct id *var = id_op->addr;
	dec_temp->addr = var;

	if (var->variable_type->data_type == DATA_FLOAT ||
	    var->variable_type->data_type == DATA_DOUBLE) {
		perror("wrong type");
	}
	if (pos == INC_HEAD) {
		struct op *inc_done = process_calculate(NUM_ONE, dec_temp, TAC_MINUS);
		dec_op->addr = var;

		cat_op(dec_op, inc_done);
		cat_tac(dec_op, NEW_TAC_2(TAC_ASSIGN, var, inc_done->addr));
	} else {
		struct id *temp_pre = new_temp(var->variable_type);
		struct op *inc_done = process_calculate(NUM_ONE, dec_temp, TAC_MINUS);
		dec_op->addr = temp_pre;

		cat_tac(dec_op, NEW_TAC_2(TAC_ASSIGN, inc_done->addr, var));
		cat_op(dec_op, inc_done);
		cat_tac(dec_op, NEW_TAC_2(TAC_ASSIGN, var, inc_done->addr));
	}

	return dec_op;
}

// 处理实参列表
struct op *process_argument_list(struct op *raw_exp_list) {
	struct op *argument_list = new_op();

	cat_op(argument_list, raw_exp_list);
	cat_tac(argument_list, arg_list_head);

	struct tac *arg = arg_list_head;

	return argument_list;
}

// 处理表达式列表的开端，在调用函数时生成实参
struct op *process_expression_list_head(struct op *arg_exp) {
	struct op *exp = new_op();

	struct id *exp_temp = arg_exp->addr;  // not *temp var*
	struct tac *arg = NEW_TAC_1(TAC_ARG, exp_temp);
	arg->next = NULL;
	arg_list_head = arg;

	cat_op(exp, arg_exp);

	return exp;
}

// 处理表达式列表，在调用函数时生成实参
struct op *process_expression_list(struct op *arg_list_pre,
                                   struct op *arg_exp) {
	struct op *exp_list = new_op();

	struct id *exp_temp = arg_exp->addr;  // not *temp var*
	struct tac *arg = NEW_TAC_1(TAC_ARG, exp_temp);
	arg->next = arg_list_head;
	arg_list_head->prev = arg;
	arg_list_head = arg;

	cat_op(exp_list, arg_exp);
	cat_op(exp_list, arg_list_pre);

	return exp_list;
}

struct var_type *process_struct_type(char *name, int pointer_level,
                                     int is_reference) {
	return new_var_type(check_struct_name(name)->struct_info.struct_type,
	                    pointer_level, is_reference);
}

/**************************************/
/************* statement **************/
/**************************************/
// 可以考虑用注释形式放到三地址文件？
struct op *process_struct_definition(char *name,
                                     struct member_def *definition_block) {
	struct op *struct_op = new_op();
	static int cur_struct_type = DATA_STRUCT_INIT;

	struct id *new_struct = add_identifier(name, ID_STRUCT, NO_TYPE, NO_INDEX);
	new_struct->struct_info.definition_list = definition_block;
	new_struct->struct_info.next_struct = struct_table;
	new_struct->struct_info.struct_type = cur_struct_type++;
	new_struct->struct_info.struct_offset = cur_member_offset;
	struct_table = new_struct;
	cur_member_offset = 0;

	return struct_op;
}

struct member_def *process_definition(struct var_type *variable_type,
                                      struct member_def *definition_list) {
	struct member_def *cur_definition = definition_list;
	while (cur_definition) {
		cur_definition->variable_type = variable_type;
		cur_definition->member_offset = cur_member_offset;
		cur_member_offset += TYPE_SIZE(variable_type);
		cur_definition = cur_definition->next_def;
	}
	return definition_list;
}

// 处理变量声明，为process_variable函数声明的变量加上类型
struct op *process_declaration(struct var_type *variable_type,
                               struct op *declaration_exp) {
	struct op *declaration = new_op();

	struct tac *cur_declaration = declaration_exp->code;
	while (
	    cur_declaration) {  // 逐个修改包含已声明变量的declaration_exp表达式所含变量的类型
		cur_declaration->id_1->variable_type = variable_type;
		if (cur_declaration->id_1->pointer_info.index != NO_INDEX) {
			cur_declaration->id_1->variable_type->pointer_level = 1;
		}
		cur_declaration = cur_declaration->next;
	}
	cat_op(declaration, declaration_exp);

	return declaration;
}

static void push_block_stack(struct id *label_begin, struct id *label_end) {
	struct block *block_pushed = new_block(label_begin, label_end);
	block_pushed->prev = block_top;
	block_top = block_pushed;
}

static void pop_block_stack() {
	struct block *block_poped = block_top;
	if (block_top == NULL) {
		perror("stack is empty");
	}
	block_top = block_top->prev;
	free(block_poped);
}

static void parse_labels() {
	while (block_top->continue_stat_head) {
		block_top->continue_stat_head->code->id_1 = block_top->label_begin;
		block_top->continue_stat_head = block_top->continue_stat_head->next;
	}
	while (block_top->break_stat_head) {
		block_top->break_stat_head->code->id_1 = block_top->label_end;
		block_top->break_stat_head = block_top->break_stat_head->next;
	}
}

void block_initialize() {
	struct id *label_begin = new_label();
	struct id *label_end = new_label();
	push_block_stack(label_begin, label_end);
}

// 处理for语句块
struct op *process_for(struct op *initialization_exp, struct op *condition_exp,
                       struct op *iteration_exp, struct op *block) {
	struct op *for_stat = new_op();

	struct id *exp_temp = condition_exp->addr;

	parse_labels();
	cat_op(for_stat, initialization_exp);
	cat_tac(for_stat, NEW_TAC_1(TAC_LABEL, block_top->label_begin));
	cat_op(for_stat, condition_exp);
	if (exp_temp) {  // 如果condition_exp不为空，则拼接label_2
		cat_tac(for_stat, NEW_TAC_2(TAC_IFZ, exp_temp, block_top->label_end));
	}
	cat_op(for_stat, block);
	cat_op(for_stat, iteration_exp);
	cat_tac(for_stat, NEW_TAC_1(TAC_GOTO, block_top->label_begin));
	if (exp_temp) {  // 如果condition_exp不为空，则拼接label_2
		cat_tac(for_stat, NEW_TAC_1(TAC_LABEL, block_top->label_end));
	}

	pop_block_stack();

	return for_stat;
}

// 处理while语句块
struct op *process_while(struct op *condition_exp, struct op *block) {
	struct op *while_stat = new_op();

	struct id *exp_temp = condition_exp->addr;

	parse_labels();
	cat_tac(while_stat, NEW_TAC_1(TAC_LABEL, block_top->label_begin));
	cat_op(while_stat, condition_exp);
	cat_tac(while_stat, NEW_TAC_2(TAC_IFZ, exp_temp, block_top->label_end));
	cat_op(while_stat, block);
	cat_tac(while_stat, NEW_TAC_1(TAC_GOTO, block_top->label_begin));
	cat_tac(while_stat, NEW_TAC_1(TAC_LABEL, block_top->label_end));

	pop_block_stack();

	return while_stat;
}

// 处理break语句
struct op *process_break() {
	struct op *break_stat = new_op();

	struct id *dummy_label = NULL;
	cat_tac(break_stat, NEW_TAC_1(TAC_GOTO, dummy_label));

	if (block_top == NULL) {
		perror("break not in a loop");
	}
	break_stat->next = block_top->break_stat_head;
	block_top->break_stat_head = break_stat;

	return break_stat;
}

// 处理continue语句
struct op *process_continue() {
	struct op *continue_stat = new_op();

	struct id *dummy_label = NULL;
	cat_tac(continue_stat, NEW_TAC_1(TAC_GOTO, dummy_label));

	if (block_top == NULL) {
		perror("continue not in a loop");
	}
	continue_stat->next = block_top->continue_stat_head;
	block_top->continue_stat_head = continue_stat;

	return continue_stat;
}

struct op *process_if_only(struct op *condition_exp, struct op *block) {
	struct op *if_only_stat = new_op();

	struct id *label = new_label();
	struct id *exp_temp = condition_exp->addr;

	cat_op(if_only_stat, condition_exp);
	cat_tac(if_only_stat, NEW_TAC_2(TAC_IFZ, exp_temp, label));
	cat_op(if_only_stat, block);
	cat_tac(if_only_stat, NEW_TAC_1(TAC_LABEL, label));

	return if_only_stat;
}

// 处理if else语句块
struct op *process_if_else(struct op *condition_exp, struct op *if_block,
                           struct op *else_block) {
	struct op *if_else_stat = new_op();

	struct id *label_1 = new_label();
	struct id *label_2 = new_label();
	struct id *exp_temp = condition_exp->addr;

	cat_op(if_else_stat, condition_exp);
	cat_tac(if_else_stat, NEW_TAC_2(TAC_IFZ, exp_temp, label_1));
	cat_op(if_else_stat, if_block);
	cat_tac(if_else_stat, NEW_TAC_1(TAC_GOTO, label_2));
	cat_tac(if_else_stat, NEW_TAC_1(TAC_LABEL, label_1));
	cat_op(if_else_stat, else_block);
	cat_tac(if_else_stat, NEW_TAC_1(TAC_LABEL, label_2));

	return if_else_stat;
}

// 处理call表达式
struct op *process_call(char *name, struct op *arg_list) {
	struct op *call_stat = new_op();

	struct id *func = find_func(name);
	struct id *t = new_temp(func->variable_type);
	call_stat->addr = t;

	struct tac *arg = arg_list->code;
	struct op *cast_arg_list =
	    param_args_type_casting(func->function_info.param_list, arg_list);

	if (func->variable_type->data_type != DATA_VOID) {
		cat_tac(call_stat, NEW_TAC_1(TAC_VAR, t));
	}
	cat_op(call_stat, cast_arg_list);
	if (func->variable_type->data_type != DATA_VOID) {
		cat_tac(call_stat, NEW_TAC_2(TAC_CALL, t, func));
	}

	return call_stat;
}

// 处理return表达式
struct op *process_return(struct op *ret_exp) {
	struct op *return_stat = new_op();

	struct id *exp_temp = ret_exp->addr;

	cat_op(return_stat, ret_exp);
	cat_tac(return_stat, NEW_TAC_1(TAC_RETURN, exp_temp));

	return return_stat;
}

// 处理output变量的表达式
struct op *process_output_variable(char *name) {
	struct op *output_stat = new_op();

	struct id *var = find_identifier(name);

	cat_tac(output_stat, NEW_TAC_1(TAC_OUTPUT, var));

	return output_stat;
}

// 处理output文本的表达式
struct op *process_output_text(char *string) {
	struct op *output_stat = new_op();

	struct id *str = add_const_identifier(
	    string, ID_STRING, new_const_type(DATA_UNDEFINED, 0));

	cat_tac(output_stat, NEW_TAC_1(TAC_OUTPUT, str));

	return output_stat;
}

// 处理input表达式
struct op *process_input(char *name) {
	struct op *input_stat = new_op();

	struct id *var = find_identifier(name);

	cat_tac(input_stat, NEW_TAC_1(TAC_INPUT, var));

	return input_stat;
}

// 处理赋值表达式
struct op *process_assign(struct op *leftval_op, struct op *exp) {
	struct op *assign_stat = new_op();

	struct id *leftval = leftval_op->addr;
	struct id *exp_temp = exp->addr;
	assign_stat->addr = exp_temp;

	cat_op(assign_stat, exp);
	if (!TYPE_CHECK(leftval, exp_temp) &&
	    !REF_TO_CONTENT(leftval->variable_type, exp_temp->variable_type) &&
	    !CONTENT_TO_REF(leftval->variable_type, exp_temp->variable_type) &&
	    !(leftval->pointer_info.temp_derefer_put &&
	      leftval->variable_type->pointer_level - 1 ==
	          exp_temp->variable_type->pointer_level)) {
		struct op *cast_exp = type_casting(leftval, exp_temp);
		exp_temp = cast_exp->addr;
		cat_op(assign_stat, cast_exp);
		printf("going to cst!\n");
	}
	cat_op(assign_stat, leftval_op);
	if (leftval->variable_type->is_reference) {
		cat_tac(assign_stat, NEW_TAC_2(TAC_DEREFER_PUT, leftval, exp_temp));
	} else if (exp_temp->variable_type->is_reference) {
		cat_tac(assign_stat, NEW_TAC_2(TAC_DEREFER_GET, leftval, exp_temp));
	} else if (leftval->pointer_info.temp_derefer_put) {
		cat_tac(assign_stat, NEW_TAC_2(TAC_DEREFER_PUT, leftval, exp_temp));
	} else {
		cat_tac(assign_stat, NEW_TAC_2(TAC_ASSIGN, leftval, exp_temp));
	}

	return assign_stat;
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

	struct id *t = new_temp(var->variable_type);
	t->pointer_info.temp_derefer_put = 1;
	content_stat->addr = t;

	cat_op(content_stat, id_op);
	cat_tac(content_stat, NEW_TAC_1(TAC_VAR, t));
	cat_tac(content_stat, NEW_TAC_2(TAC_ASSIGN, t, var));

	return content_stat;
}

// 处理被解引用并从内取值的指针
struct op *process_derefer_get(struct op *id_op) {
	struct op *content_stat = new_op();

	struct id *var = id_op->addr;
	if (!var->variable_type->pointer_level) {
		perror("data is not a pointer");
#ifndef HJJ_DEBUG
		exit(0);
#endif
	}

	struct id *t =
	    new_temp(new_var_type(var->variable_type->data_type,
	                          var->variable_type->pointer_level - 1, 0));
	content_stat->addr = t;

	cat_op(content_stat, id_op);
	cat_tac(content_stat, NEW_TAC_1(TAC_VAR, t));
	cat_tac(content_stat, NEW_TAC_2(TAC_DEREFER_GET, t, var));

	return content_stat;
}

// 处理指针对变量的引用
struct op *process_reference(struct op *id_op) {
	struct op *pointer_stat = new_op();

	struct id *var = id_op->addr;

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

/**************************************/
/********* function & program *********/
/**************************************/
// 处理整个程序，输出code
struct op *process_program(struct op *program) {
	// printf("program compiled to tac!\n");

	tac_head = program->code;

	// clear_table(GLOBAL_TABLE);
	// clear_table(LOCAL_TABLE);

	return program;
}

// 处理函数
struct op *process_function(struct op *function_head, struct op *block) {
	struct op *function = new_op();

	cat_op(function, function_head);
	cat_op(function, block);
	cat_tac(function, NEW_TAC_0(TAC_END));

	return function;
}

// 处理函数头
struct op *process_function_head(struct var_type *variable_type, char *name,
                                 struct op *parameter_list) {
	struct op *function_head = new_op();

	struct id *func = add_const_identifier(
	    name, ID_FUNC, variable_type);  // 向符号表添加类型为函数的符号

	cat_tac(function_head, NEW_TAC_1(TAC_LABEL, func));
	cat_tac(function_head, NEW_TAC_0(TAC_BEGIN));
	cat_op(function_head, parameter_list);

	function_head->code->id_1->function_info.param_list = parameter_list->code;

	return function_head;
}

// 处理函数参数列表的开端，加入标识符
struct op *process_parameter_list_head(struct var_type *variable_type,
                                       char *name) {
	struct op *parameter = new_op();

	struct id *var = add_identifier(name, ID_VAR, variable_type, NO_INDEX);
	cat_tac(parameter, NEW_TAC_1(TAC_PARAM, var));

	return parameter;
}

// 处理函数参数列表，加入标识符
struct op *process_parameter_list(struct op *param_list_pre,
                                  struct var_type *variable_type, char *name) {
	struct op *parameter_list = new_op();

	struct id *var = add_identifier(name, ID_VAR, variable_type, NO_INDEX);
	cat_op(parameter_list, param_list_pre);
	cat_tac(parameter_list, NEW_TAC_1(TAC_PARAM, var));

	return parameter_list;
}

struct op *param_args_type_casting(struct tac *func_param,
                                   struct op *args_list) {
	struct op *cast_args = new_op();

	// 正向遍历 args_list
	struct tac *arg = args_list->code;
	// 反向遍历 func_param
	struct tac *param = func_param;
	while (param && param->next->type == TAC_PARAM) {
		param = param->next;  // 移动到 func_param 的末尾
	}

	// hjj: 需要判断arg的类型是不是TAC_ARG，因为arg可能包含实参的计算表达式
	while (arg->type != TAC_ARG) {
		arg = arg->next;
	}

	while (arg && param->type == TAC_PARAM) {
		// 检查参数类型是否匹配
		if (!TYPE_CHECK(param->id_1, arg->id_1) &&
		    !REF_TO_CONTENT(param->id_1->variable_type,
		                    arg->id_1->variable_type)) {
			// 如果类型不匹配，进行类型转换
			struct op *cast_exp = type_casting(param->id_1, arg->id_1);

			cat_op(cast_args, cast_exp);  // 将转换后的代码拼接到 cast_args

			arg->id_1 = cast_exp->addr;  // 更新 args_list 中的参数
		}
		// 如果实参是引用类型
		else if (REF_TO_CONTENT(param->id_1->variable_type,
		                        arg->id_1->variable_type)) {
			struct id *t = new_temp(
			    new_var_type(arg->id_1->variable_type->data_type,
			                 arg->id_1->variable_type->pointer_level + 1, 0));

			cat_tac(cast_args, NEW_TAC_1(TAC_VAR, t));
			cat_tac(cast_args, NEW_TAC_2(TAC_REFER, t, arg->id_1));

			arg->id_1 = t;  // hjj: todo, type checking
		}
		arg = arg->next;      // 正向遍历 args_list
		param = param->prev;  // 反向遍历 func_param
	}
	// 实参太多了
	if (arg) {
		perror("too many args");
		printf("arg: %s\n", arg->id_1->name);
	}
	// 实参太少了
	if (param->type == TAC_PARAM) perror("too many params");
	cat_op(cast_args, args_list);

	return cast_args;
}

struct op *type_casting(struct id *id_remain, struct id *id_casting) {
	struct op *cast_exp = new_op();

	int type_target = id_remain->variable_type->data_type;
	int type_src = id_casting->variable_type->data_type;

	struct id *t = new_temp(
	    new_var_type(type_target, id_remain->variable_type->pointer_level,
	                 id_remain->variable_type->is_reference));  // 分配临时变量
	cast_exp->addr = t;

	char *casting_func = (char *)malloc(16);
	sprintf(casting_func, "__%s%s%s",
	        type_target == DATA_CHAR    ? "fixuns"
	        : type_target == DATA_INT   ? "fix"
	        : type_target == DATA_FLOAT ? "float"
	                                    : "",
	        type_src == DATA_CHAR    ? "unsi"
	        : type_src == DATA_INT   ? "si"
	        : type_src == DATA_FLOAT ? "sf"
	                                 : "",
	        type_target == DATA_CHAR    ? "si"
	        : type_target == DATA_INT   ? "si"
	        : type_target == DATA_FLOAT ? "sf"
	                                    : "");
	struct id *func;
	NEW_BUILT_IN_FUNC_ID(func, casting_func, type_target);

	cat_tac(cast_exp, NEW_TAC_1(TAC_VAR, t));
	cat_tac(cast_exp, NEW_TAC_1(TAC_ARG, id_casting));  // 生成 arg id_casting
	cat_tac(cast_exp, NEW_TAC_2(TAC_CALL, t, func));    // 生成 t1=call func
	return cast_exp;
}
