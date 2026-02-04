import socket
import time
import struct
import argparse
import sys
import os
import json

def parse_arguments():
    """解析命令行参数"""
    parser = argparse.ArgumentParser(description="AP3216C 远程监控客户端")
    parser.add_argument("-c", "--config", default="config.json", help="指定配置文件路径")
    parser.add_argument("--ip",help="覆盖配置文件中的服务器IP")
    parser.add_argument("--port", type=int, default = 10000, help="覆盖配置文件中的端口")
    return parser.parse_args()

def load_config(config_path):
    """加载 JSON 配置"""
    if not os.path.exists(config_path):
        print(f"警告: 找不到配置文件 {config_path}，将使用默认参数。")
        return {"server_ip": "127.0.0.1", "server_port": 10000}
    
    with open(config_path, 'r') as f:
        return json.load(f)

def start_client():
    args = parse_arguments()
    config = load_config(args.config)
    SERVER_IP = args.ip if args.ip else config.get("server_ip")
    SERVER_PORT = args.port if args.port else config.get("server_port")
    print(f"正在连接远程服务器: {SERVER_IP}:{SERVER_PORT} ...")
    while True:
        
        try:
            # 1. 创建 TCP 套接字
            client_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_sock.settimeout(5) # 设置 5 秒超时，防止死等
            
            print(f"正在尝试连接服务器 {SERVER_IP}:{SERVER_PORT}...")
            client_sock.connect((SERVER_IP, SERVER_PORT))
            print("连接成功！")
            
            while True:
                # --- 填空区 2: 发送请求 ---
                # 发送你的触发指令，比如 "Request"
                request_msg = "Request"
                client_sock.send(request_msg.encode('utf-8'))
                
                # 2. 接收板子回传的 6 字节原始数据
                data = client_sock.recv(1024)
                
                if not data:
                    print("服务器关闭了连接")
                    break
                
                # --- 填空区 3: 数据解析 ---
                # 假设板子发来的是 6 字节 Hex，代表三个 16 位整数 (IR, ALS, PS)
                # 我们用 struct.unpack 来解析原始字节
                if len(data) >= 6:
                    # 'H' 代表 unsigned short (2字节)，'<' 代表小端序
                    # 你需要根据你板子的字节序调整为 '<HHH' 或 '>HHH'
                    try:
                        ir_l =  data[0]
                        ir_h =  data[1]
                        als_l = data[2]
                        als_h = data[3]
                        ps_l =  data[4]
                        ps_h =  data[5]

                        ir_invalid = (ir_l >> 7) & 1
                        ir_val = (ir_h << 2) | (ir_l & 0x03)
                        als_val = (als_h << 8) | (als_l << 0)
                        ps_val = ((ps_h & 0x3f) << 4) | (ps_l & 0x0F)

                        #ir, als, ps = struct.unpack('<HHH', data[:6])
                        
                        # 在这里填入你的业务逻辑
                        if ir_invalid:
                            print(f"光太强！")
                        else:
                            print(f"【实时数据】红外(IR): {ir_val} | 光强(ALS): {als_val} | 接近(PS): {ps_val}")
                        
                    except Exception as e:
                        print(f"解析出错: {e} | 原始数据: {data.hex()}")
                
                # 控制采样频率，不要把板子冲爆了
                time.sleep(0.5)

        except (socket.error, socket.timeout) as e:
            print(f"网络异常: {e}，5秒后尝试重连...")
            time.sleep(5)
        except KeyboardInterrupt:
            print("\n用户手动退出")
            break
        finally:
            client_sock.close()

if __name__ == "__main__":
    start_client()