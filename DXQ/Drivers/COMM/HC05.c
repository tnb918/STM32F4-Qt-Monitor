/*******************************************************************************
* 文档: comm.h
* 作者: 曾毓
*
* 描述: STM32F4串口2通信DMA接收空闲中断 + HC05驱动源文件
*		
* 时间：2021/5/13
*
*	转载使用时请注明出处
*******************************************************************************/
#include "HC05.h"

volatile ST_HC05 hc05;		// 蓝牙模块数据对象
uint8_t rx2_dma_buff[MAX_RECV_LEN] = {0};
uint8_t recv2_buff[MAX_RECV_LEN] = {0};
uint32_t recv2_len = 0; 									// 接收数据长度

extern DMA_HandleTypeDef hdma_usart2_rx;

void ClearTail(char *buf)
{
	if (buf)
	{
		int len = strlen(buf);
		if (len > 0)
		{
			char *pb = buf + len - 1;
			while (pb >= buf)
			{
				if (*pb == '\r' || *pb == '\n' || *pb == ' ' || *pb == '\t')
				{
					*pb = '\0';
					--pb;
				}
				else
					break;
			}
		}
	}
}

void HC05_Init(void)
{
	hc05.puart = &huart2;
	hc05.bat = 0;				// 模块不在AT模式
	hc05.bok = 0;				// 模块不存在
	hc05.brtn = 0;			// 模块未应答AT命令
	hc05.ntry = 0;			// 模块没有待发AT命令
	hc05.bconn = 0;			// 蓝牙未连接
	hc05.bready = 1;		// 模块空闲
	hc05.bfindname = 0;
	hc05.rtn_timeout = 0;
	hc05.recv_len = 0;
	hc05.pbuff = recv2_buff;
	hc05.plen = &recv2_len;
	memset((void *)(hc05.recv_data), 0, MAX_RECV_LEN);
	hc05.recv_len = 0;
	memset((void *)(hc05.send_cmd), 0, MAX_RECV_LEN);
	hc05.cmd_len = 0;
	memset((void *)(hc05.need_rtn), 0, MAX_RECV_LEN);
	hc05.rtn_len = 0;
	
	StartRecvUart2();
}

void HC05_AtMode(uint8_t mode)	// 进入或退出AT模式
{
	if (mode != hc05.bat)
	{
		HAL_GPIO_WritePin(BT_EN_GPIO_Port, BT_EN_Pin, mode ? GPIO_PIN_SET : GPIO_PIN_RESET);
		if (0 == mode)	// 退出AT模式，需要加复位命令
			USendCMD(hc05.puart, "AT+RESET\r\n");
		hc05.bat = mode;
	}
}

void HC05_AtCmd(char *cmd, char *rtn, uint32_t timeout, uint8_t ntry)		// 设置要下发的AT指令
{
	if (cmd)
	{
		HC05_AtMode(1);				// 进入AT模式
		
		int len = strlen(cmd);
		if (len > 0 && len < MAX_RECV_LEN)
		{
			memcpy((void *)(hc05.send_cmd), (void *)cmd, len);
			hc05.send_cmd[len] = '\0';
			hc05.cmd_len = len;
			hc05.rtn_timeout = timeout;
			if (ntry > 1)
				hc05.ntry = ntry;
			else
				hc05.ntry = 1;
			
			if (rtn)						// 需要应答
			{
				len = strlen(rtn);
				memcpy((void *)hc05.need_rtn, rtn, len);
				hc05.need_rtn[len] = '\0';
				hc05.rtn_len = len;
				hc05.bready = 0;		// 模块忙，等待应答中
				hc05.brtn = 0;
			}
			else
			{
				memset((void *)hc05.need_rtn, 0, MAX_RECV_LEN);
				hc05.rtn_timeout = 0;
				hc05.rtn_len = 0;
				hc05.brtn = 1;		// 不需要应答
			}
		}
	}
}

void HC05_Proc(void)	// 接收数据处理
{
	static uint32_t tick = 0;
	if (hc05.bat)			// 在AT模式
	{
		if (hc05.ntry > 0)	// 有命令要发送
		{
			if (osKernelGetTickCount() >= tick + hc05.rtn_timeout)
			{
				tick = osKernelGetTickCount();
				printf("HC-05 --> %s", (char *)hc05.send_cmd);
				HAL_UART_Transmit(hc05.puart, (uint8_t *)hc05.send_cmd, hc05.cmd_len, 0xFFFF); 	// 发送一次命令
				-- hc05.ntry;
			}
		}
	}
	
	if (*hc05.plen > 0)	// 有接收数据
	{
		hc05.recv_len = *hc05.plen;
		if (hc05.recv_len >= MAX_RECV_LEN)
			hc05.recv_len = MAX_RECV_LEN - 1;
		memcpy((void *)hc05.recv_data, hc05.pbuff, hc05.recv_len);
		hc05.recv_data[hc05.recv_len] = '\0';
		
		*hc05.plen = 0;
//		if (hc05.recv_data[hc05.recv_len - 1] != '\r' && hc05.recv_data[hc05.recv_len - 1] != '\n')
//			printf("HC-05 <-- %s\n", (char *)hc05.recv_data);
//		else
//			printf("HC-05 <-- %s", (char *)hc05.recv_data);
		
		if (hc05.bat && hc05.rtn_len > 0)	// 需要应答
		{
			if (strstr((char *)hc05.recv_data, (char *)hc05.need_rtn) != NULL)	// 有需要的应答
			{
				hc05.ntry = 0;		// 不需要再发送命令了
				hc05.brtn = 1;		// 确认应答
				hc05.bready = 1;	// 模块空闲
			}
			
			if (strstr((char *)hc05.recv_data, "+NAME:") == hc05.recv_data)
			{
				if (hc05.recv_data[6] != '\0')
				{
					strncpy((char *)hc05.name, (char *)(hc05.recv_data + 6), sizeof(hc05.name));
					hc05.bfindname = 0;
					ClearTail((char *)hc05.name);
				}
			}
			if (hc05.bfindname)
			{
				if (strstr((char *)hc05.recv_data, "+NAME:") == NULL &&
						!(hc05.recv_data[0] == 'O' && hc05.recv_data[1] == 'K') && hc05.recv_data[6] != '\0')
				{
					strncpy((char *)hc05.name, (char *)(hc05.recv_data), sizeof(hc05.name));
					hc05.bfindname = 0;
//					ClearTail((char *)hc05.name);
					char *pb = (char *)hc05.name;
					while (*pb != '\r' && *pb != '\n' && *pb)
						++pb;
					*pb = '\0';
				}
			}
			if (strstr((char *)hc05.recv_data, "+ADDR:") == hc05.recv_data)
			{
				strncpy((char *)hc05.addr, (char *)(hc05.recv_data + 6), sizeof(hc05.addr));
				ClearTail((char *)hc05.addr);
			}
		}
	}
}

