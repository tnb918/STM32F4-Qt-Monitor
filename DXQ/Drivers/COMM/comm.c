/*******************************************************************************
* �ĵ�: comm.h
* ����: ��ع
*
* ����: STM32F4����ͨ��DMA���տ����жϵײ�����Դ�ļ�
*		
* ʱ�䣺2021/5/13
*
*	ת��ʹ��ʱ��ע������
*******************************************************************************/
#include "comm.h"

uint8_t rx1_dma_buff[MAX_RECV_LEN] = {0};
uint8_t recv1_buff[MAX_RECV_LEN] = {0};
uint32_t recv1_len = 0; 									// �������ݳ���

extern DMA_HandleTypeDef hdma_usart1_rx;

int fputc(int ch, FILE *f) 
{ 
	// �򴮿� 1 ����һ���ַ� 
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100); 
	return 0;
}

void USendStr(UART_HandleTypeDef *UartHandle, uint8_t *str, uint32_t len)
{
	HAL_UART_Transmit(UartHandle, str, len, 0xFFFF); 
}

void USendCMD(UART_HandleTypeDef *UartHandle, char *cmd)
{
	printf("--> %s", cmd);
	osDelay(10);
	HAL_UART_Transmit(UartHandle, (uint8_t *)cmd, strlen(cmd), 0xFFFF); 
}

void Uart1IdleCallback(UART_HandleTypeDef *huart)
{
	if( __HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET)
  {
		__HAL_UART_CLEAR_IDLEFLAG(huart);		// ����жϱ�־
		if (huart->Instance == USART1)	// ����Ǵ���1�Ŀ����ж�
		{
			HAL_UART_DMAStop(huart);						// ��ͣDMA��������
			recv1_len = MAX_RECV_LEN - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);	// ���㵱ǰ�������ݳ���
			if (recv1_len < MAX_RECV_LEN - 1 && recv1_len > 0)
			{
				memcpy(recv1_buff, rx1_dma_buff, recv1_len);	// ����dma�������ݵ��������ݻ���
				recv1_buff[recv1_len] = '\0';							// �ַ���ĩβ������
			}
			else
				recv1_len = 0;		// ��Ч����
			__HAL_UNLOCK(huart);											// ��������
			HAL_UART_Receive_DMA(huart, rx1_dma_buff, MAX_RECV_LEN);	//��DMA���գ����ݴ���dma_buff�����С�
		}
  }
}

void StartRecvUart1(void)
{
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);        				//ʹ��idle�ж�
  HAL_UART_Receive_DMA(&huart1, rx1_dma_buff, MAX_RECV_LEN);	//��DMA���գ����ݴ���recv_buff�����С�
}
