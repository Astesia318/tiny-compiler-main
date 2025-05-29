#include "e_tac.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int scope;
char temp_buf[256];
struct id *id_global, *id_local;
static int temp_amount;
static int label_amount;
static int lc_amount;

struct id *struct_table;
int cur_member_offset;
// lyc:增加受检查id的type，以处理字符常量与变量名之间的误判/不加了
// check exist时为寻找标识符，not exist时需要判断同名是否同类型
static struct id *_find_identifier(const char *name, struct id **id_table,
                                   int check) {
	int has_finded = 0;
	struct id *id_wanted = NULL;
	struct id *cur = *id_table;

	while (cur) {
		if (cur->name && !strcmp(name, cur->name) &&
		    (check == CHECK_ID_NOT_EXIST ||
		     !ID_IS_CONST(cur))) {  // check exist时找到的必须是标识符
			has_finded = 1;
			id_wanted = cur;
			break;
		}
		cur = cur->next;
	}
	// lyc:当在global表中也找不到时才会not found
	if (!has_finded && check == CHECK_ID_EXIST && *id_table == id_global) {
		perror("identifier not found");
		printf("want name: %s\n", name);
#ifndef HJJ_DEBUG
		exit(0);  // lyc
#endif
	}
	return id_wanted;
}

static struct id **_choose_id_table(int table) {
	if (table == GLOBAL_TABLE) {
		return &id_global;
	} else {
		return &id_local;
	}
}

static struct id *_collide_identifier(const char *name, int id_type,
                                      struct var_type *variable_type) {
	if (ID_IS_GCONST(id_type, variable_type->data_type))
		return _find_identifier(name, _choose_id_table(GLOBAL_TABLE),
		                        CHECK_ID_NOT_EXIST);
	else
		return _find_identifier(name, _choose_id_table(scope),
		                        CHECK_ID_NOT_EXIST);
}

static struct id *_add_identifier(const char *name, int id_type,
                                  struct var_type *variable_type,
                                  struct id **id_table, int index) {
	struct id *id_wanted;

	struct id *id_collision = _collide_identifier(name, id_type, variable_type);
	if (id_collision) {  // 表内有同名id
		if (ID_IS_GCONST(
		        id_collision->id_type,
		        variable_type->data_type)) {  // 表中已有同名常量id，返回
			return id_collision;
		} else if (id_type != ID_NUM) {
			// 表中同名不是常量id，错误
			perror("identifier declared");
			printf("add name: %s\n", name);
			return NULL;
		}
		// 字符常量与标识符名冲突，正常添加XXX:现在会添加多个除GCONST外的相同常量
	}
	// 没有冲突，向表内添加
	MALLOC_AND_SET_ZERO(id_wanted, 1, struct id);
	char *id_name = (char *)malloc(sizeof(char) * strlen(name));
	strcpy(id_name, name);
	id_wanted->name = id_name;
	id_wanted->id_type = id_type;
	id_wanted->variable_type = variable_type;
	id_wanted->pointer_info.index = index;
	id_wanted->next = *id_table;
	*id_table = id_wanted;
	id_wanted->offset = -1; /* Unset address */
	if (ID_IS_GCONST(id_type, variable_type->data_type)) {
		id_wanted->label = lc_amount++;
	}

	return id_wanted;
}
// lyc:若在local表没找见，则需要从global表里找
struct id *find_identifier(const char *name) {
	struct id *res =
	    _find_identifier(name, _choose_id_table(scope), CHECK_ID_EXIST);
	if (res) {
		return res;
	} else {
		return _find_identifier(name, _choose_id_table(GLOBAL_TABLE),
		                        CHECK_ID_EXIST);
	}
}

struct id *find_func(const char *name) {
	return _find_identifier(name, _choose_id_table(GLOBAL_TABLE),
	                        CHECK_ID_EXIST);
}

struct id *check_struct_type(int struct_type) {
	struct id *cur_struct = struct_table;
	while (cur_struct) {
		if (cur_struct->struct_info.struct_type == struct_type) {
			return cur_struct;
		}
		cur_struct = cur_struct->next;
	}
	perror("no struct found");
#ifndef HJJ_DEBUG
	exit(0);
#endif
	return NULL;
}

struct id *check_struct_name(char *name) {
	struct id *cur_struct = struct_table;
	while (cur_struct) {
		if (!strcmp(cur_struct->name, name)) {
			return cur_struct;
		}
		cur_struct = cur_struct->next;
	}
	perror("no struct found");
#ifndef HJJ_DEBUG
	exit(0);
#endif
	return NULL;
}

