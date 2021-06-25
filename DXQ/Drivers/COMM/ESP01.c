/*******************************************************************************
* 文档: comm.h
* 作者: 曾毓
*
* 描述: STM32F4串口6通信DMA接收空闲中断 + ESP8266驱动头文件
*		
* 时间：2021/5/13
*
*	转载使用时请注明出处
*******************************************************************************/
#include "ESP01.h"

volatile ST_ESP8266 esp8266;		// 模块数据对象
uint8_t rx6_dma_buff[MAX_RECV_LEN] = {0};
uint8_t recv6_buff[MAX_RECV_LEN] = {0};
uint32_t recv6_len = 0; 									// 接收数据长度

extern DMA_HandleTypeDef hdma_usart6_rx;

void ESP_AtCmd(char *cmd, char *rtn, uint32_t timeout, uint8_t ntry);		// 设置要下发的AT指令

void ESP_Init(void)
{
	esp8266.puart = &huart6;
	esp8266.btc = 0;				// 模块不在透传模式
	esp8266.bok = 0;				// 模块不存在
	esp8266.brtn = 0;				// 模块未应答AT命令
	esp8266.ntry = 0;				// 模块没有待发AT命令
	esp8266.bconn = 0;			// 未连接
	esp8266.bready = 1;			// 模块空闲
	esp8266.bserver = 0;		// 服务器未开启
	esp8266.rtn_timeout = 0;
	esp8266.recv_len = 0;
	esp8266.pbuff = recv6_buff;
	esp8266.plen = &recv6_len;
	memset((void *)(esp8266.recv_data), 0, MAX_RECV_LEN);
	esp8266.recv_len = 0;
	memset((void *)(esp8266.send_cmd), 0, MAX_RECV_LEN);
	esp8266.cmd_len = 0;
	esp8266.cmd_tick = 0;
	memset((void *)(esp8266.need_rtn), 0, MAX_RECV_LEN);
	esp8266.rtn_len = 0;
	
	StartRecvUart6();
}

void ESP_AtCmd(char *cmd, char *rtn, uint32_t timeout, uint8_t ntry)		// 设置要下发的AT指令
{
	if (cmd)
	{
		int len = strlen(cmd);
		if (len > 0 && len < MAX_RECV_LEN)
		{
			memcpy((void *)(esp8266.send_cmd), (void *)cmd, len);
			esp8266.send_cmd[len] = '\0';
			esp8266.cmd_len = len;
			esp8266.cmd_tick = 0;
			esp8266.rtn_timeout = timeout;
			if (ntry > 1)
				esp8266.ntry = ntry;
			else
				esp8266.ntry = 1;
			
			if (rtn)						// 需要应答
			{
				len = strlen(rtn);
				memcpy((void *)esp8266.need_rtn, rtn, len);
				esp8266.need_rtn[len] = '\0';
				esp8266.rtn_len = len;
				esp8266.bready = 0;		// 模块忙，等待应答中
				esp8266.brtn = 0;
			}
			else
			{
				memset((void *)esp8266.need_rtn, 0, MAX_RECV_LEN);
				esp8266.rtn_timeout = 0;
				esp8266.rtn_len = 0;
				esp8266.brtn = 1;		// 不需要应答
			}
		}
	}
}

