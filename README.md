## 语法规则

和semantic一致，但增加了：
1. for循环，且循环各条件表达式可为空。
1. 自增和自减，位置可在变量之前或之后。
1. 类型int，long，float和double及其类型检查。
1. break&continue。

## 已实现的部分

实现了以上语法规则下的三地址码及汇编代码生成。

对于目前的功能支持了生成riscv的汇编代码
scripts/riscv_my_gen.sh     使用本编译器生成.s并使用gcc生成.o
scripts/riscv_gen.sh        使用gcc生成.s并使用gcc生成.o
scripts/riscv_gdb.sh        使用qemu执行.o并转发端口使用gdb调试以便观察执行结果