void HC05_Wait(void)
{
	while (hc05.ntry > 0 && hc05.bat)
		osDelay(100);
}

uint8_t HC05_IsOK(void)						// 模块是否应答
{
	HC05_Wait();
	HC05_AtCmd("AT\r\n", "OK", 500, 3);	
	HC05_Wait();
	hc05.bok = hc05.brtn;
	return hc05.bok;
}

uint8_t HC05_SetRole(uint8_t role)	// 设置主从模式
{
	HC05_Wait();
	
	if (role)
		HC05_AtCmd("AT+ROLE=1\r\n", "OK", 500, 3);	// 主模式
	else
		HC05_AtCmd("AT+ROLE=0\r\n", "OK", 500, 3);	// 从模式
	HC05_Wait();
	
	return hc05.brtn;
}

uint8_t HC05_SetName(char *name)	// 设置蓝牙名称
{
	char buf[128];
	HC05_Wait();
	snprintf(buf, 128, "AT+NAME=\"%s\"\r\n", name);
	HC05_AtCmd(buf, "OK", 500, 3);
	HC05_Wait();
	
	return hc05.brtn;
}

uint8_t HC05_GetName(void)				// 查询蓝牙名称
{
	HC05_Wait();
	hc05.bfindname = 1;
	HC05_AtCmd("AT+NAME?\r\n", "OK", 500, 3);
	HC05_Wait();
	hc05.bfindname = 0;
	
	return hc05.brtn;
}

uint8_t HC05_GetAddr(void)				// 查询蓝牙模块地址
{
	HC05_Wait();
	HC05_AtCmd("AT+ADDR?\r\n", "OK", 500, 3);
	HC05_Wait();
	
	return hc05.brtn;
}

uint8_t HC05_SetBPS(int bps)			// 设置蓝牙端通信速率
{
	char buf[128];
	HC05_Wait();
	snprintf(buf, 128, "AT+UART=%d,1,0\r\n", bps);
	HC05_AtCmd(buf, "OK", 500, 3);
	HC05_Wait();
	
	return hc05.brtn;
}

uint8_t HC05_IsConn(void)					// 查询蓝牙模块连接状态
{
	hc05.bconn = (HAL_GPIO_ReadPin(BT_STATE_GPIO_Port, BT_STATE_Pin) == GPIO_PIN_SET) ? 1 : 0;
	return hc05.bconn;
}

uint8_t HC05_SetPSWD(char *pswd)		// 设置蓝牙配对密码
{
	char buf[128];
	HC05_Wait();
	snprintf(buf, 128, "AT+PSWD=\"%s\"\r\n", pswd);
	HC05_AtCmd(buf, "OK", 500, 3);
	HC05_Wait();
	
	return hc05.brtn;
}

void HC05IdleCallBack(UART_HandleTypeDef *huart)
{
	if( __HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET)
	{
		__HAL_UART_CLEAR_IDLEFLAG(huart);	// 清除中断标志
		if (huart->Instance == USART2)		// 如果是串口2的空闲中断
		{
			HAL_UART_DMAStop(huart);		// 暂停DMA接收数据
			recv2_len = MAX_RECV_LEN - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);	// 计算当前接收数据长度
			if (recv2_len < MAX_RECV_LEN - 1 && recv2_len > 0)
			{
				memcpy(recv2_buff, rx2_dma_buff, recv2_len);	// 复制dma缓冲数据到串口数据缓冲
				recv2_buff[recv2_len] = '\0';					// 字符串末尾结束符
			}
			else
				recv2_len = 0;		// 无效数据
				
			__HAL_UNLOCK(huart);										// 解锁串口
			HAL_UART_Receive_DMA(huart, rx2_dma_buff, MAX_RECV_LEN);	// 打开DMA接收，数据存入dma_buff数组中。
		}
	}
}

void StartRecvUart2(void)
{
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);        		//使能idle中断
  HAL_UART_Receive_DMA(&huart2, rx2_dma_buff, MAX_RECV_LEN);	//打开DMA接收，数据存入recv_buff数组中。
}