struct member_def *find_member(struct id *instance, char *member_name) {
	struct id *struct_def =
	    check_struct_type(instance->variable_type->data_type);
	struct member_def *cur_member_def = struct_def->struct_info.definition_list;
	while (cur_member_def) {
		if (!strcmp(cur_member_def->name, member_name)) {
			return cur_member_def;
		}
		cur_member_def = cur_member_def->next_def;
	}
	perror("no member found");
#ifndef HJJ_DEBUG
	exit(0);
#endif
	return NULL;
}

struct id *add_identifier(const char *name, int id_type,
                          struct var_type *variable_type, int index) {
	// lyc:对于text float double类型常量将其放到全局表里
	// hjj: so as function and struct...原因是类型转换时会添加func,
	// 这个func应当到global table。 hjj: 不过到层次结构可能会改变
	if (ID_IS_GCONST(id_type, variable_type->data_type))
		return _add_identifier(name, id_type, variable_type,
		                       _choose_id_table(GLOBAL_TABLE), index);
	else
		return _add_identifier(name, id_type, variable_type,
		                       _choose_id_table(scope), index);
}

struct id *add_const_identifier(const char *name, int id_type,
                                struct var_type *variable_type) {
	return add_identifier(name, id_type, variable_type, NO_INDEX);
}

struct member_def *add_member_def_raw(char *name, int index) {
	struct member_def *new_def;

	MALLOC_AND_SET_ZERO(new_def, 1, struct member_def);
	char *member_name = (char *)malloc(sizeof(char) * strlen(name));
	strcpy(member_name, name);
	new_def->name = member_name;
	new_def->pointer_info.index = index;

	return new_def;
}

void init_tac() {
	scope = GLOBAL_TABLE;
	id_global = NULL;
	id_local = NULL;
	temp_amount = 1;
	lc_amount = 0;
	label_amount = 1;
	cur_member_offset = 0;
}

void reset_table(int direction) {
	struct id **table = _choose_id_table(scope);
	if (direction == INTO_LOCAL_TABLE) {
		scope = LOCAL_TABLE;
	} else if (direction == OUT_LOCAL_TABLE) {
		*table = NULL;
		scope = GLOBAL_TABLE;
	}
}

// void clear_table(int scope) {
// 	struct id **table = _choose_id_table(scope);
// 	struct id *head = *table;
// 	struct id *cur = head;
// 	while (head) {
// 		head = cur->next;
// 		free(cur);
// 		cur = head;
// 	}
// 	*table = NULL;
// }

void cat_tac(struct op *dest, struct tac *src) {
	struct tac *t = dest->code;
	if (t == NULL) {
		dest->code = src;
	} else {
		while (t->next != NULL) t = t->next;
		t->next = src;
		if (src) src->prev = t;
	}
}

// 和cat_tac不同之处在于，释放了作为struct op的src
void cat_op(struct op *dest, struct op *src) {
	cat_tac(dest, src->code);
	// free(src); // hjj: free会导致continue和break出错，无法捕捉需要parse的op
}

struct op *cat_list(struct op *exp_1, struct op *exp_2) {
	struct op *stat_list = new_op();

	cat_op(stat_list, exp_1);
	cat_op(stat_list, exp_2);

	return stat_list;
}

// 目前来看，并不需要复制再释放的操作，只需要把指针本身复制给dest
struct op *cpy_op(struct op *src) { return src; }

struct member_def *cat_def(struct member_def *list_1,
                           struct member_def *list_2) {
	struct member_def *cur_def = list_1;
	while (cur_def->next_def) {
		cur_def = cur_def->next_def;
	}
	cur_def->next_def = list_2;
	return list_1;
}

struct op *new_op() {
	struct op *nop;
	MALLOC_AND_SET_ZERO(nop, 1, struct op);
	return nop;
}

struct tac *new_tac(int type, struct id *id_1, struct id *id_2,
                    struct id *id_3) {
	struct tac *ntac = (struct tac *)malloc(sizeof(struct tac));

	ntac->type = type;
	ntac->next = NULL;
	ntac->prev = NULL;
	ntac->id_1 = id_1;
	ntac->id_2 = id_2;
	ntac->id_3 = id_3;

	return ntac;
}

struct id *new_temp(struct var_type *variable_type) {
	NAME_ALLOC(buf);
	sprintf(buf, "t%d", temp_amount++);  // hjj: todo, check collision
	// return add_identifier(buf, ID_TEMP, data_type, NO_INDEX,
	return add_identifier(buf, ID_VAR, variable_type, NO_INDEX);
}

