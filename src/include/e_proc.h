#ifndef E_PROC_H
#define E_PROC_H
#include "e_tac.h"
struct op *type_casting(struct id *id_remain, struct id *id_casting);
struct op *param_args_type_casting(struct tac *func_param,
                                   struct op *args_list);
struct op *process_argument_list(struct op *exp_1);
struct op *process_expression_list_head(struct op *exp_1);
struct op *process_expression_list(struct op *exp_1, struct op *exp_2);
struct op *process_inc(struct op *id_op, int pos);
struct op *process_dec(struct op *id_op, int pos);
struct op *process_calculate(struct op *exp_1, struct op *exp_2, int cal);
struct op *process_int(int integer);
struct op *process_float(double floatnum);
struct op *process_char(char character);
struct op *process_add_identifier(char *name, struct arr_info *array_info);
struct op *process_identifier(char *name, struct arr_info *index_info);
struct op *process_instance_member(struct op *instance_op,
                                   struct member_ftch *member_fetch);
struct arr_info *process_nonconst_array_info(struct op *array_op);
struct var_type *process_struct_type(char *name, int pointer_level,
                                     int is_reference);
struct op *process_struct_head(char *name);
struct op *process_struct_definition(struct op *struct_head,
                                     struct member_def *definition_block);
struct member_def *process_definition(struct var_type *variable_type,
                                      struct member_def *member_list);
struct op *process_declaration(struct var_type *variable_type,
                               struct op *exp_1);
struct op *process_assign(struct op *leftval, struct op *exp_1);
struct op *process_derefer(struct op *id_op);
struct op *process_derefer_put(struct op *id_op);
struct op *process_derefer_get(struct op *id_op);
struct op *process_reference(struct op *id_op);
struct op *process_input(char *name);
struct op *process_output_variable(char *name);
struct op *process_output_text(char *name);
struct op *process_call(char *name, struct op *exp_1);
struct op *process_return(struct op *exp_1);
void block_initialize();
struct op *process_for(struct op *exp_1, struct op *exp_2, struct op *exp_3,
                       struct op *exp_4);
struct op *process_while(struct op *exp_1, struct op *exp_2);
struct op *process_break();
struct op *process_continue();
struct op *process_if_only(struct op *exp_1, struct op *exp_2);
struct op *process_if_else(struct op *exp_1, struct op *exp_2,
                           struct op *exp_3);

struct op *process_program(struct op *exp_1);
struct op *process_function(struct op *exp_1, struct op *exp_2);
struct op *process_function_head(struct var_type *variable_type, char *name,
                                 struct op *parameter_list);
struct op *process_parameter_list_head(struct var_type *variable_type,
                                       char *name);
struct op *process_parameter_list(struct op *exp_1,
                                  struct var_type *variable_type, char *name);
#endif