void ESP_Proc(void)	// 接收数据处理
{
	if (!esp8266.btc)			// 不在透传模式
	{
		if (esp8266.ntry > 0)	// 有命令要发送
		{
			if (osKernelGetTickCount() >= esp8266.cmd_tick + esp8266.rtn_timeout)
			{
				esp8266.cmd_tick = osKernelGetTickCount();
				printf("ESP --> %s", (char *)esp8266.send_cmd);
				HAL_UART_Transmit(esp8266.puart, (uint8_t *)esp8266.send_cmd, esp8266.cmd_len, 0xFFFF); 	// 发送一次命令
				-- esp8266.ntry;
			}
		}
	}
	
	if (*esp8266.plen > 0)	// 有接收数据
	{
		esp8266.recv_len = *esp8266.plen;
		if (esp8266.recv_len >= MAX_RECV_LEN)
			esp8266.recv_len = MAX_RECV_LEN - 1;
		memcpy((void *)esp8266.recv_data, esp8266.pbuff, esp8266.recv_len);
		esp8266.recv_data[esp8266.recv_len] = '\0';
		
		*esp8266.plen = 0;
//		if (esp8266.recv_data[esp8266.recv_len - 1] != '\r' && esp8266.recv_data[esp8266.recv_len - 1] != '\n')
//			printf("ESP <-- %s\n", (char *)esp8266.recv_data);
//		else
//			printf("ESP <-- %s", (char *)esp8266.recv_data);
		
		if (!esp8266.btc && esp8266.rtn_len > 0)	// 需要应答
		{
			char *tst = (char *)esp8266.recv_data;
			char *pFind = NULL;
			while (*tst && tst < esp8266.recv_data + MAX_RECV_LEN)
			{
				if (strstr(tst, (char *)esp8266.need_rtn) == tst)	// 有需要的应答
				{
					esp8266.ntry = 0;		// 不需要再发送命令了
					esp8266.brtn = 1;		// 确认应答
					esp8266.bready = 1;	// 模块空闲
				}
				
//				if (esp8266.bserver)
				{
					if (strstr(tst, "CONNECT") == tst + 2)	// 有客户端连接
						++ esp8266.bconn;
					else if (strstr(tst, "CLOSED") == tst + 2)	// 客户端断开了
					{
						if (esp8266.bconn > 0)
							-- esp8266.bconn;
					}
				}
				
				if (strstr(tst, "+CWSAP:") == tst)
				{
					strncpy((char *)esp8266.ssid, tst + 8, sizeof(esp8266.ssid));
					char *pb = (char *)esp8266.ssid;
					while (*pb != '"' && *pb)
						++pb;
					*pb = '\0';
				}
				else if (strstr(tst, "+CIPSTAMAC:") == tst)
				{
					tst += 12;
					for (uint8_t i = 0; i < 17; ++tst)
					{
						if (*tst == '"' || *tst == '\0')
						{
							esp8266.mac[i] = '\0';
							break;
						}
						if (*tst != ':')
							esp8266.mac[i++] = *tst;
					}
				}
				else if ((pFind = strstr(tst, "+CIFSR:APIP,")) >= tst)
				{
					pFind += 13;
					for (uint8_t i = 0; i < 15; ++pFind)
					{
						if (*pFind == '"' || *pFind == '\0')
						{
							esp8266.ap_addr[i] = '\0';
							break;
						}
						else
							esp8266.ap_addr[i++] = *pFind;
					}
				}
				else if ((pFind = strstr(tst, "+CIFSR:STAIP,")) >= tst)
				{
					pFind += 14;
					for (uint8_t i = 0; i < 15; ++pFind)
					{
						if (*pFind == '"' || *pFind == '\0')
						{
							esp8266.st_addr[i] = '\0';
							break;
						}
						else
							esp8266.st_addr[i++] = *pFind;
					}
				}
				
				for (; tst < esp8266.recv_data + MAX_RECV_LEN && *tst; ++tst)
				{
					if (*tst == '\r' || *tst == '\n')
					{
						++tst;
						for (; tst < esp8266.recv_data + MAX_RECV_LEN && *tst; ++tst)
						{
							if (*tst != '\r' && *tst != '\n')
								break;
						}
						break;
					}
				}
			}
		}
	}
}

void ESP_Wait(void)
{
	while (esp8266.ntry > 0 && !esp8266.btc)
		osDelay(100);
}

uint8_t ESP_IsOK(void)						// 模块是否应答
{
	ESP_Wait();
	ESP_AtCmd("AT\r\n", "OK", 500, 3);	
	ESP_Wait();
	esp8266.bok = esp8266.brtn;
	return esp8266.bok;
}
//AT+CWSAP="ESP_TEST","",11,0
uint8_t ESP_SetAP(char *ssid, char *pws, int ch, int wsk)			// 设置AP热点，ssid名称，pws密码，ch信道（1-11），wsk加密方式（无密码则为0）
{
	char buf[128];
	ESP_Wait();
	snprintf(buf, 128, "AT+CWSAP=\"%s\",\"%s\",%d,%d\r\n", ssid, pws, ch, wsk);
	ESP_AtCmd(buf, "OK", 500, 3);
	ESP_Wait();
	
	return esp8266.brtn;
}

uint8_t ESP_GetSSID(void)				// 查询模块SSID
{
	ESP_Wait();
	ESP_AtCmd("AT+CWSAP?\r\n", "OK", 500, 3);
	ESP_Wait();
	
	return esp8266.brtn;
}

uint8_t ESP_GetMAC(void)				// 查询模块mac地址
{
	ESP_Wait();
	ESP_AtCmd("AT+CIPSTAMAC?\r\n", "OK", 500, 3);
	ESP_Wait();
	
	return esp8266.brtn;
}

uint8_t ESP_SetMode(uint8_t mode)	// 设置模块工作方式，1：station，2：AP，3：AP+station
{
	char buf[128];
	ESP_Wait();
	snprintf(buf, 128, "AT+CWMODE=%d\r\n", mode);
	ESP_AtCmd(buf, "OK", 500, 3);
	ESP_Wait();
	
	if (esp8266.brtn)
		esp8266.nmode = mode;
	return esp8266.brtn;
}

uint8_t ESP_SetCIPMux(uint8_t mode)	// 设置连接模式，0：单连接，1：多连接
{
	char buf[128];
	ESP_Wait();
	snprintf(buf, 128, "AT+CIPMUX=%d\r\n", mode);
	ESP_AtCmd(buf, "OK", 500, 3);
	ESP_Wait();
	
	return esp8266.brtn;
}

