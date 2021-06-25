/*******************************************************************************
* 文档: comm.h
* 作者: 曾毓
*
* 描述: STM32F4串口2通信DMA接收空闲中断 + HC05驱动头文件
*		
* 时间：2021/5/13
*
*	转载使用时请注明出处
*******************************************************************************/
#ifndef __HC05_H
#define __HC05_H
#include "comm.h"

typedef struct {
	char send_cmd[MAX_RECV_LEN];			// 发送AT命令
	char need_rtn[MAX_RECV_LEN];			// 需要应答内容
	char recv_data[MAX_RECV_LEN];			// 接收数据
	char name[40];										// 模块名称
	char addr[20];										// 模块地址
	UART_HandleTypeDef *puart;				// 蓝牙模块连接串口指针
	uint8_t *pbuff;										// 接收缓冲
	uint32_t *plen;										// 接收缓冲有效长度
	uint32_t rtn_timeout;							// 应答超时
	uint32_t cmd_len;									// 命令数据长度
	uint32_t recv_len;								// 接收数据长度
	uint32_t rtn_len;									// 应答数据长度，0为不需要应答
	uint8_t ntry;											// 命令重复次数
	uint8_t bat;											// 是否在AT模式
	uint8_t bok;											// 设备是否存在
	uint8_t brtn;											// 是否应答AT命令
	uint8_t bconn;										// 是否已连接
	uint8_t bready;										// 是否空闲
	uint8_t bfindname;								// 是否查询名称
} ST_HC05;

extern volatile ST_HC05 hc05;				// 蓝牙模块数据对象
extern uint8_t rx2_dma_buff[MAX_RECV_LEN];
extern uint8_t recv2_buff[MAX_RECV_LEN];
extern uint32_t recv2_len; 									// 接收数据长度

void HC05_Init(void);								// 初始化蓝牙模块
void HC05_AtMode(uint8_t mode);			// 进入或退出AT模式
void HC05_Proc(void);								// 接收数据处理
uint8_t HC05_IsOK(void);						// 模块是否应答
uint8_t HC05_SetRole(uint8_t role);	// 设置主从模式
uint8_t HC05_SetName(char *name);		// 设置蓝牙名称
uint8_t HC05_GetName(void);					// 查询蓝牙名称
uint8_t HC05_GetAddr(void);					// 查询蓝牙模块地址
uint8_t HC05_SetBPS(int bps);				// 设置蓝牙端通信速率，默认38400bps
uint8_t HC05_IsConn(void);					// 查询蓝牙模块连接状态
uint8_t HC05_SetPSWD(char *pswd);		// 设置蓝牙配对密码
void HC05IdleCallBack(UART_HandleTypeDef *huart);
void StartRecvUart2(void);

#endif	// __HC05_H
