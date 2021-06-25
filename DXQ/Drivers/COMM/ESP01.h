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
#ifndef __ESP8266_H
#define __ESP8266_H
#include "comm.h"

typedef struct {
	char send_cmd[MAX_RECV_LEN];			// 发送AT命令
	char need_rtn[MAX_RECV_LEN];			// 需要应答内容
	char recv_data[MAX_RECV_LEN];			// 接收数据
	char ap_addr[40];									// AP端IP地址
	char st_addr[40];									// station端IP地址
	char ssid[40];										// 模块名称
	char mac[20];											// 模块mac地址
	UART_HandleTypeDef *puart;				// 模块连接串口指针
	uint8_t *pbuff;										// 接收缓冲
	uint32_t *plen;										// 接收缓冲有效长度
	uint32_t rtn_timeout;							// 应答超时
	uint32_t cmd_len;									// 命令数据长度
	uint32_t cmd_tick;								// 发送命令时间戳
	uint32_t recv_len;								// 接收数据长度
	uint32_t rtn_len;									// 应答数据长度，0为不需要应答
	uint8_t ntry;											// 命令重复次数
	uint8_t nmode;										// 工作模式，1：STATION，2：AP，3：AP+STATION
	uint8_t bserver;									// 是否开启TCP服务器
	uint8_t btc;											// 是否在透传模式
	uint8_t bok;											// 设备是否存在
	uint8_t brtn;											// 是否应答AT命令
	uint8_t bconn;										// 是否已连接
	uint8_t bready;										// 是否空闲
} ST_ESP8266;

extern volatile ST_ESP8266 esp8266;	// wifi模块数据对象
extern uint8_t rx6_dma_buff[MAX_RECV_LEN];
extern uint8_t recv6_buff[MAX_RECV_LEN];
extern uint32_t recv6_len; 					// 接收数据长度

void ESP_Init(void);								// 初始化wifi模块
void ESP_Proc(void);								// 接收数据处理
uint8_t ESP_IsOK(void);							// 模块是否应答
uint8_t ESP_SetAP(char *ssid, char *pws, int ch, int wsk);	// 设置AP热点，ssid名称，pws密码，ch信道（1-11），wsk加密方式（无密码则为0）
uint8_t ESP_GetSSID(void);					// 查询ssid名称
uint8_t ESP_GetMAC(void);						// 查询模块mac地址
uint8_t ESP_SetMode(uint8_t mode);	// 设置模块工作方式，1：station，2：AP，3：AP+station
uint8_t ESP_SetCIPMux(uint8_t mode);// 设置连接模式，0：单连接，1：多连接
uint8_t ESP_SetTCPServer(uint8_t mode, uint16_t port);				// 设置TCP服务器，mode 0：关闭，1：打开，port：端口
void ESP_ServerSend(uint8_t cli, uint8_t *str, uint32_t len);	// 服务器向客户端发送数据
uint8_t ESP_JoinAP(char *apssid, char *pswd);		// 模块连接AP热点
uint8_t ESP_GetIPAddr(void);				// 查询模块IP地址
uint8_t ESP_ClientToServer(char *server, uint16_t port);			// 模块连接TCP服务器
uint8_t ESP_SetCIPMode(uint8_t mode);	// 设置透传模式，0：退出透传，1：进入透传
uint8_t ESP_CloseClient(void);				// 查询断开连接
void ESP8266IdleCallBack(UART_HandleTypeDef *huart);
void StartRecvUart6(void);

#endif	// __ESP8266_H