struct id *new_label() {
	NAME_ALLOC(label);
	sprintf(label, ".L%d", label_amount++);
	return add_const_identifier(label, ID_LABEL,
	                            new_const_type(DATA_UNDEFINED, NOT_PTR));
}

struct block *new_block(struct id *l_begin, struct id *l_end) {
	struct block *nstack;
	MALLOC_AND_SET_ZERO(nstack, 1, struct block);
	nstack->label_begin = l_begin;
	nstack->label_end = l_end;
	return nstack;
}

struct var_type *new_var_type(int data_type, int pointer_level,
                              int is_reference) {
	struct var_type *new_type;
	MALLOC_AND_SET_ZERO(new_type, 1, struct var_type);
	new_type->data_type = data_type;
	new_type->pointer_level = pointer_level;
	new_type->is_reference = is_reference;
	return new_type;
}

struct var_type *new_const_type(int data_type, int pointer_level) {
	return new_var_type(data_type, pointer_level, 0);
}

const char *id_to_str(struct id *id) {
	if (id == NULL) return "NULL";

	switch (id->id_type) {
		case ID_NUM:
			// XXX:怎么释放
			if (id->variable_type->data_type == DATA_CHAR) {
				char *buf = (char *)malloc(16);  // 动态分配内存
				sprintf(buf, "\'%s\'", id->name);
				return buf;  // 返回动态分配的字符串
			}
		case ID_VAR:
		case ID_FUNC:
		case ID_LABEL:
		case ID_STRING:
			return id->name;

		default:
			perror("unknown TAC arg type");
			return "?";
	}
}

void output_struct(FILE *f, struct id *id_struct) {
	PRINT_1("struct %s\n", id_struct->name);
	struct member_def *cur_definition = id_struct->struct_info.definition_list;
	while (cur_definition) {
		if (cur_definition->pointer_info.index == NO_INDEX) {
			PRINT_2("member %s %s\n",
			        data_to_str(cur_definition->variable_type),
			        cur_definition->name);
		} else {
			PRINT_3("member %s %s[%d]\n",
			        data_to_str(cur_definition->variable_type),
			        cur_definition->name, cur_definition->pointer_info.index);
		}
		cur_definition = cur_definition->next_def;
	}
}

const char *data_to_str(struct var_type *variable_type) {
	int data_type = variable_type->data_type;
	char *buf = (char *)malloc(NAME_SIZE);

	if (data_type == DATA_UNDEFINED) {
		sprintf(buf, "NULL");
	} else if (data_type >= DATA_STRUCT_INIT) {
		sprintf(buf, "%s", check_struct_type(data_type)->name);
	} else {
		switch (data_type) {
			case DATA_INT:
				sprintf(buf, "%s", "int");
				break;
			case DATA_LONG:
				sprintf(buf, "%s", "long");
				break;
			case DATA_FLOAT:
				sprintf(buf, "%s", "float");
				break;
			case DATA_DOUBLE:
				sprintf(buf, "%s", "double");
				break;
			case DATA_CHAR:
				sprintf(buf, "%s", "char");
				break;
			default:
				perror("unknown data type");
				printf("id type: %d\n", data_type);
				sprintf(buf, "%s", "?");
				break;
		}
	}
	int cur_pointer = 0;
	while (cur_pointer != variable_type->pointer_level) {
		strcat(buf, "*");
		cur_pointer++;
	}
	if (variable_type->is_reference) {
		strcat(buf, " &");
	}
	return buf;
}

