#!/bin/bash

# --- 配置区 ---
DEV_NODE="/dev/ap3216c"
SERVER_PORT=10000

echo "\033[33m>>> [Board] 启动运行环境检查...\033[0m"

# 1. 检查 I2C 总线硬件
# 使用 i2cdetect 检查 0x1e 地址是否响应
if i2cdetect -y 0 | grep -q "1e"; then 
    echo "\033[32m[OK] 传感器硬件在线 (Addr: 0x1e)\033[0m"
else
    echo "\033[31m[警告] 未在 I2C 总线上发现传感器，请检查接线！或i2cdetect -y 0是否存在却未能匹配成功\033[0m"
fi

# 2. 检查并加载驱动
if [ ! -c "$DEV_NODE" ]; then
    echo "[1/2] 设备节点不存在，尝试加载驱动..."
    if [ -f "./ap3216c.ko" ]; then
        insmod ap3216c.ko
        sleep 1
        if [ -c "$DEV_NODE" ]; then
            echo "\033[32m[OK] 驱动加载成功，节点: $DEV_NODE\033[0m"
        else
            echo "\033[31m[错误] 驱动加载失败，请检查 dmesg 日志！\033[0m"
            exit 1
        fi
    else
        echo "\033[31m[错误] 未找到 ap3216c.ko 驱动文件！\033[0m"
        exit 1
    fi
else
    echo "\033[32m[OK] 驱动已处于加载状态。\033[0m"
fi

# 3. 检查端口占用
if netstat -tunl | grep -q ":$SERVER_PORT"; then
    echo "\033[31m[错误] 端口 $SERVER_PORT 已被占用，请先关闭旧进程！\033[0m"
    exit 1
fi

# 4. 启动穿透隧道 (后台运行)
if [ -f "./frp_0.51.3_linux_arm/frpc" ]; then
    echo "[2/2] 正在后台启动 FRP 穿透隧道..."
    ./frp_0.51.3_linux_arm/frpc -c ./frp_0.51.3_linux_arm/frpc.ini > /dev/null 2>&1 &
fi

# 5. 启动 C 服务器
echo "\033[32m>>> 启动 C 传感器服务器...\033[0m"
./server 