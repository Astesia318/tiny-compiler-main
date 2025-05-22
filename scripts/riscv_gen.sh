riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -O0 ./test/test.c -S -o ./test/test_t.s -fPIC
riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -O0 ./test/test.c -o ./test/test.o -fPIC
