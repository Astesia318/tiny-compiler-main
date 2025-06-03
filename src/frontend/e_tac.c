#include "e_tac.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_internal.h"
#include "e_custom.h"

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
		     !ID_IS_CONST(cur->id_type))) {  // check exist时找到的必须是标识符
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
	if (ID_IS_GLOBAL(id_type))
		return _find_identifier(name, _choose_id_table(GLOBAL_TABLE),
		                        CHECK_ID_NOT_EXIST);
	else
		return _find_identifier(name, _choose_id_table(scope),
		                        CHECK_ID_NOT_EXIST);
}

static struct id *_add_identifier(const char *name, int id_type,
                                  struct var_type *variable_type,
                                  struct id **id_table,
                                  struct arr_info *array_info) {
	struct id *new_id;

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
	MALLOC_AND_SET_ZERO(new_id, 1, struct id);
	char *id_name = (char *)malloc(sizeof(char) * strlen(name));
	strcpy(id_name, name);
	new_id->name = id_name;
	new_id->id_type = id_type;
	new_id->variable_type = variable_type;
	new_id->scope = scope;
	new_id->array_info = array_info;
	new_id->next = *id_table;
	*id_table = new_id;
	new_id->offset = -1; /* Unset address */
	if (ID_IS_GCONST(id_type, variable_type->data_type)) {
		new_id->label = lc_amount++;
	}

	return new_id;
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


struct id *add_identifier(const char *name, int id_type,
                          struct var_type *variable_type,
                          struct arr_info *array_info) {
	// lyc:对于text float double类型常量将其放到全局表里
	// hjj: so as function and struct...原因是类型转换时会添加func,
	// 这个func应当到global table。 hjj: 不过到层次结构可能会改变
	// lyc: 说得对，使用GLOBAL代替了，GCONST改回去了
	if (ID_IS_GLOBAL(id_type))
		return _add_identifier(name, id_type, variable_type,
		                       _choose_id_table(GLOBAL_TABLE), array_info);
	else
		return _add_identifier(name, id_type, variable_type,
		                       _choose_id_table(scope), array_info);
}

struct id *add_const_identifier(const char *name, int id_type,
                                struct var_type *variable_type) {
	return add_identifier(name, id_type, variable_type, NO_INDEX);
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


struct op *new_op() {
	struct op *nop;
	MALLOC_AND_SET_ZERO(nop, 1, struct op);
	return nop;
}

struct tac *new_tac(int type, struct id *id_1, struct id *id_2,
                    struct id *id_3) {
	struct tac *ntac;
	MALLOC_AND_SET_ZERO(ntac, 1, struct tac);

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
	return add_identifier(buf, ID_VAR, variable_type, NO_INDEX);
}

struct id *new_label() {
	NAME_ALLOC(label);
	sprintf(label, ".L%d", label_amount++);
	return add_const_identifier(label, ID_LABEL,
	                            new_const_type(DATA_UNDEFINED, 0));
}

struct block *new_block(struct id *l_begin, struct id *l_end) {
	struct block *nstack;
	MALLOC_AND_SET_ZERO(nstack, 1, struct block);
	nstack->label_begin = l_begin;
	nstack->label_end = l_end;
	return nstack;
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


const char *data_to_str(struct var_type *variable_type,
                        struct arr_info *array_info) {
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
	// 要考虑指针数组，数组类型要少输出一次*
	int cur_pointer = (array_info != NO_INDEX);
	while (cur_pointer < variable_type->pointer_level) {
		strcat(buf, "*");
		cur_pointer++;
	}

	if (variable_type->is_reference) {
		strcat(buf, "&");
	}
	return buf;
}

void output_tac(FILE *f, struct tac *code) {
	struct id *id_1 = code->id_1;
	struct id *id_2 = code->id_2;
	struct id *id_3 = code->id_3;
	switch (code->type) {
		case TAC_PLUS:
			PRINT_3("%s = %s + %s\n", id_to_str(id_1), id_to_str(id_2),
			        id_to_str(id_3));
			break;

		case TAC_MINUS:
			PRINT_3("%s = %s - %s\n", id_to_str(id_1), id_to_str(id_2),
			        id_to_str(id_3));
			break;

		case TAC_MULTIPLY:
			PRINT_3("%s = %s * %s\n", id_to_str(id_1), id_to_str(id_2),
			        id_to_str(id_3));
			break;

		case TAC_DIVIDE:
			PRINT_3("%s = %s / %s\n", id_to_str(id_1), id_to_str(id_2),
			        id_to_str(id_3));
			break;

		case TAC_EQ:
			PRINT_3("%s = (%s == %s)\n", id_to_str(id_1), id_to_str(id_2),
			        id_to_str(id_3));
			break;

		case TAC_NE:
			PRINT_3("%s = (%s != %s)\n", id_to_str(id_1), id_to_str(id_2),
			        id_to_str(id_3));
			break;

		case TAC_LT:
			PRINT_3("%s = (%s < %s)\n", id_to_str(id_1), id_to_str(id_2),
			        id_to_str(id_3));
			break;

		case TAC_LE:
			PRINT_3("%s = (%s <= %s)\n", id_to_str(id_1), id_to_str(id_2),
			        id_to_str(id_3));
			break;

		case TAC_GT:
			PRINT_3("%s = (%s > %s)\n", id_to_str(id_1), id_to_str(id_2),
			        id_to_str(id_3));
			break;

		case TAC_GE:
			PRINT_3("%s = (%s >= %s)\n", id_to_str(id_1), id_to_str(id_2),
			        id_to_str(id_3));
			break;

		case TAC_REFER:
			PRINT_2("%s = &%s\n", id_to_str(id_1), id_to_str(id_2));
			break;

		case TAC_DEREFER_PUT:
			PRINT_2("*%s = %s\n", id_to_str(id_1), id_to_str(id_2));
			break;

		case TAC_DEREFER_GET:
			PRINT_2("%s = *%s\n", id_to_str(id_1), id_to_str(id_2));
			break;

		case TAC_VAR_REFER_INIT:
			PRINT_2("ref init %s = %s\n", id_to_str(id_1), id_to_str(id_2));
			break;

		case TAC_ASSIGN:
			PRINT_2("%s = %s\n", id_to_str(id_1), id_to_str(id_2));
			break;

		case TAC_GOTO:
			PRINT_1("goto %s\n", id_1->name);
			break;

		case TAC_IFZ:
			PRINT_2("ifz %s goto %s\n", id_to_str(id_1), id_2->name);
			break;

		case TAC_ARG:
			PRINT_2("arg %s %s\n",
			        data_to_str(id_1->variable_type, id_1->array_info),
			        id_to_str(id_1));
			break;

		case TAC_PARAM:
			PRINT_2("param %s %s\n",
			        data_to_str(id_1->variable_type, id_1->array_info),
			        id_to_str(id_1));
			break;

		case TAC_CALL:
			if (id_1 == NULL)
				PRINT_1("call %s\n", id_2->name);
			else
				PRINT_2("%s = call %s\n", id_to_str(id_1), id_to_str(id_2));
			break;

		case TAC_INPUT:
			PRINT_1("input %s\n", id_to_str(id_1));
			break;

		case TAC_OUTPUT:
			PRINT_1("output %s\n", id_to_str(id_1));
			break;

		case TAC_RETURN:
			PRINT_1("return %s\n", id_to_str(id_1));
			break;

		case TAC_LABEL:
			PRINT_1("label %s\n", id_to_str(id_1));
			break;

		case TAC_VAR:
			if (id_1->array_info == NO_INDEX) {
				PRINT_2("var %s %s\n",
				        data_to_str(id_1->variable_type, id_1->array_info),
				        id_to_str(id_1));
			} else {
				PRINT_2("var %s %s",
				        data_to_str(id_1->variable_type, id_1->array_info),
				        id_to_str(id_1));
				for (int cur_level = 0;
				     cur_level <= id_1->array_info->max_level; cur_level++) {
					PRINT_1("[%d]", id_1->array_info->const_index[cur_level]);
				}
				PRINT_0("\n");
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
#ifdef HJJ_TERMINAL
	vfprintf(stdout, format, args);
#else
	vfprintf(f, format, args);
#endif
	va_end(args);
}