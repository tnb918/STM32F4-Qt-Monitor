#include "DS_18B20.h"
#define EnableINT()
#define DisableINT()

#define DS_PRECISION 		0x7f   //精度配置寄存器 1f=9位; 3f=10位; 5f=11位; 7f=12位;
#define DS_AlarmTH  		0x64
#define DS_AlarmTL  		0x8a
#define DS_CONVERT_TICK 1000

#define ResetDQ() HAL_GPIO_WritePin(DATA_GPIO_Port, DATA_Pin, GPIO_PIN_RESET)
#define SetDQ()  	HAL_GPIO_WritePin(DATA_GPIO_Port, DATA_Pin, GPIO_PIN_SET)
#define GetDQ()  	HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin) 

// 微秒延时
void DWT_Init(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;	// 使能DWT外设
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk ; 					// 使能Cortex-M DWT CYCCNT寄存器
	DWT->CYCCNT = 0; 																// DWT CYCCNT寄存器计数清0
}

#pragma GCC push_options
#pragma GCC optimize ("O3")
void delayUS_DWT(uint32_t us) {
		volatile uint32_t cycles = (SystemCoreClock/1000000L)*us;
		volatile uint32_t start = DWT->CYCCNT;
		do  {
		} while(DWT->CYCCNT - start < cycles);
}
#pragma GCC pop_options

unsigned char ResetDS18B20(void)
{
	unsigned char resport;
	SetDQ();
	delayUS_DWT(50);

	ResetDQ();
	delayUS_DWT(500);  //500us （该时间的时间范围可以从480到960微秒）
	SetDQ();
	delayUS_DWT(40);  //40us
	//resport = GetDQ();
	uint16_t cnt = 0;
	while(GetDQ() && cnt < 500)
	{
		++cnt;
		delayUS_DWT(1);  //40us
	}
	if (cnt >= 500)
		resport = 1;
	else
		resport = 0;
	
	delayUS_DWT(500);  //500us
	SetDQ();
	return resport;
}

void DS18B20WriteByte(unsigned char Dat)
{
	unsigned char i;
	for(i = 8; i > 0; i--)
	{
		ResetDQ();     		//在15u内送数到数据线上，DS18B20在15-60u读数
		delayUS_DWT(5);   //5us
		if(Dat & 0x01)
			SetDQ();
		else
			ResetDQ();
		delayUS_DWT(65);  //65us
		SetDQ();
		delayUS_DWT(2);  	//连续两位间应大于1us
		Dat >>= 1;
	} 
}


unsigned char DS18B20ReadByte(void)
{
	unsigned char i,Dat;
	SetDQ();
	delayUS_DWT(5);
	for(i = 8; i > 0; i--)
	{
		Dat >>= 1;
		ResetDQ();     //从读时序开始到采样信号线必须在15u内，且采样尽量安排在15u的最后
		delayUS_DWT(5);   //5us
		SetDQ();
		delayUS_DWT(5);   //5us
		if(GetDQ())
			Dat |= 0x80;
		else
			Dat &= 0x7f;  
		delayUS_DWT(65);   //65us
		SetDQ();
	}
	return Dat;
}

void ReadRom(unsigned char *Read_Addr)
{
	unsigned char i;

	DS18B20WriteByte(ReadROM);
	for(i = 8; i > 0; i--)
	{
		*Read_Addr = DS18B20ReadByte();
		Read_Addr++;
	}
}

void DS18B20Init(unsigned char Precision,unsigned char AlarmTH,unsigned char AlarmTL)
{
	DisableINT();
	ResetDS18B20();
	
	DS18B20WriteByte(SkipROM); 
	DS18B20WriteByte(WriteScratchpad);
	DS18B20WriteByte(AlarmTL);
	DS18B20WriteByte(AlarmTH);
	DS18B20WriteByte(Precision);

	ResetDS18B20();
	DS18B20WriteByte(SkipROM); 
	DS18B20WriteByte(CopyScratchpad);
	EnableINT();

	while(!GetDQ());  //等待复制完成 ///////////
}

void DS18B20StartConvert(void)
{
	DisableINT();
	ResetDS18B20();
	DS18B20WriteByte(SkipROM); 
	DS18B20WriteByte(StartConvert); 
	EnableINT();
}

void DS18B20_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  GPIO_InitStruct.Pin = DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}


void ds18b20_init(void)
{
	DWT_Init();
	DS18B20_Configuration();
	ResetDS18B20();
	DS18B20Init(DS_PRECISION, DS_AlarmTH, DS_AlarmTL);
	DS18B20StartConvert();
}


float ds18b20_read(void)
{
	unsigned char DL, DH;
	unsigned short TemperatureData;
	float Temperature;

	DisableINT();
	DS18B20StartConvert();
	ResetDS18B20();
	DS18B20WriteByte(SkipROM); 
	DS18B20WriteByte(ReadScratchpad);
	DL = DS18B20ReadByte();
	DH = DS18B20ReadByte(); 
	EnableINT();

	TemperatureData = DH;
	TemperatureData <<= 8;
	TemperatureData |= DL;

	Temperature = (float)((float)TemperatureData * 0.0625); //分辨率为0.0625度

	return  Temperature;
}
