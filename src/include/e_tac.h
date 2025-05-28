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
#define NO_INDEX 0                        // 无地址标识
#define NOT_POINTER 0                     // 非指针
#define IS_POINTER 1                      // 非指针

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
#define NO_TYPE -1   // 无类型
#define ID_VAR 0     // 变量
#define ID_FUNC 1    // 函数
#define ID_TEMP 2    // 临时变量
#define ID_NUM 3     // 数字常量
#define ID_LABEL 4   // 标签
#define ID_STRING 5  // 字符串

// 数据类型
#define PTR_OFFSET 10
#define REF_OFFSET 20
#define NO_DATA -2       // 无数据类型（未定义）
#define DATA_VOID -1     // 空数据类型 // hjj: todo, 尚未实装检测return
#define DATA_INT 0       // 整型
#define DATA_LONG 1      // 长整型
#define DATA_FLOAT 2     // 浮点型
#define DATA_DOUBLE 3    // 双精度浮点型
#define DATA_CHAR 4      // 单字符型
#define DATA_PINT 10     // 整型指针
#define DATA_PLONG 11    // 长整型指针
#define DATA_PFLOAT 12   // 浮点型指针
#define DATA_PDOUBLE 13  // 双精度浮点型指针
#define DATA_PCHAR 14    // 单字符型指针
#define DATA_RINT 20     // 整型引用
#define DATA_RLONG 21    // 长整型引用
#define DATA_RFLOAT 22   // 浮点型引用
#define DATA_RDOUBLE 23  // 双精度浮点型引用
#define DATA_RCHAR 24    // 单字符型引用

#define DATA_IS_POINTER(type) ((type >= DATA_PINT) && (type <= DATA_PCHAR))
#define DATA_IS_REF(type) ((type >= DATA_RINT) && (type <= DATA_RCHAR))
#define POINTER_TO_CONTENT(type) (type - PTR_OFFSET)
#define CONTENT_TO_POINTER(type) (type + PTR_OFFSET)
#define REF_TO_CONTENT(type) (type - REF_OFFSET)
#define CONTENT_TO_REF(type) (type + REF_OFFSET)
#define REF_TO_POINTER(type) (type - REF_OFFSET + PTR_OFFSET)
#define POINTER_TO_REF(type) (type + REF_OFFSET - PTR_OFFSET)

#define POINTER_TO_REF(type) (type + REF_OFFSET - PTR_OFFSET)

// 三地址码类型
#define TAC_UNDEF -1        // 未定义
#define TAC_END 0           // 结束
#define TAC_LABEL 1         // 标签
#define TAC_BEGIN 2         // 函数开始
#define TAC_PARAM 3         // 参数
#define TAC_VAR 4           // 变量声明
#define TAC_IFZ 5           // 条件跳转（if not）
#define TAC_CALL 6          // 函数调用
#define TAC_RETURN 7        // 返回
#define TAC_OUTPUT 8        // 输出
#define TAC_INPUT 9         // 输入
#define TAC_ASSIGN 10       // 赋值
#define TAC_NEGATIVE 12     // 取负
#define TAC_INTEGER 13      // 整数常量
#define TAC_IDENTIFIER 14   // 标识符
#define TAC_ARG 15          // 函数参数
#define TAC_GOTO 16         // 无条件跳转
#define TAC_PLUS 20         // 加法
#define TAC_MINUS 21        // 减法
#define TAC_MULTIPLY 22     // 乘法
#define TAC_DIVIDE 23       // 除法
#define TAC_EQ 24           // 等于
#define TAC_NE 25           // 不等于
#define TAC_LT 26           // 小于
#define TAC_LE 27           // 小于等于
#define TAC_GT 28           // 大于
#define TAC_GE 29           // 大于等于
#define TAC_REFER 30        // 引用
#define TAC_DEREFER_PUT 31  // 解引用并赋值
#define TAC_DEREFER_GET 32  // 解引用但不赋值

#define BUF_ALLOC(buf) char buf[BUF_SIZE] = {0};

#define NAME_ALLOC(name) char name[NAME_SIZE] = {0};

#define NEW_TAC_0(type) new_tac(type, NULL, NULL, NULL)

#define NEW_TAC_1(type, id_1) new_tac(type, id_1, NULL, NULL)

#define NEW_TAC_2(type, id_1, id_2) new_tac(type, id_1, id_2, NULL)

#define NEW_TAC_3(type, id_1, id_2, id_3) new_tac(type, id_1, id_2, id_3)

#define MALLOC_AND_SET_ZERO(pointer, amount, type)     \
	pointer = (type *)malloc((amount) * sizeof(type)); \
	memset(pointer, 0, (amount) * sizeof(type));

#define NEW_BUILT_IN_FUNC_ID(identifier, func_name, type) \
	MALLOC_AND_SET_ZERO(identifier, 1, struct id);        \
	identifier->name = func_name;                         \
	identifier->id_type = ID_FUNC;                        \
	identifier->data_type = type;                         \
	identifier->offset = -1;

#define TAC_IS_CMP(cal) (cal >= TAC_EQ && cal <= TAC_GE)
#define ID_IS_CONST(id) (id->id_type == ID_NUM || id->id_type == ID_STRING)
#define ID_IS_GCONST(id_type, data_type)                                      \
	(id_type == ID_STRING || id_type == ID_NUM && (data_type == DATA_FLOAT || \
	                                               data_type == DATA_DOUBLE))
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
#define TYPE_CHECK(id1, id2)                                            \
	((id1->data_type == id2->data_type) ||                              \
	 ((id1->data_type == DATA_CHAR) && (id2->data_type == DATA_INT)) || \
	 ((id1->data_type == DATA_INT) && (id2->data_type == DATA_CHAR)))

#ifdef HJJ_DEBUG
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
struct id {
	const char *name;
	union {
		int num_int;
		float
		    num_float;  // XXX:默认只实现float了先，process_float中也默认传入DATA_FLOAT了,asm_lc中的取址逻辑也要因此修改
		char num_char;
	} num;

	int id_type;
	int data_type;  // control the type of 'num'

	int scope;
	int offset;
	int label;

	struct id *next;

	// used in pointer deref
	int temp_derefer_put;
	// int temp_derefer_get;

	struct tac *func_param;  // ID_func的参数列表，为了实现类型转换

	int index;       // 数组
	int is_pointer;  // 指针
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

// 符号表
void reset_table(int direction);
// void clear_table(int scope);
struct id *find_identifier(const char *name);
struct id *find_func(const char *name);
struct id *add_identifier(const char *name, int id_type, int data_type,
                          int index, int is_pointer);
struct id *add_const_identifier(const char *name, int id_type, int data_type);

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
struct id *new_temp(int data_type);
struct id *new_label();
struct block *new_block(struct id *label_begin, struct id *label_end);

// 字符串处理
const char *id_to_str(struct id *id);
const char *data_to_str(int type);
void output_tac(FILE *f, struct tac *code);
void source_to_tac(FILE *f, struct tac *code);
void input_str(FILE *f, const char *format, ...);

#endif