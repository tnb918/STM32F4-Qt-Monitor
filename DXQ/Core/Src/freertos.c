/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gpio.h"
#include "GUI.h"
#include "tim.h"
#include "DS_18B20.h"
#include "MPU6050.h"
#include <stdio.h>
#include <string.h>
#include "Display_3D.h"
#include "COMM.h"
#include "HC05.h"
#include "math.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern GUI_FLASH const GUI_FONT GUI_FontHZ_KaiTi_16;
extern GUI_FLASH const GUI_FONT GUI_FontHZ_SimSun_12;
extern GUI_CONST_STORAGE GUI_BITMAP bmTNB;
extern GUI_CONST_STORAGE GUI_BITMAP bmWYF;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
#define WS_LOGO 0
#define WS_GUI1 1
#define WS_GUI2 2
#define WS_GUI3 3
#define WS_GUI4 4

int g_ws = 0;
uint32_t intick = 0;
uint32_t tick = 0;
uint8_t leds_sta = 0x00;
uint32_t beeptick =0;
volatile float temp = 0;
uint8_t tempwarn = 0;
uint8_t mpuwarn =0;
uint8_t pageidx = 0;
uint32_t warntick = 0;
uint32_t interval = 50;//数据采集间隔
bool flag = true;

float tempLmt = 32.0f; //温度上限
uint16_t g_upstep = 100;//上传时间间隔
uint8_t g_mpustep = 7;//振动检测灵敏度
uint8_t g_warntime = 30;//报警时长
uint8_t g_paridx = 0;//参数索引
uint8_t g_bUping = 0;

#define SEND_LED_TIME 50
#define RECV_LED_TIME 100
uint32_t send_data = 0;
uint32_t recv_data = 0;
bool send_flag = false;
bool recv_flag = false;
bool init_flag = false;
uint32_t recv_tick = 0;
uint32_t send_tick = 0;

#define MAX_DATALEN 80
float vTemp[MAX_DATALEN];
int cTemp = 0;
float vPitch[MAX_DATALEN];
int cPitch = 0;
float vRoll[MAX_DATALEN];
int cRoll = 0;
float vYaw[MAX_DATALEN];
int cYaw = 0;

unsigned char TAB_TANG[128] = {	/* 笑脸 0xccc0*/
0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,0xE0,0xF0,0xF0,0x78,0x38,0x38,0x38,0x38,
0x38,0x38,0x38,0x38,0x78,0xF0,0xF0,0xE0,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0xF8,0xFE,0xFF,0x0F,0x07,0x01,0x00,0xF0,0xF0,0xF0,0xF0,0x00,0x00,
0x00,0x00,0xF0,0xF0,0xF0,0xF0,0x00,0x01,0x07,0x0F,0xFF,0xFE,0xF8,0x00,0x00,0x00,
0x00,0x00,0x00,0x1F,0x7F,0xFF,0xF0,0xE0,0x88,0x18,0x38,0x30,0x70,0x60,0x60,0x60,
0x60,0x60,0x60,0x70,0x30,0x38,0x18,0x88,0xE0,0xF0,0xFF,0x7F,0x1F,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x07,0x07,0x0F,0x0F,0x1E,0x1C,0x1C,0x1C,0x1C,
0x1C,0x1C,0x1C,0x1C,0x1E,0x0F,0x0F,0x07,0x07,0x03,0x01,0x00,0x00,0x00,0x00,0x00
};



