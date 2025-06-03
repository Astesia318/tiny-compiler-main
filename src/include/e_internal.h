#ifndef E_INTERNAL_H
#define E_INTERNAL_H
#include "e_proc.h"
#include "e_tac.h"

struct op *__deref(struct op *exp);

struct op *process_derefer(struct op *id_op);

struct op *process_derefer_put(struct op *id_op);

struct op *process_derefer_get(struct op *id_op);

struct op *process_reference(struct op *id_op);

struct op *process_int(int integer);

struct op *process_float(double floatnum);

struct op *process_char(char character);
#endif