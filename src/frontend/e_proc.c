#include "e_proc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_tac.h"

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

	struct id *exp_l_addr = exp_l->addr;
	struct id *exp_r_addr = exp_r->addr;

	struct id *t = new_temp(exp_l->addr->data_type);  // 分配临时变量
	exp->addr = t;
	cat_tac(exp, NEW_TAC_1(TAC_VAR, t));
	cat_op(exp, exp_l);  // 拼接exp和exp_l的code
	cat_op(exp, exp_r);  // 拼接exp和exp_r的code

	if (DATA_IS_REF(exp_l_addr->data_type)) {
		struct id *tl = new_temp(REF_TO_CONTENT(exp_l_addr->data_type));

		cat_tac(exp, NEW_TAC_1(TAC_VAR, tl));
		cat_tac(exp, NEW_TAC_2(TAC_DEREFER_GET, tl, exp_l_addr));

		exp_l_addr = tl;
		t->data_type = tl->data_type;
	}
	if (DATA_IS_REF(exp_r_addr->data_type)) {
		struct id *tr = new_temp(REF_TO_CONTENT(exp_r_addr->data_type));

		cat_tac(exp, NEW_TAC_1(TAC_VAR, tr));
		cat_tac(exp, NEW_TAC_2(TAC_DEREFER_GET, tr, exp_r_addr));

		exp_r_addr = tr;
	}

	if (exp_l_addr->data_type == DATA_FLOAT ||
	    exp_r_addr->data_type == DATA_FLOAT) {
		// 对于浮点数的运算，生成调用内部函数的三地址码

		if ((TYPE_CHECK(exp_l_addr, exp_r_addr)) == 0) {
			if (exp_l_addr->data_type == DATA_FLOAT) {
				struct op *cast_exp = type_casting(exp_l_addr, exp_r_addr);
				exp_r_addr = cast_exp->addr;
				cat_op(exp, cast_exp);
			} else if (exp_r_addr->data_type == DATA_FLOAT) {
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

			struct id *const_0 =
			    add_identifier("0", ID_NUM, DATA_INT, NO_INDEX);
			const_0->num.num_int = 0;
			struct id *const_1 =
			    add_identifier("1.0", ID_NUM, DATA_FLOAT, NO_INDEX);
			const_1->num.num_float = 1.0;

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

// 处理形如"a=-b"的表达式
struct op *process_negative(struct op *exp) {
	struct op *neg_exp = new_op();

	struct id *t = new_temp(exp->addr->data_type);
	struct id *exp_addr = exp->addr;
	neg_exp->addr = t;

	cat_op(neg_exp, exp);
	cat_tac(neg_exp, NEW_TAC_2(TAC_NEGATIVE, t, exp_addr));

	return neg_exp;
}

// 分配整数型数字符号
struct op *process_int(int integer) {
	struct op *int_exp = new_op();

	BUF_ALLOC(buf);  // 声明一个char数组变量buf，储存符号名
	sprintf(buf, "%d", integer);
	struct id *var = add_identifier(buf, ID_NUM, DATA_INT,
	                                NO_INDEX);  // 向符号表添加以buf为名的符号
	var->num.num_int = integer;
	int_exp->addr = var;

	return int_exp;
}

// 分配浮点型数字符号
struct op *process_float(double floatnum) {
	struct op *float_exp = new_op();

	BUF_ALLOC(buf);
	sprintf(buf, "%f", floatnum);
	struct id *var = add_identifier(buf, ID_NUM, DATA_FLOAT, NO_INDEX);
	var->num.num_float = floatnum;
	float_exp->addr = var;

	return float_exp;
}

struct op *process_char(char character) {
	struct op *char_exp = new_op();

	BUF_ALLOC(buf);
	sprintf(buf, "%c", character);
	struct id *var = add_identifier(buf, ID_NUM, DATA_CHAR, NO_INDEX);
	var->num.num_char = character;
	char_exp->addr = var;

	return char_exp;
}

// 处理右值
struct op *process_rightval(char *name) {
	struct op *id_exp = new_op();

	struct id *var = find_identifier(name);
	id_exp->addr = var;
	if (var->reference_stat != NULL) {
		cat_op(id_exp, var->reference_stat);
		var->reference_stat = NULL;
	}
	if (var->dereference_stat != NULL) {
		cat_op(id_exp, var->dereference_stat);
		var->dereference_stat = NULL;
	}

	return id_exp;
}

// 处理形如"a++"和"++a"的表达式
struct op *process_inc(char *name, int pos) {
	struct op *inc_exp = new_op();

	struct id *var = find_identifier(name);
	struct id *t = new_temp(var->data_type);
	struct id *num = add_identifier("1", ID_NUM, NO_DATA, NO_INDEX);
	inc_exp->addr = t;

	if (var->data_type != DATA_INT) {
		perror("wrong type");
	}
	cat_tac(inc_exp, NEW_TAC_1(TAC_VAR, t));
	if (pos == INC_HEAD) {
		cat_tac(inc_exp, NEW_TAC_3(TAC_PLUS, t, var, num));
		cat_tac(inc_exp, NEW_TAC_2(TAC_ASSIGN, var, t));
	} else {
		cat_tac(inc_exp, NEW_TAC_2(TAC_ASSIGN, t, var));
		cat_tac(inc_exp, NEW_TAC_3(TAC_PLUS, var, t, num));
	}

	return inc_exp;
}

// 处理形如"a--"和"--a"的表达式
struct op *process_dec(char *name, int pos) {
	struct op *dec_exp = new_op();

	struct id *var = find_identifier(name);
	struct id *t = new_temp(var->data_type);
	struct id *num = add_identifier("1", ID_NUM, NO_DATA, NO_INDEX);
	dec_exp->addr = t;

	if (var->data_type != DATA_INT) {
		perror("wrong type");
	}
	cat_tac(dec_exp, NEW_TAC_1(TAC_VAR, t));
	if (pos == INC_HEAD) {
		cat_tac(dec_exp, NEW_TAC_3(TAC_MINUS, t, var, num));
		cat_tac(dec_exp, NEW_TAC_2(TAC_ASSIGN, var, t));
	} else {
		cat_tac(dec_exp, NEW_TAC_2(TAC_ASSIGN, t, var));
		cat_tac(dec_exp, NEW_TAC_3(TAC_MINUS, var, t, num));
	}

	return dec_exp;
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

/**************************************/
/************* statement **************/
/**************************************/
// 处理变量声明，为process_variable函数声明的变量加上类型
struct op *process_declaration(int data_type, struct op *declaration_exp) {
	struct op *declaration = new_op();

	struct tac *head = declaration_exp->code;
	while (
	    head) {  // 逐个修改包含已声明变量的declaration_exp表达式所含变量的类型
		if (head->id_1->index == NO_INDEX) {
			head->id_1->data_type = data_type;
		} else {
			head->id_1->data_type = CONTENT_TO_POINTER(data_type);
		}
		head = head->next;
	}
	cat_op(declaration, declaration_exp);

	return declaration;
}

// 处理变量声明的末尾，向符号表加入尚未初始化类型的变量
struct op *process_variable_list_end(char *name, int index) {
	struct op *variable = new_op();

	struct id *var = add_identifier(name, ID_VAR, NO_TYPE, index);

	cat_tac(variable, NEW_TAC_1(TAC_VAR, var));

	return variable;
}

// 处理变量声明，向符号表加入尚未初始化类型的变量
struct op *process_variable_list(struct op *var_list_pre, char *name,
                                 int index) {
	struct op *variable_list = new_op();

	struct id *var = add_identifier(name, ID_VAR, NO_TYPE, index);

	cat_op(variable_list, var_list_pre);
	cat_tac(variable_list, NEW_TAC_1(TAC_VAR, var));

	return variable_list;
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
	struct id *t = new_temp(func->data_type);
	call_stat->addr = t;

	struct tac *arg = arg_list->code;
	struct op *cast_arg_list =
	    param_args_type_casting(func->func_param, arg_list);

	if (func->data_type != DATA_VOID) {
		cat_tac(call_stat, NEW_TAC_1(TAC_VAR, t));
	}
	cat_op(call_stat, cast_arg_list);
	if (func->data_type != DATA_VOID) {
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

	struct id *str = add_identifier(string, ID_STRING, NO_DATA, NO_INDEX);

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
struct op *process_assign(char *name, struct op *exp) {
	struct op *assign_stat = new_op();

	struct id *var = find_identifier(name);
	struct id *exp_temp = exp->addr;
	struct id *t;  // used in ref
	assign_stat->addr = exp_temp;

	cat_op(assign_stat, exp);
	if ((TYPE_CHECK(var, exp_temp)) == 0 &&
	    POINTER_TO_CONTENT(var->data_type) != exp_temp->data_type &&
	    REF_TO_CONTENT(var->data_type) != exp_temp->data_type) {
		struct op *cast_exp = type_casting(var, exp_temp);
		exp_temp = cast_exp->addr;
		cat_op(assign_stat, cast_exp);
	}
	if (var->dereference_stat != NULL) {
		if (!DATA_IS_POINTER(
		        var->data_type)) {  // hjj: 类型检查，以后可能要改掉
			perror("prohibit dereferencing a nonpointer");
			printf("var name: %s\n",var->name);
			printf("var type: %d\n",var->data_type);
#ifndef HJJ_DEBUG
			exit(0);
#endif
		}
		cat_tac(var->dereference_stat,
		        NEW_TAC_2(TAC_DEREFER_PUT, var->dereference_stat->code->id_1,
		                  exp_temp));
		cat_op(assign_stat, var->dereference_stat);
		var->dereference_stat = NULL;
	} else if (DATA_IS_REF(var->data_type)) {
		cat_tac(assign_stat, NEW_TAC_2(TAC_DEREFER_PUT, var, exp_temp));
	} else {
		cat_tac(assign_stat, NEW_TAC_2(TAC_ASSIGN, var, exp_temp));
	}

	return assign_stat;
}

// 处理被解引用并向内赋值的指针
const char *process_derefer_put(char *name) {
	struct op *content_stat = new_op();

	struct id *var = find_identifier(name);
	if (!DATA_IS_POINTER(var->data_type)) {
		perror("data is not a pointer");
		printf("data name: %s\n", var->name);
		printf("data type: %d\n", var->data_type);
#ifndef HJJ_DEBUG
		exit(0);
#endif
	}

	struct id *t = new_temp(var->data_type);
	cat_tac(content_stat, NEW_TAC_1(TAC_VAR, t));
	cat_tac(content_stat, NEW_TAC_2(TAC_ASSIGN, t, var));
	t->dereference_stat = content_stat;

	return t->name;
}

// 处理被解引用并从内取值的指针
const char *process_derefer_get(char *name) {
	struct op *content_stat = new_op();

	struct id *var = find_identifier(name);
	if (!DATA_IS_POINTER(var->data_type)) {
		perror("data is not a pointer");
#ifndef HJJ_DEBUG
		exit(0);
#endif
	}

	struct id *t = new_temp(POINTER_TO_CONTENT(var->data_type));
	cat_tac(content_stat, NEW_TAC_1(TAC_VAR, t));
	cat_tac(content_stat, NEW_TAC_2(TAC_DEREFER_GET, t, var));
	t->dereference_stat = content_stat;

	return t->name;
}

// 处理引用的变量
const char *process_reference(char *name) {
	struct op *pointer_stat = new_op();

	struct id *var = find_identifier(name);
	if (DATA_IS_POINTER(var->data_type)) {
		perror("data is a pointer");
#ifndef HJJ_DEBUG
		exit(0);
#endif
	}

	struct id *t = new_temp(CONTENT_TO_POINTER(var->data_type));
	cat_tac(pointer_stat, NEW_TAC_1(TAC_VAR, t));
	cat_tac(pointer_stat, NEW_TAC_2(TAC_REFER, t, var));
	t->reference_stat = pointer_stat;

	return t->name;
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
struct op *process_function(struct op *function_head, struct op *parameter_list,
                            struct op *block) {
	struct op *function = new_op();

	function_head->code->id_1->func_param = parameter_list->code;

	cat_op(function, function_head);
	cat_op(function, parameter_list);
	cat_op(function, block);
	cat_tac(function, NEW_TAC_0(TAC_END));

	return function;
}

// 处理函数头
struct op *process_function_head(int data_type, char *name) {
	struct op *function_head = new_op();

	struct id *func = add_identifier(name, ID_FUNC, data_type,
	                                 NO_INDEX);  // 向符号表添加类型为函数的符号
	cat_tac(function_head, NEW_TAC_1(TAC_LABEL, func));
	cat_tac(function_head, NEW_TAC_0(TAC_BEGIN));

	return function_head;
}

// 处理函数参数列表的开端，加入标识符
struct op *process_parameter_list_head(int data_type, char *name) {
	struct op *parameter = new_op();

	// if(DATA_IS_REF(data_type)) data_type = REF_TO_POINTER(data_type);
	struct id *var = add_identifier(name, ID_VAR, data_type, NO_INDEX);
	cat_tac(parameter, NEW_TAC_1(TAC_PARAM, var));

	return parameter;
}

// 处理函数参数列表，加入标识符
struct op *process_parameter_list(struct op *param_list_pre, int data_type,
                                  char *name) {
	struct op *parameter_list = new_op();

	// if(DATA_IS_REF(data_type)) data_type = REF_TO_POINTER(data_type);
	struct id *var = add_identifier(name, ID_VAR, data_type, NO_INDEX);
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
		if (TYPE_CHECK(param->id_1, arg->id_1) == 0 &&
		    REF_TO_CONTENT(param->id_1->data_type) != arg->id_1->data_type) {
			// 如果类型不匹配，进行类型转换
			struct op *cast_exp = type_casting(param->id_1, arg->id_1);

			cat_op(cast_args, cast_exp);  // 将转换后的代码拼接到 cast_args

			arg->id_1 = cast_exp->addr;  // 更新 args_list 中的参数
		}
		// 如果实参是引用类型
		else if (REF_TO_CONTENT(param->id_1->data_type) ==
		         arg->id_1->data_type) {
			struct id *t = new_temp(CONTENT_TO_POINTER(arg->id_1->data_type));

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

	int type_target = id_remain->data_type;
	int type_src = id_casting->data_type;

	struct id *t = new_temp(type_target);  // 分配临时变量
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