/* USER CODE END Variables */
/* Definitions for MainTask */
osThreadId_t MainTaskHandle;
const osThreadAttr_t MainTask_attributes = {
  .name = "MainTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KeyTask */
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UartTask */
osThreadId_t UartTaskHandle;
const osThreadAttr_t UartTask_attributes = {
  .name = "UartTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GUITask */
osThreadId_t GUITaskHandle;
const osThreadAttr_t GUITask_attributes = {
  .name = "GUITask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for DataTask */
osThreadId_t DataTaskHandle;
const osThreadAttr_t DataTask_attributes = {
  .name = "DataTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void WSLogo(void);
void DrawLogo(void);
void DrawGUI1(void);
void DrawGUI2(void);
void DrawGUI3(void);
void DrawGUI4(void);
void BeepWork(int time, int tune);
void DispSeg(uint8_t num[4], uint8_t dot);
void BeepDone(void);
void InitHC05(void);
float BtoKB(float x);
/* USER CODE END FunctionPrototypes */

void StartMainTask(void *argument);
void StartKeyTask(void *argument);
void StartUartTask(void *argument);
void StartGUITask(void *argument);
void StartDataTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of MainTask */
  MainTaskHandle = osThreadNew(StartMainTask, NULL, &MainTask_attributes);

  /* creation of KeyTask */
  KeyTaskHandle = osThreadNew(StartKeyTask, NULL, &KeyTask_attributes);

  /* creation of UartTask */
  UartTaskHandle = osThreadNew(StartUartTask, NULL, &UartTask_attributes);

  /* creation of GUITask */
  GUITaskHandle = osThreadNew(StartGUITask, NULL, &GUITask_attributes);

  /* creation of DataTask */
  DataTaskHandle = osThreadNew(StartDataTask, NULL, &DataTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartMainTask */
/**
  * @brief  Function implementing the MainTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMainTask */
void StartMainTask(void *argument)
{
  /* USER CODE BEGIN StartMainTask */
	uint8_t num[4]={0};
  /* Infinite loop */
  for(;;)
  {
		switch(g_ws)
		{
			case WS_LOGO:
				WSLogo();
				break;
			default:
				break;
		}
		if(g_ws != WS_LOGO)
		{
			if((tempwarn || mpuwarn) && g_warntime > 0)
			{
				if(0 == warntick)
					warntick = osKernelGetTickCount();
				else if(osKernelGetTickCount() >= warntick + 1000 * g_warntime)
				{
					tempwarn = mpuwarn = 0;
					warntick = 0;
				}
				else
				{
					uint32_t tic = warntick + 1000 * g_warntime - osKernelGetTickCount();
					num[0] = (tic / 10000) % 10;
					num[1] = (tic / 1000) % 10;
					num[2] = (tic / 100) % 10;
					num[3] = (tic / 10) % 10;
					if(num[2] == 1 || num[2] == 3 || num[2] == 5)
					{
						int warnflag = (tempwarn?1:0)+(mpuwarn?2:0);
						switch(warnflag)
						{
							default:
								break;
							case 1:
								BeepWork(100,1);
							break;
							case 2:
								BeepWork(200,3);
							break;
							case 3:
								BeepWork(100,num[2]);			
							break;
						}
					}
				}
			}
			else
				num[0] = num[1] = num[2] = num[3] = ' ';
			DispSeg(num,2);			
		}
		BeepDone();
		
		if(recv_flag)
		{
			HAL_GPIO_WritePin(L3_GPIO_Port, L3_Pin, GPIO_PIN_RESET);
			if(osKernelGetTickCount() >= recv_tick + RECV_LED_TIME)
			{
				HAL_GPIO_WritePin(L3_GPIO_Port, L3_Pin, GPIO_PIN_SET);
				recv_flag = false;
				recv_tick = 0;
			}
		}
		if(send_flag)
		{
			HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, GPIO_PIN_RESET);
			if(osKernelGetTickCount() >= send_tick + g_upstep * 0.1)
			{
				HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, GPIO_PIN_SET);
				send_flag = false;
				send_tick = 0;
			}			
		}
			
    osDelay(1);
  }
  /* USER CODE END StartMainTask */
}

/* USER CODE BEGIN Header_StartKeyTask */
/**
* @brief Function implementing the KeyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartKeyTask */
void StartKeyTask(void *argument)
{
  /* USER CODE BEGIN StartKeyTask */
  /* Infinite loop */
	uint32_t keytick = 0;
  for(;;)
  {
		uint8_t key = ScanKey();
		
		if(tempwarn || mpuwarn)
		{
			if(g_keysta == (KEY1 | KEY2 | KEY3 | KEY4))
			{
				if(0 == keytick)
					keytick = osKernelGetTickCount();
				else if(osKernelGetTickCount() >= keytick + 3000)
				{
					tempwarn = mpuwarn = 0;
					warntick = 0;			
				}
			}
			else
				keytick = 0;
			key = 0xff;//按键无效
		}
		else
			keytick = 0;
		
		/*
		if(tempwarn || mpuwarn)
		{
			key = 0xff;
			if(keyLongPress())
			{
				tempwarn = mpuwarn = 0;
				warntick = 0;
				f1 = 1; f2 = 1;
			}
		}
		else
			key = ScanKey();
		*/
		
		switch(g_ws)
		{
			case WS_LOGO:
				if(KEY5 == key)
				{
					g_ws = WS_GUI1;
					BeepWork(500,1);
					SetLeds(0x00);					
				}
				break;
			case WS_GUI1:
				if(KEY6 == key)
				{
					g_ws = WS_LOGO;
					intick = 0;
				}				
				if(KEY1 == key)
				{
					g_ws = WS_GUI4;
					pageidx = 0;
				}
				else if(KEY4 == key)
				{
					g_ws = WS_GUI2;
					pageidx = 0;
				}
				else if(KEY2 == key)
				{
					if(pageidx > 0)
						--pageidx;					
				}
				else if(KEY3 == key)
				{
					if(pageidx < 2)
						++pageidx;
				}
				break;
				
			case WS_GUI2:
				if(KEY6 == key)
				{
					g_ws = WS_LOGO;
					intick = 0;
				}				
				else if(KEY1 == key)
				{
					g_ws = WS_GUI1;
					pageidx = 0;
				}
				else if(KEY4 == key)
				{
					g_ws = WS_GUI3;	
					//InitHC05();
					pageidx = 0;
				}
				else if(KEY2 == key)
				{
					if(pageidx > 0)
						--pageidx;
				}
				else if(KEY3 == key)
				{
					if(pageidx < 4)
						++pageidx;
				}
				else if(KEY5 == key)
				{
					(interval == 50) ? (interval = 500) : (interval = 50); 
				}
				break;
				
			case WS_GUI3:
				if(KEY1 == key)
				{
					g_ws = WS_GUI2;
					pageidx = 0;
				}
				else if(KEY4 == key)
				{
					g_ws = WS_GUI4;	
					pageidx = 0;
				}					
				else if(KEY2 == key)
					g_bUping = 1;
				else if(KEY3 == key)
					g_bUping = 0;				
				else if(KEY5 == key)
				{
					if(0 == pageidx)
					{
						//初始化HC05工作模式
						InitHC05();
					}
					else if(1 == pageidx)
					{
						send_data = 0;
						recv_data = 0;
					}
				}
				else if(KEY6 == key)
				{
					pageidx == 0 ? (pageidx = 1) : (pageidx = 0);
				}
				break;	
				
			case WS_GUI4:
				if(KEY6 == key)
				{
					g_ws = WS_LOGO;
					intick = 0;
				}
				if(KEY1 == key)
				{
					g_ws = WS_GUI3;
					//InitHC05();
					pageidx = 0;
				}
				else if(KEY4 == key)
				{
					g_ws = WS_GUI1;	
					pageidx = 0;
				}
				else if(KEY5 == key)
				{
					++g_paridx;
					g_paridx %= 4;
				}
				else if(KEY2 == key)
				{
					if(0 == g_paridx)
					{
						if(tempLmt < 90)
							tempLmt += 1;
					}
					if(1 == g_paridx)
					{
						if(g_mpustep < 9)
							++g_mpustep;
					}
					if(2 == g_paridx)
					{
						if(g_warntime < 60)
							++g_warntime;
					}	
					if(3 == g_paridx)
					{
						if(g_upstep < 10000)
							g_upstep += 100;
					}					
				}
				else if(KEY3 == key)
				{
					if(0 == g_paridx)
					{
						if(tempLmt >0)
							tempLmt -= 1;
					}
					if(1 == g_paridx)
					{
						if(g_mpustep > 0)
							--g_mpustep;
					}
					if(2 == g_paridx)
					{
						if(g_warntime > 0)
							--g_warntime;
					}	
					if(3 == g_paridx)
					{
						if(g_upstep > 100)
							g_upstep -= 100;
					}							
				}
				
				break;		
				
			default:
				break;
		}
    osDelay(10);
  }
  /* USER CODE END StartKeyTask */
}

/* USER CODE BEGIN Header_StartUartTask */
/**
* @brief Function implementing the UartTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUartTask */
void StartUartTask(void *argument)
{
  /* USER CODE BEGIN StartUartTask */
  /* Infinite loop */
	StartRecvUart1();
	HC05_Init();
  for(;;)
  {
		if(recv1_len > 0)
		{
			printf("%s", recv1_buff);
			recv1_len = 0;
		}
		
		HC05_Proc();
		if(hc05.recv_len > 0 )
		{
			printf("%s", hc05.recv_data);
			char *pb = (char *)(hc05.recv_data);
			char buf[50];
			if('Q' == pb[0] && 'P' == pb[1] && 'A' == pb[2] && 'R' == pb[3])
			{
				//查询参数，应答
				sprintf(buf,"P1:%.0f,P2:%d,P3:%d,P4:%d\n",tempLmt,g_mpustep,g_warntime,g_upstep);
				USendStr(&huart2,(uint8_t *)buf,strlen(buf));
				if(!hc05.bat)
				{
					send_data += strlen(buf);
					send_flag = true;
					send_tick = osKernelGetTickCount();
				}
			}
			else if('P' == pb[0] && '1' == pb[1] && ':' == pb[2])
			{
				//参数设置
				tempLmt = atof(pb + 3);
				pb = strstr(pb, "P2:");
				if(pb)
				{
					g_mpustep = atoi(pb + 3);
					pb = strstr(pb, "P3:");
					if(pb)
					{
						g_warntime = atoi(pb + 3);
						pb = strstr(pb, "P4:");
						if(pb)
						{
							g_upstep = atoi(pb + 3);
							USendStr(&huart2,(uint8_t *)"OK\n",3);
							if(!hc05.bat)
							{
								send_data += 3;
								send_flag = true;
								send_tick = osKernelGetTickCount();
							}
						}
					}
				}
			}
			else if('I' == pb[0] && 'F' == pb[1] && 'U' == pb[2] && 'P' == pb[3])
			{
				g_bUping == 1 ? (g_bUping = 0) : (g_bUping = 1);
			}
			
			if(!hc05.bat)
			{
				recv_data += hc05.recv_len;
				if(!init_flag)
					recv_flag = true;

				recv_tick = osKernelGetTickCount();
			}
			hc05.recv_len = 0;
			
		}
    osDelay(1);
  }
  /* USER CODE END StartUartTask */
}

/* USER CODE BEGIN Header_StartGUITask */
/**
* @brief Function implementing the GUITask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartGUITask */
void StartGUITask(void *argument)
{
  /* USER CODE BEGIN StartGUITask */
	GUI_Init();
  /* Infinite loop */
  for(;;)
  {
		switch(g_ws)
		{
			case WS_LOGO: DrawLogo(); break;
			case WS_GUI1: DrawGUI1(); break;
			case WS_GUI2: DrawGUI2(); break;
			case WS_GUI3: DrawGUI3(); break;
			case WS_GUI4: DrawGUI4(); break;			
			default: break;
		}
    osDelay(100);
  }

  /* USER CODE END StartGUITask */
}

/* USER CODE BEGIN Header_StartDataTask */
/**
* @brief Function implementing the DataTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDataTask */
void StartDataTask(void *argument)
{
  /* USER CODE BEGIN StartDataTask */
	osDelay(1000);
	ds18b20_init();
	uint8_t mpuok = MPU_init();
	uint8_t cnt = 0;
	while(cnt++ < 3 && !mpuok)
	{
		osDelay(500);
		mpuok = MPU_init();
	}
	
	InitHC05();

	uint32_t dstick = 0;
	uint32_t mputick = 0;
	uint32_t uptick = 0;
	int warncnt = 0;
  /* Infinite loop */
  for(;;)
  {
		if(osKernelGetTickCount() >= dstick + interval)
		{
			dstick = osKernelGetTickCount();
			float ft = ds18b20_read();
			if(ft < 125)
			{
				temp = ft;			

				if(cTemp < MAX_DATALEN)
					vTemp[cTemp++] = temp;
				else
				{
					memcpy((void*)vTemp,(void*)(vTemp+1),sizeof(vTemp[0])*(MAX_DATALEN-1));
					vTemp[MAX_DATALEN-1] = temp;
				}
				
				if(temp >= tempLmt)
				{
					printf("temp:%.1f",temp);
					tempwarn = 1;
					//warntick = osKernelGetTickCount();
				}
					
			}
		}

		if(mpuok)
		{
			if(osKernelGetTickCount() >= mputick + interval)
			{
				flag = !flag;
				mputick = osKernelGetTickCount();			
				MPU_getdata();
				if(flag || interval == 50)
				{
					//printf("1");
					if(cPitch < MAX_DATALEN)
						vPitch[cPitch++] = fAX;
					else
					{
						memcpy((void*)vPitch,(void*)(vPitch+1),sizeof(vPitch[0])*(MAX_DATALEN-1));
						vPitch[MAX_DATALEN-1] = fAX;
					}			
					
					if(cRoll < MAX_DATALEN)
						vRoll[cRoll++] = fAY;
					else
					{
						memcpy((void*)vRoll,(void*)(vRoll+1),sizeof(vRoll[0])*(MAX_DATALEN-1));
						vRoll[MAX_DATALEN-1] = fAY;
					}		
					
					if(cYaw < MAX_DATALEN)
						vYaw[cYaw++] = fAZ;
					else
					{
						memcpy((void*)vYaw,(void*)(vYaw+1),sizeof(vYaw[0])*(MAX_DATALEN-1));
						vYaw[MAX_DATALEN-1] = fAZ;
					}							
				}
			
				//int ss = gx*gx+gy*gy+gz*gz;
				//printf("%d\n",ss);
				if(g_mpustep >0 && (gx*gx+gy*gy+gz*gz > 1000 * pow(4,(10 - g_mpustep))) && (g_mpustep != 0))
				{
					if(++warncnt >= 3)//test
					{
						mpuwarn = 1;
						//printf("333333333333333333333333333333333333\n");
						//printf("axyz:%6d %6d %6d,gxyz:%6d %6d %6d\n",ax, ay, az, gx, gy, gz);
						//warntick = osKernelGetTickCount();
					}
				}
				else
					warncnt = 0;
			}
		}
		
		if(HC05_IsConn() && g_bUping)
		{
			if(osKernelGetTickCount() >= uptick +g_upstep)
			{
				uptick = osKernelGetTickCount();
				char buf[100];
				sprintf(buf,"T:%4.1f,A:%6d %6d %6d,G:%6d %6d %6d,F:%5.1f %5.1f %5.1f,W:%d\n",
				temp,ax,ay,az,gx,gy,gz,fAX,fAY,fAZ,(tempwarn?1:0)+(mpuwarn?2:0));
				USendStr(&huart2,(uint8_t*)buf,strlen(buf));
				
				if(!hc05.bat)
				{
					send_data += strlen(buf);
					send_flag = true;
					send_tick = osKernelGetTickCount();
				}
			}

		}

    osDelay(1);
  }
  /* USER CODE END StartDataTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void WSLogo(void)
{
	SetLeds(leds_sta);
	if(osKernelGetTickCount() >= tick + 500)
	{
		leds_sta = ~leds_sta;
		tick = osKernelGetTickCount();
	}	
	
	if(0 == intick)
		intick = osKernelGetTickCount();
	else if(osKernelGetTickCount() >= intick + 8000)
  {
		g_ws = WS_GUI1;
		SetLeds(0x00);			
		BeepWork(500,1);
	}
}

void DrawLogo(void)
{
	if(0 == intick)
		intick = osKernelGetTickCount();
	else
	{
		if(osKernelGetTickCount() <=intick + 2000)
		{
			GUI_Clear();
			GUI_SetFont(&GUI_FontHZ_KaiTi_16);
			GUI_DispStringAt("专业实践",32,5);
			GUI_DispStringAt("综合设计II",20,25);
			GUI_DispStringAt("防火、防盗监测器",0,43);
			GUI_Update();
		}
		else if(osKernelGetTickCount() <= intick + 5000)
		{
			GUI_Clear();
			GUI_SetFont(&GUI_FontHZ_KaiTi_16);
			GUI_DispStringAt("成员1:汤宁标\n      18205223",0,0);
			GUI_DispStringAt("成员2:王宇凡\n      18205225",0,32);
			GUI_Update();
		}
		else if(osKernelGetTickCount() <= intick + 8000)
		{
			GUI_Clear();
			GUI_DrawBitmap(&bmTNB,0,0);
			GUI_DrawBitmap(&bmWYF,65,0);
			GUI_Update();
		}
		else 
			intick = 0;
	}
}

void DrawGUI1(void)//菜单主界面
{
	GUI_Clear();
	GUI_SetFont(&GUI_FontHZ_SimSun_12);
	GUI_SetColor(GUI_COLOR_BLACK);
	GUI_DispStringAt("实时监测",0,0);
	GUI_SetColor(GUI_COLOR_WHITE);
	GUI_DispStringAt("数据曲线",0,13);
	GUI_DispStringAt("无线通信",0,26);
	GUI_DispStringAt("参数设置",0,39);
	GUI_DispStringAt("K1︽ K2《 K3》 K4︾",0,52);

	GUI_DrawHLine(52,0,128);
	GUI_DrawVLine(48,0,52);
	
	char buf[20];
	
	if(0 == pageidx)
	{

		GUI_DispStringAt("当前温度：",50, 0);
		GUI_DispStringAt("震动报警：",50, 26);
		sprintf(buf,"%.1f℃",temp);
		GUI_DispStringAt(buf, 90, 13);
		GUI_DispStringAt(mpuwarn ? "是":"否", 90, 39);		
	}
	else if(1 == pageidx)
	{
		sprintf(buf,"ax:%6d",ax);
		GUI_DispStringAt(buf, 50, 0);	
		if(ax > 0)
			GUI_FillRect(70, 13, 70 + ax * 55 / 32768, 16);
		else if(ax < 0)
			GUI_DrawRect(70, 13, 70 - ax * 55 / 32768, 16);
		
		sprintf(buf,"ay:%6d",ay);
		GUI_DispStringAt(buf, 50, 17);			
		if(ay > 0)
			GUI_FillRect(70, 30, 70 + ay * 55 / 32768, 33);
		else if(ay < 0)
			GUI_DrawRect(70, 30, 70 - ay * 55 / 32768, 33);
		
		sprintf(buf,"az:%6d",az);
		GUI_DispStringAt(buf, 50, 34);			
		if(az > 0)
			GUI_FillRect(70, 47, 70 + az * 55 / 32768, 50);
		else if(az < 0)
			GUI_DrawRect(70, 47, 70 - az * 55 / 32768, 50);		
	}
	else if(2 == pageidx)
	{
		sprintf(buf,"gx:%6d",gx);
		GUI_DispStringAt(buf, 50, 0);	
		if(gx > 0)
			GUI_FillRect(70, 13, 70 + gx * 55 / 32768, 16);
		else if(gx < 0)
			GUI_DrawRect(70, 13, 70 - gx * 55 / 32768, 16);
		
		sprintf(buf,"ay:%6d",gy);
		GUI_DispStringAt(buf, 50, 17);			
		if(gy > 0)
			GUI_FillRect(70, 30, 70 + gy * 55 / 32768, 33);
		else if(gy < 0)
			GUI_DrawRect(70, 30, 70 - gy * 55 / 32768, 33);
		
		sprintf(buf,"gz:%6d",gz);
		GUI_DispStringAt(buf, 50, 34);			
		if(gz > 0)
			GUI_FillRect(70, 47, 70 + gz * 55 / 32768, 50);
		else if(gz < 0)
			GUI_DrawRect(70, 47, 70 - gz * 55 / 32768, 50);				
	}
	GUI_Update();
}

void DrawGUI2(void)//菜单主界面
{
	char buf1[20];
	char buf2[20];
	int i;
	
	GUI_Clear();
	GUI_SetFont(&GUI_FontHZ_SimSun_12);	
	GUI_DispStringAt("实时监测",0,0);
	GUI_SetColor(GUI_COLOR_BLACK);
	GUI_DispStringAt("数据曲线",0,13);
	GUI_SetColor(GUI_COLOR_WHITE);	
	GUI_DispStringAt("无线通信",0,26);
	GUI_DispStringAt("参数设置",0,39);
	GUI_DispStringAt("K1︽ K2《 K3》 K4︾",0,52);

	GUI_DrawHLine(52,0,128);
	GUI_DrawVLine(48,0,52);
	
	int sw = 128 - 48;
	int sh = 64 - 12 -12;
	int ox = 48;
	int oy = 12 + sh;
	
	switch(pageidx)
	{
		default:
			break;
		
		case 0:
		{
			float tempMin = 25;
			float tempMax = 35;
			
			int tt = (((int)(temp) + 2) / 5) * 5;
			tempMin = tt - 5;
			tempMax = tt + 5; 
			
			float dh = sh / (tempMax-tempMin);
			sprintf(buf1,"温度:%.1f℃",temp);
			GUI_DispStringAt(buf1,50,0);
			
			if(tempLmt >= tempMin && tempLmt <= tempMax)
			{
				for(i = 0; i < MAX_DATALEN; i += 6)
					GUI_DrawHLine(oy - (tempLmt - tempMin) * dh, ox + i, ox + i + 3);				
			}
			
			for(i = 0; i < cTemp && i< MAX_DATALEN; ++i)
				GUI_DrawLine(ox + i - 1, oy - (vTemp[i - 1] - tempMin) * dh, ox + i, oy - (vTemp[i] - tempMin) * dh);	
			sprintf(buf2,"%dms",(interval==50)?interval:2*interval);
			GUI_DispStringAt(buf2,90,39);
			
		}
			break;
		case 1:
		{
			float tempMin = -90;
			float tempMax =  90;		
			float dh = sh / (tempMax-tempMin);
			sprintf(buf1,"俯仰角:%.1f°",fAX);
			GUI_DispStringAt(buf1,50,0);
			GUI_DrawHLine(oy + tempMin * dh, ox, ox + sw);
			
			for(i = 0; i < cPitch && i< MAX_DATALEN; ++i)
				GUI_DrawLine(ox + i - 1, oy - (vPitch[i - 1] - tempMin) * dh, ox + i, oy - (vPitch[i] - tempMin) * dh);			
			sprintf(buf2,"%dms",(interval==50)?interval:2*interval);
			GUI_DispStringAt(buf2,90,39);			
		}
			break;	
		
		case 2:
		{
			float tempMin = -180;
			float tempMax =  180;		
			float dh = sh / (tempMax-tempMin);
			sprintf(buf1,"横滚角:%.1f°",fAY);
			GUI_DispStringAt(buf1,50,0);
			GUI_DrawHLine(oy + tempMin * dh, ox, ox + sw);
			
			for(i = 0; i < cRoll && i< MAX_DATALEN; ++i)
				GUI_DrawLine(ox + i - 1, oy - (vRoll[i - 1] - tempMin) * dh, ox + i, oy - (vRoll[i] - tempMin) * dh);
			sprintf(buf2,"%dms",(interval==50)?interval:2*interval);
			GUI_DispStringAt(buf2,90,39);			
		}
			break;
		
		case 3:
		{
			float tempMin = -180;
			float tempMax =  180;		
			float dh = sh / (tempMax-tempMin);
			sprintf(buf1,"航向角:%.1f°",fAZ);
			GUI_DispStringAt(buf1,50,0);
			GUI_DrawHLine(oy + tempMin * dh, ox, ox + sw);
			for(i = 0; i < cYaw && i< MAX_DATALEN; ++i)
				GUI_DrawLine(ox + i - 1, oy - (vYaw[i - 1] - tempMin) * dh, ox + i, oy - (vYaw[i] - tempMin) * dh);			
			sprintf(buf2,"%dms",(interval==50)?interval:2*interval);
			GUI_DispStringAt(buf2,90,39);					
		}
			break;
		
		case 4:
		{
			ox = (48 + 128) / 2;
			oy = (12 + 40) / 2 ;
			RateCube(fAY, -fAX, fAZ, GUI_COLOR_WHITE, ox, oy); 
			//printf("fAX:%f",fAX);
		
			RotatePic32X32(TAB_TANG, fAY, -fAX, fAZ, GUI_COLOR_WHITE, ox -16, oy - 16);			
		}
		break;		
	}
	

	GUI_Update();
}


void DrawGUI3(void)//菜单主界面
{
	char send_buf[50];
	char recv_buf[50];
	float tsend_data = 0;
	float trecv_data = 0;
	GUI_Clear();
	GUI_SetFont(&GUI_FontHZ_SimSun_12);
	GUI_DispStringAt("实时监测",0,0);
	GUI_DispStringAt("数据曲线",0,13);
	GUI_SetColor(GUI_COLOR_BLACK);		
	GUI_DispStringAt("无线通信",0,26);
	GUI_SetColor(GUI_COLOR_WHITE);
	GUI_DispStringAt("参数设置",0,39);
	GUI_DispStringAt("K1︽ K2《 K3》 K4︾",0,52);

	if(pageidx == 0)
	{
		GUI_DispStringAt((char*)(hc05.name),50,0);
		GUI_DispStringAt(HC05_IsConn() ? "已连接" : "未连接",50,16);
		GUI_DispStringAt(g_bUping ? "上传中" : "未上传",50,32);		
	}
	else if(pageidx == 1)
	{
		if(send_data < 1024)
		{
			tsend_data = send_data;
			sprintf(send_buf,"SEND:%.0fB",tsend_data);
		}
		else
		{
			tsend_data = BtoKB(send_data);
			sprintf(send_buf,"SEND:%.2fKB",tsend_data);
		}
		GUI_DispStringAt(send_buf,50,0);
		
		if(recv_data < 1024)
		{
			trecv_data = recv_data;
			sprintf(recv_buf,"RECV:%.0fB",trecv_data);
		}
		else
		{
			trecv_data = BtoKB(recv_data);
			sprintf(recv_buf,"RECV:%.2fKB",trecv_data);
		}		
		GUI_DispStringAt(recv_buf,50,13);
		
		GUI_DispStringAt("KEY5:CLEAN",50,39);
	}

	
	GUI_DrawHLine(52,0,128);
	GUI_DrawVLine(48,0,52);
	GUI_Update();
}

void DrawGUI4(void)//菜单主界面
{
	char buf[20];
	
	GUI_Clear();
	GUI_SetFont(&GUI_FontHZ_SimSun_12);

	GUI_DispStringAt("实时监测",0,0);
	GUI_DispStringAt("数据曲线",0,13);
	GUI_DispStringAt("无线通信",0,26);	
	GUI_SetColor(GUI_COLOR_BLACK);
	GUI_DispStringAt("参数设置",0,39);	
	GUI_SetColor(GUI_COLOR_WHITE);
	GUI_DispStringAt("K1︽ K2《 K3》 K4︾",0,52);
	
	GUI_DrawHLine(52,0,128);
	GUI_DrawVLine(48,0,52);
	
	GUI_DispStringAt("温度上限:",50,0);
	GUI_DispStringAt("震动灵敏度:",50,13);
	GUI_DispStringAt("报警时长:",50,26);	
	GUI_DispStringAt("上传间隔:",50,39);
	
	sprintf(buf,"%.0f℃",tempLmt);
	if(0 == g_paridx)
		GUI_SetColor(GUI_COLOR_BLACK);
	GUI_DispStringAt(buf,104,0);
	GUI_SetColor(GUI_COLOR_WHITE);

	sprintf(buf,"%d",g_mpustep);
	if(1 == g_paridx)
		GUI_SetColor(GUI_COLOR_BLACK);
	GUI_DispStringAt(buf,116,13);
	GUI_SetColor(GUI_COLOR_WHITE);	
	
	sprintf(buf,"%dS",g_warntime);
	if(2 == g_paridx)
		GUI_SetColor(GUI_COLOR_BLACK);
	GUI_DispStringAt(buf,104,26);
	GUI_SetColor(GUI_COLOR_WHITE);	

	sprintf(buf,"%.1fS",g_upstep / 1000.0f);
	if(3 == g_paridx)
		GUI_SetColor(GUI_COLOR_BLACK);
	GUI_DispStringAt(buf,104,39);
	GUI_SetColor(GUI_COLOR_WHITE);
	
	GUI_Update();
}

void BeepWork(int time,int tune)
{
	static uint16_t TAB[] = {494, 523, 588, 660, 698, 784, 880, 988};//音阶对应频率
	HAL_TIM_Base_Start(&htim3);
	
	if(tune >= 1 && tune <= 7)
	{
		int pre = 1000000 / TAB[tune];
		__HAL_TIM_SET_AUTORELOAD(&htim3, pre);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pre/2);
		
		beeptick = osKernelGetTickCount() + time;
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	}
}

void BeepDone(void)
{
	if(beeptick >0 && osKernelGetTickCount() >= beeptick)
	{
		beeptick = 0;
		HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
	}
}

void DispSeg(uint8_t num[4], uint8_t dot)
{
	for(int i=0;i<4;++i)
	{
		Write595(i,num[i],(dot==(i+1))?1:0);
		osDelay(1);
	}
}

void InitHC05(void)
{
	init_flag = true;
	uint32_t oldrecv_data = recv_data;
	if(HC05_IsOK())
	{	
		HC05_SetRole(0);
		HC05_GetName();
		printf("HC05 Name:%s\n",hc05.name);
		HC05_AtMode(0);
		osDelay(100);
		if(recv_data != oldrecv_data)
			recv_data = oldrecv_data;
		init_flag = false;
	}			
}

float BtoKB(float x)
{
	if(x < 1024)
		x = x;
	else if(x < 1024 * 1024)
		x /= 1024.0;
	return x;
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
