#!/bin/bash

if [[ "$(uname -m)" != arm* ]];then
    echo "请在ARM架构开发板执行此脚本"
    exit 1
fi
# --- 配置区 ---
SERVER_NAME="server"
MODULE_NAME="ap3216c"
DEV_NODE="/dev/ap3216c"
PROJECT_DIR=$(pwd)

echo "\033[33m>>> [Board] 启动系统卸载与环境清理...\033[0m"

# 1. 停止运行中的服务进程
echo "[1/4] 正在停止传感器服务器与穿透服务..."
# 杀死 C 服务器
PID_SERVER=$(pidof $SERVER_NAME)
if [ ! -z "$PID_SERVER" ]; then
    kill -9 $PID_SERVER
    echo "已终止 $SERVER_NAME (PID: $PID_SERVER)"
fi

# 杀死 FRP 客户端
PID_FRPC=$(pidof frpc)
if [ ! -z "$PID_FRPC" ]; then
    kill -9 $PID_FRPC
    echo "已终止 frpc 隧道 (PID: $PID_FRPC)"
fi

# 2. 卸载内核驱动模块
echo "[2/4] 正在检查并卸载内核驱动..."
if lsmod | grep -q "$MODULE_NAME"; then
    rmmod $MODULE_NAME
    if [ $? -eq 0 ]; then
        echo "\033[32m[OK] 驱动 $MODULE_NAME 已成功卸载\033[0m"
    else
        echo "\033[31m[错误] 驱动卸载失败，可能设备正忙！\033[0m"
    fi
else
    echo "驱动模块未加载，跳过。"
fi

# 3. 清理残留的设备节点 (通常 rmmod 会自动处理，这里做二次确认)
if [ -c "$DEV_NODE" ]; then
    rm -f $DEV_NODE
    echo "清理设备节点: $DEV_NODE"
fi

# 4. 删除二进制文件 (可选，如果你想彻底从板子上移除)
echo "[4/4] 是否彻底删除当前目录下的可执行文件? (y/n)"
read confirm
if [ "$confirm" = "y" ] || [ "$confirm" = "Y" ]; then
    rm -f ./server ./ap3216c.ko ./frpc
    echo "\033[32m[OK] 二进制文件已清理。\033[0m"
else
    echo "保留二进制文件。"
fi

echo "\033[35m>>> 清理完成！系统已恢复初始状态。\033[0m"