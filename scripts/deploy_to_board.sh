#!/bin/bash

# --- 配置区 ---
BOARD_DEST="/home/root/sensor_project"  # 板子上的目标路径
ADB_CMD="adb" # 如果需要 sudo 权限，可改为 "sudo adb"

echo -e "\033[33m>>> [VM] 启动环境检查与部署...\033[0m"



# 2. 检查 ADB 连接
#$ADB_CMD get-state > /dev/null 2>&1
#if [ $? -ne 0 ]; then
#    echo -e "\033[31m[错误] ADB 未连接或设备未识别，请检查数据线或 adbd 状态。\033[0m"
#    exit 1
#fi

# 3. 执行顶层 Makefile 编译
echo -e "\033[32m[1/3] 正在调用顶层 Makefile 编译工程...\033[0m"
#cd .. # 进入根目录执行
echo "输入板载内核路径"
read -e KERNEL_PATH
echo "输入交叉编译工具链"
read -e ARM_TOOL
#检查交叉编译链环境
#if ! command -v $(ARM_TOOL) &> /dev/null; then
#    echo -e "\033[31m[错误] 未发现交叉编译器，请先配置环境变量！\033[0m"
#    exit 1 
#fi
if [[ -z "$KERNEL_PATH" || -z "$ARM_TOOL" ]]; then
    # 用户直接回车，不传 KERNELDIR 变量，触发 Makefile 的 ?= 逻辑
    make clean && make
  
else
    make clean && make KERNELDIR="${KERNEL_PATH}" CROSS_COMPILE="${ARM_TOOL}"
  
fi

if [ $? -ne 0 ]; then
      echo -e "\033[31m[错误] 编译失败！\033[0m"
      exit 1
fi

 4. 创建板子目标目录并推送文件
echo -e "\033[32m[2/3] 正在通过 ADB 推送文件到板子: $BOARD_DEST\033[0m"
$ADB_CMD shell "mkdir -p $BOARD_DEST"

# 推送核心文件
$ADB_CMD push driver/ap3216c.ko $BOARD_DEST/
$ADB_CMD push server/server $BOARD_DEST/
$ADB_CMD push scripts/run_on_board.sh $BOARD_DEST/
$ADB_CMD push scripts/uninstall.sh $BOARD_DEST/
$ADB_CMD push scripts/frp_0.51.3_linux_arm $BOARD_DEST/ # 如果需要穿透

# 5. 权限设置
echo -e "\033[32m[3/3] 设置板子端脚本执行权限...\033[0m"
$ADB_CMD shell "chmod +x $BOARD_DEST/*.sh $BOARD_DEST/server $BOARD_DEST/frp_0.51.3_linux_arm/frpc"

echo -e "\033[35m>>> [VM] 部署完成！请在板子上运行 scripts/run_on_board.sh\033[0m"