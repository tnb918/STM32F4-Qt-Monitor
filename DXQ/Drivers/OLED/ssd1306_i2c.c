#include "ssd1306_i2c.h"
#include "main.h"

#define OLED_SCL		I2C_SCL_Pin
#define OLED_SDA		I2C_SDA_Pin
#define OLED_GPIO		I2C_SCL_GPIO_Port

#define OLED_SCL_H         HAL_GPIO_WritePin(OLED_GPIO, OLED_SCL,GPIO_PIN_SET)
#define OLED_SCL_L         HAL_GPIO_WritePin(OLED_GPIO, OLED_SCL,GPIO_PIN_RESET)

#define OLED_SDA_H         HAL_GPIO_WritePin(OLED_GPIO, OLED_SDA,GPIO_PIN_SET)
#define OLED_SDA_L         HAL_GPIO_WritePin(OLED_GPIO, OLED_SDA,GPIO_PIN_RESET)

//#define SDA_IN()  {GPIOB->MODER&=~(3<<(7*2));GPIOB->MODER|=0<<7*2;}	//PB7输入模式
//#define SDA_OUT() {GPIOB->MODER&=~(3<<(7*2));GPIOB->MODER|=1<<7*2;} //PB7输出模式
//#define OLED_SCL_read      HAL_GPIO_ReadPin(OLED_GPIO, OLED_SCL)
//#define OLED_SDA_read      HAL_GPIO_ReadPin(OLED_GPIO, OLED_SDA)

//#define IIC_SCL    PBout(6) //SCL
//#define IIC_SDA    PBout(7) //SDA	 
//#define READ_SDA   PBin(7)  //输入SDA 
static void I2C_delay(void)
{
    volatile int i = 10;
    while (i)
        i--;
}

void OLED_IIC_Start(void)
{
	OLED_SDA_H;	  	  
	OLED_SCL_H;  
	I2C_delay();
 	OLED_SDA_L;     //START:when CLK is high,DATA change form high to low 
	I2C_delay();
	OLED_SCL_L;     //钳住I2C总线，准备发送或接收数据 
}

void OLED_IIC_Stop(void)
{
	OLED_SCL_L;
	OLED_SDA_L;//STOP:when CLK is high DATA change form low to high
	I2C_delay();
	OLED_SCL_H; 
	OLED_SDA_H;//发送I2C总线结束信号
	I2C_delay();
}

void OLED_IIC_Ack(void)
{	
	u8 ucErrTime=0;
	OLED_SDA_H;
	I2C_delay();
	OLED_SCL_H;
	I2C_delay();
	OLED_SCL_L;
}

void OLED_IIC_SendByte(u8 dat)
{
    u8 i;   
    OLED_SCL_L;//拉低时钟开始数据传输
	 
    for(i = 0; i < 8; i++)
    {             
			if(dat & 0x80)
        OLED_SDA_H;
			else 
				OLED_SDA_L;
      dat <<= 1;
			
			I2C_delay();
			OLED_SCL_H;
			I2C_delay();
			OLED_SCL_L;	
			I2C_delay();
    }	 
}
