make test
riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -O0 ./test/my_test.s -o ./test/test.o -fPIC