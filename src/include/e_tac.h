#ifndef E_TAC_H
#define E_TAC_H

#include <stdarg.h>
#include <stdio.h>

#include "e_config.h"

// 常量定义
#define MAX 100        // 最大值
#define BUF_SIZE 64    // 缓冲区大小
#define NAME_SIZE 256  // 名称字符串的最大长度

#define NUM_ZERO (struct op *)1145141919  // negative占位
#define NUM_ONE (struct op *)114514       // inc&dec占位
#define NO_ADDR -1                        // 无地址标识
#define NO_INDEX (struct arr_info *)NULL  // 无下标

// 符号表操作方向
#define INC_HEAD 0  // 增加到表头
#define INC_TAIL 1  // 增加到表尾
#define DEC_HEAD 2  // 从表头减少
#define DEC_TAIL 3  // 从表尾减少

// 符号表范围
#define LOCAL_TABLE 1                 // 局部符号表
#define GLOBAL_TABLE 0                // 全局符号表
#define INTO_LOCAL_TABLE LOCAL_TABLE  // 进入局部符号表
#define OUT_LOCAL_TABLE GLOBAL_TABLE  // 退出局部符号表

// 是否确认标识符已存在
#define CHECK_ID_NOT_EXIST 0
#define CHECK_ID_EXIST 1

// 类型检查中两操作数是否是赋值关系
#define IS_NOT_ASSIGN 0
#define IS_ASSIGN 1

// 符号类型
#define ID_NO_TYPE -1   // 无类型
#define ID_UNDEFINED 0  // 未定义类型
#define ID_VAR 1        // 变量
#define ID_FUNC 2       // 函数
#define ID_NUM 3        // 数字常量
#define ID_LABEL 4      // 标签
#define ID_STRING 5     // 字符串
#define ID_STRUCT 6     // 结构体

// 变量类型
#define NO_TYPE (struct var_type *)NULL  // 无类型

// 数据类型
#define PTR_OFFSET 10
#define REF_OFFSET 20
#define DATA_VOID -1      // 空数据类型 // hjj: todo, 尚未实装检测return
#define DATA_UNDEFINED 0  // 无数据类型（未定义）
#define DATA_INT 1        // 整型
#define DATA_LONG 2       // 长整型
#define DATA_FLOAT 3      // 浮点型
#define DATA_DOUBLE 4     // 双精度浮点型
#define DATA_CHAR 5       // 单字符型
#define DATA_STRUCT_INIT 100

// #define DATA_IS_POINTER(type) ((type >= DATA_PINT) && (type <= DATA_PCHAR))
// #define DATA_IS_REF(type) ((type >= DATA_RINT) && (type <= DATA_RCHAR))

// #define POINTER_TO_CONTENT(type_1, type_2)         \
// 	((type_1)->data_type == (type_2)->data_type && \
// 	 (type_1)->pointer_level == (type_2)->pointer_level + 1)
#define POINTER_TO_CONTENT(type_op_1, type_op_2)         \
	((type_op_1)->addr->variable_type->data_type ==      \
	     (type_op_2)->addr->variable_type->data_type &&  \
	 ((type_op_2)->addr->variable_type->pointer_level == \
	  (type_op_1)->addr->variable_type->pointer_level -  \
	      (type_op_1)->deref_count))
// #define CONTENT_TO_POINTER(type_1, type_2)         \
// 	((type_1)->data_type == (type_2)->data_type && \
// 	 (type_1)->pointer_level + 1 == (type_2)->pointer_level)
#define CONTENT_TO_POINTER(type_op_1, type_op_2)         \
	((type_op_1)->addr->variable_type->data_type ==      \
	     (type_op_2)->addr->variable_type->data_type &&  \
	 ((type_op_1)->addr->variable_type->pointer_level == \
	  (type_op_2)->addr->variable_type->pointer_level -  \
	      (type_op_2)->deref_count))

#define REF_TO_CONTENT(type_1, type_2)                                       \
	((type_1)->data_type == (type_2)->data_type && (type_1)->is_reference && \
	 (type_1)->pointer_level == (type_2)->pointer_level)
#define CONTENT_TO_REF(type_1, type_2)                                       \
	((type_1)->data_type == (type_2)->data_type && (type_2)->is_reference && \
	 (type_1)->pointer_level == (type_2)->pointer_level)
#define REF_TO_POINTER(type_1, type_2)                                       \
	((type_1)->data_type == (type_2)->data_type && (type_1)->is_reference && \
	 (type_1)->pointer_level + 1 == (type_2)->pointer_level)
