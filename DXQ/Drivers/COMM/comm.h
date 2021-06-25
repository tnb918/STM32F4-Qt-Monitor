/*******************************************************************************
* �ĵ�: comm.h
* ����: ��ع
*
* ����: STM32F4����ͨ��DMA���տ����жϵײ�����ͷ�ļ�
*		
* ʱ�䣺2021/5/13
*
*	ת��ʹ��ʱ��ע������
*******************************************************************************/
#ifndef __COMM_H
#define __COMM_H
#include "stm32f4xx_hal.h"
#include "usart.h"

#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>

#define MAX_RECV_LEN 1024
extern uint8_t rx1_dma_buff[MAX_RECV_LEN];
extern uint8_t recv1_buff[MAX_RECV_LEN];
extern uint32_t recv1_len; 									// �������ݳ���

void USendStr(UART_HandleTypeDef *UartHandle, uint8_t *str, uint32_t len);
void USendCMD(UART_HandleTypeDef *UartHandle, char *cmd);
void StartRecvUart1(void);
void Uart1IdleCallback(UART_HandleTypeDef *huart);

#endif	// __COMM_H
