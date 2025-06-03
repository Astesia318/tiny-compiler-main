#include "e_custom.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_proc.h"
#include "e_tac.h"
#include "o_reg.h"

struct member_def *add_member_def_raw(char *name, struct arr_info *array_info) {
	struct member_def *new_def;

	MALLOC_AND_SET_ZERO(new_def, 1, struct member_def);
	char *member_name = (char *)malloc(sizeof(char) * strlen(name));
	strcpy(member_name, name);
	new_def->name = member_name;
	new_def->array_info = array_info;

	return new_def;
}
struct member_def *cat_def(struct member_def *list_1,
                           struct member_def *list_2) {
	struct member_def *cur_def = list_1;
	while (cur_def->next_def) {
		cur_def = cur_def->next_def;
	}
	cur_def->next_def = list_2;
	return list_1;
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

struct arr_info *increase_array_level(struct arr_info *array_info,
                                      struct arr_info *new_info,
                                      int const_or_not) {
	array_info->max_level += 1;
	int cur_level = array_info->max_level;
	if (const_or_not == IS_CONST_INDEX) {
		int size = new_info->const_index[0];
		array_info->const_or_not[cur_level] = IS_CONST_INDEX;
		array_info->const_index[cur_level] = size;
		array_info->array_offset[cur_level] *= size;
	} else {
		struct op *exp = new_info->nonconst_index[0];
		array_info->const_or_not[cur_level] = NOT_CONST_INDEX;
		array_info->nonconst_index[cur_level] = exp;
		array_info->in_declaration_or_not = NOT_DECLARATION;
	}
	return array_info;
}

struct arr_info *new_array_info(struct op *first_exp, int const_or_not) {
	struct arr_info *new_info;
	MALLOC_AND_SET_ZERO(new_info, 1, struct arr_info);
	new_info->max_level = 0;
	new_info->in_declaration_or_not = MAYBE_DECLARATION;
	if (const_or_not == IS_CONST_INDEX) {
		int first_size = first_exp->addr->number_info.num.num_int;
		new_info->const_or_not[0] = IS_CONST_INDEX;
		new_info->const_index[0] = first_size;
		new_info->array_offset[0] = first_size;
	} else {
		new_info->const_or_not[0] = NOT_CONST_INDEX;
		new_info->nonconst_index[0] = first_exp;
		new_info->in_declaration_or_not = NOT_DECLARATION;
	}
	return new_info;
}

struct member_ftch *new_member_fetch(int is_pointer_fetch, char *name,
                                     struct arr_info *index_info) {
	struct member_ftch *new_fetch;
	MALLOC_AND_SET_ZERO(new_fetch, 1, struct member_ftch);
	char *member_name = (char *)malloc(sizeof(char) * strlen(name));
	strcpy(member_name, name);
	new_fetch->name = member_name;
	new_fetch->is_pointer_fetch = is_pointer_fetch;
	new_fetch->index_info = index_info;
	return new_fetch;
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

struct member_def *find_member(struct id *instance, const char *member_name) {
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
// 处理数组标识符
struct op *process_array_identifier(struct op *array_op,
                                           struct arr_info *index_info) {
	struct op *id_op = new_op();

	struct id *array = array_op->addr;
	struct arr_info *array_info = array->array_info;
	if (!array->variable_type->pointer_level) {
		perror("id is not from an array");
#ifndef HJJ_DEBUG
		exit(0);
#endif
	}

	if (array_info->max_level < index_info->max_level) {
		perror("index level too high");
#ifndef HJJ_DEBUG
		exit(0);
#endif
	}

	struct id *t_1 =
	    new_temp(new_var_type(array->variable_type->data_type,
	                          array->variable_type->pointer_level, 0));
	int deref_count = 0;

	cat_op(id_op, array_op);
	cat_tac(id_op, NEW_TAC_1(TAC_VAR, t_1));
	cat_tac(id_op, NEW_TAC_2(TAC_ASSIGN, t_1, array));
	for (int cur_level = 0; cur_level <= index_info->max_level; cur_level++) {
		if (index_info->const_or_not[cur_level] == IS_CONST_INDEX) {
			if (array_info->const_index[cur_level] <
			    index_info->const_index[cur_level] + 1) {
				perror("index size too large");
#ifndef HJJ_DEBUG
				exit(0);
#endif
			}

			struct id *t_2 =
			    new_temp(new_var_type(array->variable_type->data_type,
			                          array->variable_type->pointer_level, 0));
			struct op *num_op =
			    process_int(index_info->const_index[cur_level] *
			                array_info->array_offset[array_info->max_level] /
			                array_info->array_offset[cur_level] *
			                DATA_SIZE(array->variable_type->data_type));

			cat_tac(id_op, NEW_TAC_1(TAC_VAR, t_2));
			cat_tac(id_op, NEW_TAC_3(TAC_PLUS, t_2, t_1, num_op->addr));

			t_1 = t_2;
			deref_count++;
		} else {
			struct op *exp_op = index_info->nonconst_index[cur_level];
			struct id *index = exp_op->addr;
			struct id *t_2 =
			    new_temp(new_var_type(array->variable_type->data_type,
			                          array->variable_type->pointer_level, 0));
			struct id *t_inc =
			    new_temp(new_var_type(index->variable_type->data_type,
			                          index->variable_type->pointer_level, 0));

			cat_tac(id_op, NEW_TAC_1(TAC_VAR, t_2));
			cat_tac(id_op, NEW_TAC_1(TAC_VAR, t_inc));
			cat_op(id_op, exp_op);
			cat_tac(id_op,
			        NEW_TAC_3(
			            TAC_MULTIPLY, t_inc, index,
			            process_int(DATA_SIZE(array->variable_type->data_type))
			                ->addr));
			cat_tac(id_op, NEW_TAC_3(TAC_PLUS, t_2, t_1, t_inc));

			t_1 = t_2;
			deref_count++;
		}
	}
	id_op->deref_count = deref_count;
	id_op->addr = t_1;

	return id_op;
}
// lyc:
struct op *process_instance_member(struct op *instance_op,
                                   struct member_ftch *member_fetch) {
	struct id *instance = instance_op->addr;
	struct member_def *member = find_member(instance, member_fetch->name);

	int member_pointer_level = member->variable_type->pointer_level;
	int instance_pointer_level = instance->variable_type->pointer_level;
	int member_data_type = member->variable_type->data_type;
	int instance_data_type = instance->variable_type->data_type;
	int deref_count = instance_op->deref_count;
	if (member_fetch->is_pointer_fetch && instance_pointer_level == 0) {
		perror("Can't apply '->' to a non-pointer");
	}
	int is_pointer_fetch =
	    member_fetch->is_pointer_fetch || instance_pointer_level;
	int is_ref = instance->variable_type->is_reference;
	struct arr_info *member_index = member_fetch->index_info;

	struct id *t_member_ptr = new_temp(new_var_type(
	    member_data_type, member_pointer_level + (member_index == NULL),
	    NOT_REF));  // member_addr
	struct id *t_instance_ptr = new_temp(new_var_type(
	    instance_data_type, instance_pointer_level + !is_pointer_fetch,
	    NOT_REF));  // base_addr
	cat_tac(instance_op, NEW_TAC_1(TAC_VAR, t_member_ptr));
	cat_tac(instance_op, NEW_TAC_1(TAC_VAR, t_instance_ptr));
	cat_tac(instance_op,
	        NEW_TAC_2(is_pointer_fetch || is_ref ? TAC_ASSIGN : TAC_REFER,
	                  t_instance_ptr, instance));
	cat_tac(instance_op, NEW_TAC_3(TAC_PLUS, t_member_ptr, t_instance_ptr,
	                               process_int(member->member_offset)->addr));
	instance_op->deref_count = 1;
	instance_op->addr = t_member_ptr;
	// lyc:玄学操作，只是为了正常调用array处理函数↓
	t_member_ptr->array_info = member->array_info;
	if (member_index) {
		instance_op = process_array_identifier(instance_op, member_index);
		t_member_ptr->array_info = NO_INDEX;
	}
	return instance_op;
}
struct var_type *process_struct_type(char *name, int pointer_level,
                                     int is_reference) {
	return new_var_type(check_struct_name(name)->struct_info.struct_type,
	                    pointer_level, is_reference);
}
struct op *process_struct_head(char *name) {
	struct op *struct_op = new_op();
	static int cur_struct_type = DATA_STRUCT_INIT;

	struct id *new_struct = add_identifier(name, ID_STRUCT, NO_TYPE, NO_INDEX);
	new_struct->struct_info.next_struct = struct_table;
	new_struct->struct_info.struct_type = cur_struct_type++;
	struct_table = new_struct;
	cur_member_offset = 0;
	struct_op->addr = new_struct;
	return struct_op;
}

struct op *process_struct_definition(struct op *struct_head,
                                     struct member_def *definition_block) {
	struct id *new_struct = struct_head->addr;
	new_struct->struct_info.definition_list = definition_block;
	new_struct->struct_info.struct_offset = cur_member_offset;

	return struct_head;
}

struct member_def *process_definition(struct var_type *variable_type,
                                      struct member_def *definition_list) {
	struct member_def *cur_definition = definition_list;
	while (cur_definition) {
		cur_definition->variable_type = variable_type;
		if (cur_definition->array_info != NO_INDEX) {
			cur_definition->variable_type->pointer_level =
			    cur_definition->array_info->max_level + 1;
		}
		cur_definition->member_offset = cur_member_offset;
		cur_member_offset +=
		    TYPE_SIZE(variable_type, cur_definition->array_info);

		cur_definition = cur_definition->next_def;
	}
	return definition_list;
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

void output_struct(FILE *f, struct id *id_struct) {
	PRINT_1("struct %s\n", id_struct->name);
	struct member_def *cur_definition = id_struct->struct_info.definition_list;
	while (cur_definition) {
		if (cur_definition->array_info == NO_INDEX) {
			PRINT_2("member %s %s",
			        data_to_str(cur_definition->variable_type,
			                    cur_definition->array_info),
			        cur_definition->name);
		} else {
			PRINT_2("member %s %s",
			        data_to_str(cur_definition->variable_type,
			                    cur_definition->array_info),
			        cur_definition->name);
			for (int cur_level = 0;
			     cur_level <= cur_definition->array_info->max_level;
			     cur_level++) {
				PRINT_1("[%d]",
				        cur_definition->array_info->const_index[cur_level]);
			}
		}
		cur_definition = cur_definition->next_def;
		PRINT_0("\n");
	}
}