#define POINTER_TO_REF(type_1, type_2)                                       \
	((type_1)->data_type == (type_2)->data_type && (type_2)->is_reference && \
	 (type_1)->pointer_level == (type_2)->pointer_level + 1)

// 三地址码类型
#define TAC_UNDEF -1           // 未定义
#define TAC_END 0              // 结束
#define TAC_LABEL 1            // 标签
#define TAC_BEGIN 2            // 函数开始
#define TAC_PARAM 3            // 参数
#define TAC_VAR 4              // 变量声明
#define TAC_IFZ 5              // 条件跳转（if not）
#define TAC_CALL 6             // 函数调用
#define TAC_RETURN 7           // 返回
#define TAC_OUTPUT 8           // 输出
#define TAC_INPUT 9            // 输入
#define TAC_ASSIGN 10          // 赋值
#define TAC_NEGATIVE 12        // 取负
#define TAC_INTEGER 13         // 整数常量
#define TAC_IDENTIFIER 14      // 标识符
#define TAC_ARG 15             // 函数参数
#define TAC_GOTO 16            // 无条件跳转
#define TAC_PLUS 20            // 加法
#define TAC_MINUS 21           // 减法
#define TAC_MULTIPLY 22        // 乘法
#define TAC_DIVIDE 23          // 除法
#define TAC_EQ 24              // 等于
#define TAC_NE 25              // 不等于
#define TAC_LT 26              // 小于
#define TAC_LE 27              // 小于等于
#define TAC_GT 28              // 大于
#define TAC_GE 29              // 大于等于
#define TAC_REFER 30           // 引用
#define TAC_DEREFER_PUT 31     // 解引用并赋值
#define TAC_DEREFER_GET 32     // 解引用但不赋值
#define TAC_VAR_REFER_INIT 33  // 初始化引用变量

#define BUF_ALLOC(buf) char buf[BUF_SIZE] = {0};

#define NAME_ALLOC(name) char name[NAME_SIZE] = {0};

#define NEW_TAC_0(type) new_tac(type, NULL, NULL, NULL)

#define NEW_TAC_1(type, id_1) new_tac(type, id_1, NULL, NULL)

#define NEW_TAC_2(type, id_1, id_2) new_tac(type, id_1, id_2, NULL)

#define NEW_TAC_3(type, id_1, id_2, id_3) new_tac(type, id_1, id_2, id_3)

#define MALLOC_AND_SET_ZERO(pointer, amount, type)     \
	pointer = (type *)malloc((amount) * sizeof(type)); \
	memset(pointer, 0, (amount) * sizeof(type));

#define NEW_BUILT_IN_FUNC_ID(identifier, func_name, data_type)       \
	identifier = add_const_identifier(func_name, ID_FUNC,            \
	                                  new_const_type(data_type, 0)); \
	identifier->offset = -1;

#define TAC_IS_CMP(cal) (cal >= TAC_EQ && cal <= TAC_GE)
#define ID_IS_CONST(id_type) (id_type == ID_NUM || id_type == ID_STRING)
#define ID_IS_INTCONST(id_type, data_type) \
	(id_type == ID_NUM && (data_type == DATA_INT || data_type == DATA_CHAR))
#define ID_IS_GCONST(id_type, data_type)                                      \
	(id_type == ID_STRING || id_type == ID_NUM && (data_type == DATA_FLOAT || \
	                                               data_type == DATA_DOUBLE))
#define ID_IS_GLOBAL(id_type) \
	(id_type == ID_FUNC || id_type == ID_STRUCT || ID_IS_CONST(id_type))

#define TAC_TO_FUNC(cal)                \
	(cal == TAC_PLUS       ? "__addsf3" \
	 : cal == TAC_MINUS    ? "__subsf3" \
	 : cal == TAC_MULTIPLY ? "__mulsf3" \
	 : cal == TAC_DIVIDE   ? "__divsf3" \
	 : cal == TAC_EQ       ? "__eqsf2"  \
	 : cal == TAC_NE       ? "__nesf2"  \
	 : cal == TAC_LT       ? "__ltsf2"  \
	 : cal == TAC_LE       ? "__lesf2"  \
	 : cal == TAC_GT       ? "__gtsf2"  \
	 : cal == TAC_GE       ? "__gesf2"  \
	                       : "")
#define TYPE_CHECK(id1, id2)                                              \
	(((id1->variable_type->data_type == id2->variable_type->data_type) || \
	  ((id1->variable_type->data_type == DATA_CHAR) &&                    \
	   (id2->variable_type->data_type == DATA_INT)) ||                    \
	  ((id1->variable_type->data_type == DATA_INT) &&                     \
	   (id2->variable_type->data_type == DATA_CHAR))) &&                  \
	 (id1->variable_type->pointer_level == id2->variable_type->pointer_level))

