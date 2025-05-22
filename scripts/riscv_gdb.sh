#!/bin/bash

echo "Starting QEMU for RISC-V program..."

qemu-riscv32 -g 1234 ./test/test.o &

# 获取后台进程的 PID (可选，如果后续需要管理该进程)
qemu_pid=$!
echo "QEMU started in background with PID: $qemu_pid. Waiting for GDB connection on port 1234."

echo "Continuing with other commands in the script..."
# 在这里可以添加脚本需要执行的其他命令
# 例如：
sleep 1 # 等待QEMU启动
gdb-multiarch -x ./scripts/gdb_commands.txt ./test/test.o # 假设你有一个GDB脚本来自动连接和调试

echo "Script has finished its main tasks."

# 判断该进程是否结束
if ps -p $qemu_pid > /dev/null; then
    echo "QEMU with PID $qemu_pid has been sent the kill signal."
    kill $qemu_pid
fi
