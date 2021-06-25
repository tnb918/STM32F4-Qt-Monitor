/*******************************************************************************
* �ĵ�: comm.h
* ����: ��ع
*
* ����: STM32F4����2ͨ��DMA���տ����ж� + HC05����Դ�ļ�
*		
* ʱ�䣺2021/5/13
*
*	ת��ʹ��ʱ��ע������
*******************************************************************************/
#include "HC05.h"

volatile ST_HC05 hc05;		// ����ģ�����ݶ���
uint8_t rx2_dma_buff[MAX_RECV_LEN] = {0};
uint8_t recv2_buff[MAX_RECV_LEN] = {0};
uint32_t recv2_len = 0; 									// �������ݳ���

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
	hc05.bat = 0;				// ģ�鲻��ATģʽ
	hc05.bok = 0;				// ģ�鲻����
	hc05.brtn = 0;			// ģ��δӦ��AT����
	hc05.ntry = 0;			// ģ��û�д���AT����
	hc05.bconn = 0;			// ����δ����
	hc05.bready = 1;		// ģ�����
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

void HC05_AtMode(uint8_t mode)	// ������˳�ATģʽ
{
	if (mode != hc05.bat)
	{
		HAL_GPIO_WritePin(BT_EN_GPIO_Port, BT_EN_Pin, mode ? GPIO_PIN_SET : GPIO_PIN_RESET);
		if (0 == mode)	// �˳�ATģʽ����Ҫ�Ӹ�λ����
			USendCMD(hc05.puart, "AT+RESET\r\n");
		hc05.bat = mode;
	}
}

void HC05_AtCmd(char *cmd, char *rtn, uint32_t timeout, uint8_t ntry)		// ����Ҫ�·���ATָ��
{
	if (cmd)
	{
		HC05_AtMode(1);				// ����ATģʽ
		
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
			
			if (rtn)						// ��ҪӦ��
			{
				len = strlen(rtn);
				memcpy((void *)hc05.need_rtn, rtn, len);
				hc05.need_rtn[len] = '\0';
				hc05.rtn_len = len;
				hc05.bready = 0;		// ģ��æ���ȴ�Ӧ����
				hc05.brtn = 0;
			}
			else
			{
				memset((void *)hc05.need_rtn, 0, MAX_RECV_LEN);
				hc05.rtn_timeout = 0;
				hc05.rtn_len = 0;
				hc05.brtn = 1;		// ����ҪӦ��
			}
		}
	}
}

void HC05_Proc(void)	// �������ݴ���
{
	static uint32_t tick = 0;
	if (hc05.bat)			// ��ATģʽ
	{
		if (hc05.ntry > 0)	// ������Ҫ����
		{
			if (osKernelGetTickCount() >= tick + hc05.rtn_timeout)
			{
				tick = osKernelGetTickCount();
				printf("HC-05 --> %s", (char *)hc05.send_cmd);
				HAL_UART_Transmit(hc05.puart, (uint8_t *)hc05.send_cmd, hc05.cmd_len, 0xFFFF); 	// ����һ������
				-- hc05.ntry;
			}
		}
	}
	
	if (*hc05.plen > 0)	// �н�������
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
		
		if (hc05.bat && hc05.rtn_len > 0)	// ��ҪӦ��
		{
			if (strstr((char *)hc05.recv_data, (char *)hc05.need_rtn) != NULL)	// ����Ҫ��Ӧ��
			{
				hc05.ntry = 0;		// ����Ҫ�ٷ���������
				hc05.brtn = 1;		// ȷ��Ӧ��
				hc05.bready = 1;	// ģ�����
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

uint8_t HC05_IsOK(void)						// ģ���Ƿ�Ӧ��
{
	HC05_Wait();
	HC05_AtCmd("AT\r\n", "OK", 500, 3);	
	HC05_Wait();
	hc05.bok = hc05.brtn;
	return hc05.bok;
}

uint8_t HC05_SetRole(uint8_t role)	// ��������ģʽ
{
	HC05_Wait();
	
	if (role)
		HC05_AtCmd("AT+ROLE=1\r\n", "OK", 500, 3);	// ��ģʽ
	else
		HC05_AtCmd("AT+ROLE=0\r\n", "OK", 500, 3);	// ��ģʽ
	HC05_Wait();
	
	return hc05.brtn;
}

uint8_t HC05_SetName(char *name)	// ������������
{
	char buf[128];
	HC05_Wait();
	snprintf(buf, 128, "AT+NAME=\"%s\"\r\n", name);
	HC05_AtCmd(buf, "OK", 500, 3);
	HC05_Wait();
	
	return hc05.brtn;
}

uint8_t HC05_GetName(void)				// ��ѯ��������
{
	HC05_Wait();
	hc05.bfindname = 1;
	HC05_AtCmd("AT+NAME?\r\n", "OK", 500, 3);
	HC05_Wait();
	hc05.bfindname = 0;
	
	return hc05.brtn;
}

uint8_t HC05_GetAddr(void)				// ��ѯ����ģ���ַ
{
	HC05_Wait();
	HC05_AtCmd("AT+ADDR?\r\n", "OK", 500, 3);
	HC05_Wait();
	
	return hc05.brtn;
}

uint8_t HC05_SetBPS(int bps)			// ����������ͨ������
{
	char buf[128];
	HC05_Wait();
	snprintf(buf, 128, "AT+UART=%d,1,0\r\n", bps);
	HC05_AtCmd(buf, "OK", 500, 3);
	HC05_Wait();
	
	return hc05.brtn;
}

uint8_t HC05_IsConn(void)					// ��ѯ����ģ������״̬
{
	hc05.bconn = (HAL_GPIO_ReadPin(BT_STATE_GPIO_Port, BT_STATE_Pin) == GPIO_PIN_SET) ? 1 : 0;
	return hc05.bconn;
}

uint8_t HC05_SetPSWD(char *pswd)		// ���������������
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
		__HAL_UART_CLEAR_IDLEFLAG(huart);	// ����жϱ�־
		if (huart->Instance == USART2)		// ����Ǵ���2�Ŀ����ж�
		{
			HAL_UART_DMAStop(huart);		// ��ͣDMA��������
			recv2_len = MAX_RECV_LEN - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);	// ���㵱ǰ�������ݳ���
			if (recv2_len < MAX_RECV_LEN - 1 && recv2_len > 0)
			{
				memcpy(recv2_buff, rx2_dma_buff, recv2_len);	// ����dma�������ݵ��������ݻ���
				recv2_buff[recv2_len] = '\0';					// �ַ���ĩβ������
			}
			else
				recv2_len = 0;		// ��Ч����
				
			__HAL_UNLOCK(huart);										// ��������
			HAL_UART_Receive_DMA(huart, rx2_dma_buff, MAX_RECV_LEN);	// ��DMA���գ����ݴ���dma_buff�����С�
		}
	}
}

void StartRecvUart2(void)
{
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);        		//ʹ��idle�ж�
  HAL_UART_Receive_DMA(&huart2, rx2_dma_buff, MAX_RECV_LEN);	//��DMA���գ����ݴ���recv_buff�����С�
}
