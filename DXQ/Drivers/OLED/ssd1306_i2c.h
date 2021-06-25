#ifndef _SSD1306_I2C_H
#define _SSD1306_I2C_H
#include "main.h"
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
void OLED_IIC_Start(void);
void OLED_IIC_Stop(void);
void OLED_IIC_Ack(void);
void OLED_IIC_SendByte(u8 dat);
#endif
