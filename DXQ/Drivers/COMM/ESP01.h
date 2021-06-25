/*******************************************************************************
* �ĵ�: comm.h
* ����: ��ع
*
* ����: STM32F4����6ͨ��DMA���տ����ж� + ESP8266����ͷ�ļ�
*		
* ʱ�䣺2021/5/13
*
*	ת��ʹ��ʱ��ע������
*******************************************************************************/
#ifndef __ESP8266_H
#define __ESP8266_H
#include "comm.h"

typedef struct {
	char send_cmd[MAX_RECV_LEN];			// ����AT����
	char need_rtn[MAX_RECV_LEN];			// ��ҪӦ������
	char recv_data[MAX_RECV_LEN];			// ��������
	char ap_addr[40];									// AP��IP��ַ
	char st_addr[40];									// station��IP��ַ
	char ssid[40];										// ģ������
	char mac[20];											// ģ��mac��ַ
	UART_HandleTypeDef *puart;				// ģ�����Ӵ���ָ��
	uint8_t *pbuff;										// ���ջ���
	uint32_t *plen;										// ���ջ�����Ч����
	uint32_t rtn_timeout;							// Ӧ��ʱ
	uint32_t cmd_len;									// �������ݳ���
	uint32_t cmd_tick;								// ��������ʱ���
	uint32_t recv_len;								// �������ݳ���
	uint32_t rtn_len;									// Ӧ�����ݳ��ȣ�0Ϊ����ҪӦ��
	uint8_t ntry;											// �����ظ�����
	uint8_t nmode;										// ����ģʽ��1��STATION��2��AP��3��AP+STATION
	uint8_t bserver;									// �Ƿ���TCP������
	uint8_t btc;											// �Ƿ���͸��ģʽ
	uint8_t bok;											// �豸�Ƿ����
	uint8_t brtn;											// �Ƿ�Ӧ��AT����
	uint8_t bconn;										// �Ƿ�������
	uint8_t bready;										// �Ƿ����
} ST_ESP8266;

extern volatile ST_ESP8266 esp8266;	// wifiģ�����ݶ���
extern uint8_t rx6_dma_buff[MAX_RECV_LEN];
extern uint8_t recv6_buff[MAX_RECV_LEN];
extern uint32_t recv6_len; 					// �������ݳ���

void ESP_Init(void);								// ��ʼ��wifiģ��
void ESP_Proc(void);								// �������ݴ���
uint8_t ESP_IsOK(void);							// ģ���Ƿ�Ӧ��
uint8_t ESP_SetAP(char *ssid, char *pws, int ch, int wsk);	// ����AP�ȵ㣬ssid���ƣ�pws���룬ch�ŵ���1-11����wsk���ܷ�ʽ����������Ϊ0��
uint8_t ESP_GetSSID(void);					// ��ѯssid����
uint8_t ESP_GetMAC(void);						// ��ѯģ��mac��ַ
uint8_t ESP_SetMode(uint8_t mode);	// ����ģ�鹤����ʽ��1��station��2��AP��3��AP+station
uint8_t ESP_SetCIPMux(uint8_t mode);// ��������ģʽ��0�������ӣ�1��������
uint8_t ESP_SetTCPServer(uint8_t mode, uint16_t port);				// ����TCP��������mode 0���رգ�1���򿪣�port���˿�
void ESP_ServerSend(uint8_t cli, uint8_t *str, uint32_t len);	// ��������ͻ��˷�������
uint8_t ESP_JoinAP(char *apssid, char *pswd);		// ģ������AP�ȵ�
uint8_t ESP_GetIPAddr(void);				// ��ѯģ��IP��ַ
uint8_t ESP_ClientToServer(char *server, uint16_t port);			// ģ������TCP������
uint8_t ESP_SetCIPMode(uint8_t mode);	// ����͸��ģʽ��0���˳�͸����1������͸��
uint8_t ESP_CloseClient(void);				// ��ѯ�Ͽ�����
void ESP8266IdleCallBack(UART_HandleTypeDef *huart);
void StartRecvUart6(void);

#endif	// __ESP8266_H