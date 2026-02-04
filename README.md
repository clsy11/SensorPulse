SensorPulse: 基于 Linux 驱动与并发线程池的传感器监控系统

## 1. 项目简介
本项目实现了一个从底层硬件驱动到上层分布式监控的全栈系统。通过 C 语言编写 Linux 内核驱动程序采集 AP3216C 传感器数据，利用多线程并发服务端进行实时分发，并由 Python 客户端进行业务解析与展示。

## 2. 系统架构
数据流向如下：
**[AP3216C 传感器]** -- (I2C 总线) --> 
**[Linux 内核驱动 (ap3216c.ko)]** -- (字符设备接口 /dev/ap3216) --> 
**[C 并发服务器 (Thread Pool)]** -- (TCP Socket / Binary Stream) --> 
**[Python 客户端 (Protocol Parser)]**

## 3. 技术要点

* **并发模型**：基于 epoll ET (边沿触发) + `EPOLLONESHOT` 机制的自定义线程池。
* **通信协议**：自定义 6 字节二进制流协议，手写位运算屏蔽寄存器状态位。
* **同步机制**：互斥锁保护任务队列，堆区动态分配任务参数解决多线程数据竞态。

## 4. 环境要求
* **硬件**：100ask I.MX6ULL 开发板 (或其他带 I2C 的板子)
* **内核**：Linux 4.1.15 / 5.4.x
* **编译器**：arm-linux-gnueabihf-gcc
* **工具**：adb(虚拟机推送文件向开发板)

## 5. 快速开始
1.确保开发板能上网，且在云服务器运行frps。配置./client/config.json,frpc.ini,frps.ini的公网ip为你的云服务器ip，选用合适的端口。
2.在虚拟机执行deploy_to_board.sh，要求输入KERNNEL时，请输入虚拟机上开发板内核地址。
  要求输入交叉编译工具链时，请输入与内核匹配的交叉编译工具链
3.在虚拟机/home/root/sensor_project下执行run_on_board.sh,开发板将自动执行server和frpc
4.在一个能上网的设备上执行client.py