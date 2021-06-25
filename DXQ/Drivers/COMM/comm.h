/*******************************************************************************
* 文档: comm.h
* 作者: 曾毓
*
* 描述: STM32F4串口通信DMA接收空闲中断底层驱动头文件
*		
* 时间：2021/5/13
*
*	转载使用时请注明出处
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
extern uint32_t recv1_len; 									// 接收数据长度

void USendStr(UART_HandleTypeDef *UartHandle, uint8_t *str, uint32_t len);
void USendCMD(UART_HandleTypeDef *UartHandle, char *cmd);
void StartRecvUart1(void);
void Uart1IdleCallback(UART_HandleTypeDef *huart);

#endif	// __COMM_H
