# 1. 条件赋值：默认无交叉编译前缀（虚拟机本机编译），命令行可传参覆盖（板子交叉编译）
CROSS_COMPILE ?= 

# 2. 定义编译器：拼接交叉编译前缀，默认是 gcc（虚拟机），传参后是 arm-linux-gnueabihf-gcc（板子）
CC = $(CROSS_COMPILE)gcc

# 3. 编译选项（按需添加，比如警告、头文件目录）
CFLAGS = -Wall 

# 4. 定义目标可执行文件和源码文件
TARGET = test_driver
SRC = ./test_driver.c  # 替换成你的 .c 文件名，多个文件用 空格 分隔（如 main.c test.c）

# 5. 默认目标：编译生成可执行文件
all:
	$(CC) $(SRC) $(CFLAGS) -o $(TARGET)

# 6. 清理目标：删除可执行文件
clean:
	rm -f $(TARGET)