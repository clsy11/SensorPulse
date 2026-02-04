#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>

/* * 传感器原始数据协议定义
 * 对应 AP3216C 寄存器 0x0A - 0x0F 的 6 字节二进制流
 */

/* --- 掩码定义 (Masks) --- */
/* IR (红外): 0x0A 寄存器 Bit 7 是溢出位, Bit 1:0 是低 2 位数据 */
#define PROTOCOL_IR_OVERFLOW_MASK    0x80  // 1000 0000
#define PROTOCOL_IR_LOW_DATA_MASK    0x03  // 0000 0011

/* PS (接近): 0x0E 寄存器 Bit 3:0 是低 4 位数据, 0x0F 寄存器 Bit 5:0 是高 6 位数据 */
#define PROTOCOL_PS_LOW_DATA_MASK    0x0F  // 0000 1111
#define PROTOCOL_PS_HIGH_DATA_MASK   0x3F  // 0011 1111

/* --- 协议封装结构体 --- */
/* 使用 #pragma pack(1) 确保编译器不进行字节对齐优化，保证 6 字节严丝合缝 */
#pragma pack(1)
typedef struct {
    uint8_t ir_l;   // 0x0A
    uint8_t ir_h;   // 0x0B
    uint8_t als_l;  // 0x0C
    uint8_t als_h;  // 0x0D
    uint8_t ps_l;   // 0x0E
    uint8_t ps_h;   // 0x0F
} RawSensorPacket;
#pragma pack()

/* --- 业务层结构体 (洗干净后的数据) --- */
typedef struct {
    uint16_t ir;
    uint16_t als;
    uint16_t ps;
    int ir_overflow;
} CleanedSensorData;

#endif