#ifdef HJJ_TERMINAL
#define PRINT_3(string, var_1, var_2, var_3) printf(string, var_1, var_2, var_3)
#define PRINT_2(string, var_1, var_2) printf(string, var_1, var_2)
#define PRINT_1(string, var_1) printf(string, var_1)
#define PRINT_0(string) printf(string)
#else
#define PRINT_3(string, var_1, var_2, var_3) \
	fprintf(f, string, var_1, var_2, var_3)
#define PRINT_2(string, var_1, var_2) fprintf(f, string, var_1, var_2)
#define PRINT_1(string, var_1) fprintf(f, string, var_1)
#define PRINT_0(string) fprintf(f, string)
#endif

// 符号
#define NOT_REF 0
#define IS_REF 1
struct var_type {
	int data_type;
	int pointer_level;
	int is_reference;
};

#define NOT_CONST_INDEX 0
#define IS_CONST_INDEX 1
#define NOT_DECLARATION 0
#define MAYBE_DECLARATION 1
struct arr_info {
	int max_level;
	int in_declaration_or_not;
	int array_offset[MAX];
	int const_or_not[MAX];
	int const_index[MAX];
	struct op *nonconst_index[MAX];
};

struct func_info {
	struct tac *param_list;  // ID_func的参数列表，为了实现类型转换
};

struct num_info {
	union {
		int num_int;
		float
		    num_float;  // XXX:默认只实现float了先，process_float中也默认传入DATA_FLOAT了,asm_lc中的取址逻辑也要因此修改
		char num_char;
	} num;
};

struct member_def {
	const char *name;
	int member_offset;

	struct var_type *variable_type;

	struct arr_info *array_info;

	struct member_def *next_def;
};

#define NOT_POINTER_FETCH 0
#define IS_POINTER_FETCH 1
struct member_ftch {
	const char *name;
	int is_pointer_fetch;
	struct arr_info *index_info;
};

struct strc_info {
	struct member_def *definition_list;
	int struct_type;
	int struct_offset;

	struct id *next_struct;
};

struct id {
	const char *name;

	int id_type;
	struct var_type *variable_type;

	int scope;
	int offset;
	int label;

	struct id *next;

	struct arr_info *array_info;
	struct num_info number_info;
	struct func_info function_info;
	struct strc_info struct_info;
};

// 三地址码
struct tac {
	int type;
	struct tac *prev;
	struct tac *next;
	struct id *id_1;
	struct id *id_2;
	struct id *id_3;
};

// 表达式
struct op {
	struct tac *code;
	struct id *addr;
	struct op *next;  // used in continue and break

	int deref_count;
};

// used in continue and break
struct block {
	struct id *label_begin;
	struct id *label_end;
	struct op *continue_stat_head;
	struct op *break_stat_head;
	struct block *prev;
};

extern int scope;
extern struct tac *tac_head;
extern struct tac *tac_tail;
extern FILE *source_file, *tac_file, *obj_file;
extern struct id *id_global, *id_local;

extern struct id *struct_table;
extern int cur_member_offset;

// 符号表
void reset_table(int direction);
// void clear_table(int scope);
struct id *find_identifier(const char *name);

struct id *find_func(const char *name);

struct id *add_identifier(const char *name, int id_type,
                          struct var_type *variable_type,
                          struct arr_info *array_info);
struct id *add_const_identifier(const char *name, int id_type,
                                struct var_type *variable_type);

// 三地址码表
void init_tac();
void cat_tac(struct op *dest, struct tac *src);
void cat_op(struct op *dest, struct op *src);             // 会释放src
struct op *cat_list(struct op *exp_1, struct op *exp_2);  // 会释放exp_2
struct op *cpy_op(struct op *src);

// 初始化
struct op *new_op();
struct tac *new_tac(int type, struct id *id_1, struct id *id_2,
                    struct id *id_3);
struct id *new_temp(struct var_type *variable_type);
struct id *new_label();
struct block *new_block(struct id *label_begin, struct id *label_end);

// 字符串处理
const char *id_to_str(struct id *id);
const char *data_to_str(struct var_type *variable_type,
                        struct arr_info *array_info);
void output_tac(FILE *f, struct tac *code);
void output_struct(FILE *f, struct id *id_struct);
void source_to_tac(FILE *f, struct tac *code);
void input_str(FILE *f, const char *format, ...);

#endif