void output_tac(FILE *f, struct tac *code) {
	switch (code->type) {
		case TAC_PLUS:
			PRINT_3("%s = %s + %s\n", id_to_str(code->id_1),
			        id_to_str(code->id_2), id_to_str(code->id_3));
			break;

		case TAC_MINUS:
			PRINT_3("%s = %s - %s\n", id_to_str(code->id_1),
			        id_to_str(code->id_2), id_to_str(code->id_3));
			break;

		case TAC_MULTIPLY:
			PRINT_3("%s = %s * %s\n", id_to_str(code->id_1),
			        id_to_str(code->id_2), id_to_str(code->id_3));
			break;

		case TAC_DIVIDE:
			PRINT_3("%s = %s / %s\n", id_to_str(code->id_1),
			        id_to_str(code->id_2), id_to_str(code->id_3));
			break;

		case TAC_EQ:
			PRINT_3("%s = (%s == %s)\n", id_to_str(code->id_1),
			        id_to_str(code->id_2), id_to_str(code->id_3));
			break;

		case TAC_NE:
			PRINT_3("%s = (%s != %s)\n", id_to_str(code->id_1),
			        id_to_str(code->id_2), id_to_str(code->id_3));
			break;

		case TAC_LT:
			PRINT_3("%s = (%s < %s)\n", id_to_str(code->id_1),
			        id_to_str(code->id_2), id_to_str(code->id_3));
			break;

		case TAC_LE:
			PRINT_3("%s = (%s <= %s)\n", id_to_str(code->id_1),
			        id_to_str(code->id_2), id_to_str(code->id_3));
			break;

		case TAC_GT:
			PRINT_3("%s = (%s > %s)\n", id_to_str(code->id_1),
			        id_to_str(code->id_2), id_to_str(code->id_3));
			break;

		case TAC_GE:
			PRINT_3("%s = (%s >= %s)\n", id_to_str(code->id_1),
			        id_to_str(code->id_2), id_to_str(code->id_3));
			break;

		case TAC_REFER:
			PRINT_2("%s = &%s\n", id_to_str(code->id_1), id_to_str(code->id_2));
			break;

		case TAC_DEREFER_PUT:
			PRINT_2("*%s = %s\n", id_to_str(code->id_1), id_to_str(code->id_2));
			break;

		case TAC_DEREFER_GET:
			PRINT_2("%s = *%s\n", id_to_str(code->id_1), id_to_str(code->id_2));
			break;

		case TAC_VAR_REFER_INIT:
			PRINT_2("ref init %s = %s\n", id_to_str(code->id_1),
			        id_to_str(code->id_2));
			break;

		case TAC_ASSIGN:
			PRINT_2("%s = %s\n", id_to_str(code->id_1), id_to_str(code->id_2));
			break;

		case TAC_GOTO:
			PRINT_1("goto %s\n", code->id_1->name);
			break;

		case TAC_IFZ:
			PRINT_2("ifz %s goto %s\n", id_to_str(code->id_1),
			        code->id_2->name);
			break;

		case TAC_ARG:
			PRINT_2("arg %s %s\n", data_to_str(code->id_1->variable_type),
			        id_to_str(code->id_1));
			break;

		case TAC_PARAM:
			PRINT_2("param %s %s\n", data_to_str(code->id_1->variable_type),
			        id_to_str(code->id_1));
			break;

		case TAC_CALL:
			if (code->id_1 == NULL)
				PRINT_1("call %s\n", (char *)code->id_2);
			else
				PRINT_2("%s = call %s\n", id_to_str(code->id_1),
				        id_to_str(code->id_2));
			break;

		case TAC_INPUT:
			PRINT_1("input %s\n", id_to_str(code->id_1));
			break;

		case TAC_OUTPUT:
			PRINT_1("output %s\n", id_to_str(code->id_1));
			break;

		case TAC_RETURN:
			PRINT_1("return %s\n", id_to_str(code->id_1));
			break;

		case TAC_LABEL:
			PRINT_1("label %s\n", id_to_str(code->id_1));
			break;

		case TAC_VAR:
			if (code->id_1->pointer_info.index == NO_INDEX) {
				PRINT_2("var %s %s\n", data_to_str(code->id_1->variable_type),
				        id_to_str(code->id_1));
			} else {
				PRINT_3("var %s %s[%d]\n",
				        data_to_str(code->id_1->variable_type),
				        id_to_str(code->id_1), code->id_1->pointer_info.index);
			}
			break;

		case TAC_BEGIN:
			PRINT_0("begin\n");
			break;

		case TAC_END:
			PRINT_0("end\n\n");
			break;

		default:
			perror("unknown TAC opcode");
			break;
	}
#ifdef HJJ_DEBUG
	fflush(f);
#endif
	// code = code->next;
	// }
}

void source_to_tac(FILE *f, struct tac *code) {
	struct id *cur_struct = struct_table;
	while (cur_struct) {
		output_struct(f, cur_struct);
		cur_struct = cur_struct->struct_info.next_struct;
		PRINT_0("\n");
	}
	while (code) {
		output_tac(f, code);
		code = code->next;
	}
}

void input_str(FILE *f, const char *format, ...) {
	va_list args;
	va_start(args, format);
#ifdef HJJ_DEBUG
	vfprintf(stdout, format, args);
#else
	vfprintf(f, format, args);
#endif
	va_end(args);
}