uint8_t ESP_SetTCPServer(uint8_t mode, uint16_t port)	// 设置TCP服务器，mode 0：关闭，1：打开，port：端口
{
	char buf[128];
	ESP_Wait();
	if (0 == mode)
		ESP_AtCmd("AT+CIPSERVER=0\r\n", "OK", 500, 3);
	else
	{
		snprintf(buf, 128, "AT+CIPSERVER=%d,%d\r\n", mode, port);
		ESP_AtCmd(buf, "OK", 500, 3);
	}
	ESP_Wait();
	
	esp8266.bserver = mode ? 1 : 0;
	return esp8266.brtn;
}

void ESP_ServerSend(uint8_t cli, uint8_t *str, uint32_t len)	// 服务器向客户端发送数据
{
	if (esp8266.bconn)
	{
		if (!esp8266.btc)
		{
			char buf[128];
			ESP_Wait();
			snprintf(buf, 128, "AT+CIPSEND=%d,%d\r\n", cli, len);
			ESP_AtCmd(buf, "", 100, 1);
			ESP_Wait();
		}
		HAL_UART_Transmit(esp8266.puart, str, len, 0xFFFF); 	// 发送数据
	}
}

uint8_t ESP_JoinAP(char *apssid, char *pswd)		// 模块连接AP热点
{
	char buf[128];
	ESP_Wait();
	snprintf(buf, 128, "AT+CWJAP=\"%s\",\"%s\"\r\n", apssid, pswd);
	ESP_AtCmd(buf, "OK", 8000, 2);
	ESP_Wait();
	
	return esp8266.brtn;
}

uint8_t ESP_GetIPAddr(void)				// 查询模块IP地址
{
	ESP_Wait();
	ESP_AtCmd("AT+CIFSR\r\n", "OK", 1000, 3);
	ESP_Wait();
	
	printf("ESP ap_addr:%s\nESP stat_addr:%s\n", esp8266.ap_addr, esp8266.st_addr);
	return esp8266.brtn;
}

uint8_t ESP_ClientToServer(char *server, uint16_t port)		// 模块连接TCP服务器
{
	char buf[128];
	ESP_Wait();
	snprintf(buf, 128, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", server, port);
	ESP_AtCmd(buf, "OK", 500, 3);
	ESP_Wait();
	
	if (esp8266.brtn)
		esp8266.bconn = 1;
	else
		esp8266.bconn = 0;
	return esp8266.brtn;
}

uint8_t ESP_SetCIPMode(uint8_t mode)	// 设置透传模式，0：退出透传，1：进入透传
{
	char buf[128];
	ESP_Wait();
	if (mode)
	{
		snprintf(buf, 128, "AT+CIPMODE=%d\r\n", mode);
		ESP_AtCmd(buf, "OK", 500, 3);
		ESP_Wait();
		ESP_AtCmd("AT+CIPSEND\r\n", "OK", 500, 3);
		if (esp8266.brtn)
			esp8266.btc = 1;
		ESP_Wait();
	}
	else
	{
		esp8266.btc = 0;
		ESP_AtCmd("+++", "", 1000, 1);
		ESP_Wait();
		snprintf(buf, 128, "AT+CIPMODE=%d\r\n", mode);
		ESP_AtCmd(buf, "OK", 500, 3);
		ESP_Wait();
	}
	
	return esp8266.brtn;
}

uint8_t ESP_CloseClient(void)				// 查询断开连接
{
	ESP_Wait();
	ESP_AtCmd("AT+CIPCLOSE\r\n", "OK", 500, 3);
	ESP_Wait();
	
	return esp8266.brtn;
}

void ESP8266IdleCallBack(UART_HandleTypeDef *huart)
{
	if( __HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET)
	{
		__HAL_UART_CLEAR_IDLEFLAG(huart);	// 清除中断标志
		if (huart->Instance == USART6)		// 如果是串口6的空闲中断
		{
			HAL_UART_DMAStop(huart);		// 暂停DMA接收数据
			recv6_len = MAX_RECV_LEN - __HAL_DMA_GET_COUNTER(&hdma_usart6_rx);	// 计算当前接收数据长度
			if (recv6_len < MAX_RECV_LEN - 1 && recv6_len > 0)
			{
				if (1 == recv6_len && rx6_dma_buff[0] == 0)	// 无效数据
					recv6_len = 0;
				else
				{
					memcpy(recv6_buff, rx6_dma_buff, recv6_len);	// 复制dma缓冲数据到串口数据缓冲
					recv6_buff[recv6_len] = '\0';					// 字符串末尾结束符
				}
			}
			else
				recv6_len = 0;		// 无效数据
			__HAL_UNLOCK(huart);										// 解锁串口
			HAL_UART_Receive_DMA(huart, rx6_dma_buff, MAX_RECV_LEN);	// 打开DMA接收，数据存入dma_buff数组中。
		}
	}
}

void StartRecvUart6(void)
{
  __HAL_UART_ENABLE_IT(&huart6, UART_IT_IDLE);        		//使能idle中断
  HAL_UART_Receive_DMA(&huart6, rx6_dma_buff, MAX_RECV_LEN);	//打开DMA接收，数据存入recv_buff数组中。
}
