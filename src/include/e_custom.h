#ifndef E_CUSTOM_H
#define E_CUSTOM_H
#include "e_proc.h"
#include "e_tac.h"

extern struct id *struct_table;
extern int cur_member_offset;

struct member_def *add_member_def_raw(char *name, struct arr_info *array_info);
struct member_def *cat_def(struct member_def *list_1,
                           struct member_def *list_2);
struct var_type *new_var_type(int data_type, int pointer_level,
                              int is_reference);

struct var_type *new_const_type(int data_type, int pointer_level);

struct arr_info *increase_array_level(struct arr_info *array_info,
                                      struct arr_info *new_info,
                                      int const_or_not);

struct arr_info *new_array_info(struct op *first_exp, int const_or_not);

struct member_ftch *new_member_fetch(int is_pointer_fetch, char *name,
                                     struct arr_info *index_info);

struct id *check_struct_name(char *name);

struct member_def *find_member(struct id *instance, const char *member_name);

struct op *process_array_identifier(struct op *array_op,
                                    struct arr_info *index_info);

struct op *process_instance_member(struct op *instance_op,
                                   struct member_ftch *member_fetch);
struct var_type *process_struct_type(char *name, int pointer_level,
                                     int is_reference);
struct op *process_struct_head(char *name);

struct op *process_struct_definition(struct op *struct_head,
                                     struct member_def *definition_block);

struct member_def *process_definition(struct var_type *variable_type,
                                      struct member_def *definition_list);
struct id *check_struct_type(int struct_type);
void output_struct(FILE *f, struct id *id_struct);

#endif