#ifndef REG_MANAGER_H
#define REG_MANAGER_H

#ifndef DEBUG_PRINT
#define DEBUG_PRINT 1
#endif

#define R_UNDEF -1
#define R_a5 0
#define R_a4 1
#define R_a3 2
#define R_a2 3
#define R_a1 4
#define R_a0 5
#define R_a6 6
#define R_a7 7
#define R_t0 8
#define R_t1 9
#define R_t2 10
#define R_t3 11
#define R_t4 12
#define R_t5 13
#define R_t6 14
#define R_zero 15

//TODO:迟早被删
#define R_FLAG 0
#define R_IP 1
#define R_BP 2
#define R_JP 3
#define R_TP 4
#define R_GEN 0
#define R_NUM 15

/* frame */
#define FORMAL_OFF -0x10 	/* first formal parameter */
#define OBP_OFF 0 		/* dynamic chain */
#define RET_OFF 4 		/* ret address */
#define LOCAL_OFF -0x10		/* local var */

#define MODIFIED 1
#define UNMODIFIED 0

#include <stdio.h>

// 寄存器描述符结构
struct rdesc {
    struct id *var; // 当前寄存器存储的变量
    int mod;        // 是否被修改
	struct rdesc *next;
	struct rdesc *prev;
};

// 全局变量
extern struct rdesc rdesc[];
extern const char *reg_name[];
extern const char *args_name[];

#define RDESC_NUM(p_rdesc) (p_rdesc - rdesc)
#define FIND_LATEST_RDESC(first_appear,latest_appear) \
	int latest_appear = first_appear; \
	while (rdesc[latest_appear].next) { \
		latest_appear = RDESC_NUM(rdesc[latest_appear].next); \
	}

#define DATA_ALIGN 4

#define WORD 4
#define HWORD 2
#define BYTE 1

#define TYPE_SIZE(data_type) ( \
    (data_type) == DATA_INT    ? 4 : \
    (data_type) == DATA_LONG   ? 4 : \
    (data_type) == DATA_FLOAT  ? 4 : \
    (data_type) == DATA_CHAR   ? 1 : \
    (data_type) == DATA_DOUBLE ? 8 : \
    -1 /* 或者返回一个错误码，比如 -1, 或者 ((void)0) 引发编译错误 */ \
)
//XXX:这里认为no_data是string
#define TYPE_ALIGN(data_type) ( \
    (data_type) == NO_DATA     ? 2 : \
    (data_type) == DATA_INT    ? 2 : \
    (data_type) == DATA_LONG   ? 2 : \
    (data_type) == DATA_FLOAT  ? 2 : \
    (data_type) == DATA_CHAR   ? 1 : \
    (data_type) == DATA_DOUBLE ? 3 : \
    -1 /* 或者返回一个错误码，比如 -1, 或者 ((void)0) 引发编译错误 */ \
)
#define max(a,b) (a>b?a:b)
#define ALIGN(data_size) (max(DATA_ALIGN,data_size))
#define STORE_OP(data_size) ( \
    (data_size) == WORD    ? "sw" : \
    (data_size) == HWORD   ? "sh" : \
    (data_size) == BYTE    ? "sb" : \
    ""  \
)
#define LOAD_OP(data_size) ( \
    (data_size) == WORD    ? "lw" : \
    (data_size) == HWORD   ? "lhu" : \
    (data_size) == BYTE    ? "lbu" : \
    ""  \
)
#define LOCAL_VAR_OFFSET(identifier, _offset)  \
	do{	\
		int data_size = TYPE_SIZE(identifier->data_type);	\
		identifier->scope = LOCAL_TABLE;	\
		identifier->offset = _offset-data_size;	\
		_offset-=ALIGN(data_size);	\
	}while(0)

#define OP_TO_CAL(op,a,b) (    \
    strcmp(op,"add")==0 ? a+b : \
    strcmp(op,"sub")==0 ? a-b : \
    strcmp(op,"mul")==0 ? a*b : \
    strcmp(op,"div")==0 ? a/b : \
    -1 \
)
#define OP_TO_CMP(op,a,b) ( \
    op==TAC_EQ  ? a==b : \
    op==TAC_NE  ? a!=b : \
    op==TAC_LT  ? a<b : \
    op==TAC_LE  ? a<b : \
    op==TAC_GT  ? a>=b : \
    op==TAC_GE  ? a<=b : \
    -1  \
)

// 函数声明
void rdesc_clear_all(int r);
void rdesc_clear_prev(int r);
void rdesc_clear_temp(int r);
void rdesc_fill(int r, struct id *s, int mod);
void asm_write_back(int r);
void asm_load(int r, struct id *s);
void asm_load_var(struct id *a, const char *reg); // sw reg &a
void asm_store_var(struct id *a, const char *reg);//sw reg &a
int reg_find(struct id *s);
int reg_alloc(struct id *s);
int reg_get();
#endif // REG_MANAGER_H