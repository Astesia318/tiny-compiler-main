## 语法规则

和semantic一致，但增加了：
1. for循环，且循环各条件表达式可为空。
1. 自增和自减，位置可在变量之前或之后。
1. 类型int，long，float和double及其类型检查。
1. break&continue。

## 已实现的部分

目前已将三人负责的部分各自打包为共享库so文件，具体情况如下：

riscv.c -> libriscv.so

custom.c -> libcustom.so

internal.c -> libinternal.so

剩余代码为公共部分，在拥有以上库的前提下，可以在删除上述对应的三个源代码的情况下make出目标程序

## 如何运行编译器

scripts/riscv_gen.sh（可选）使用gcc生成.s并使用gcc生成.o

scripts/riscv_my_gen.sh     使用本编译器生成.s并使用gcc生成.o

scripts/riscv_gdb.sh 1/0    若为1则使用qemu执行.o并转发端口使用gdb调试以便观察执行结果；若未0则直接运行
