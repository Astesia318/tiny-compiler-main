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
struct op *process_array_identifier(struct op *array_op, int index);
struct op *process_identifier(char *name);

struct op *process_declaration(int data_type, struct op *exp_1);
struct op *process_variable_list_end(char *name, int index,int is_pointer);
struct op *process_variable_list(struct op *exp_1, char *name, int index,int is_pointer);
struct op *process_assign(struct op *leftval, struct op *exp_1);
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
struct op *process_function(struct op *exp_1, struct op *exp_2,
                            struct op *exp_3);
struct op *process_function_head(int data_type, char *name);
struct op *process_parameter_list_head(int data_type, char *name);
struct op *process_parameter_list(struct op *exp_1, int data_type, char *name);
#endif