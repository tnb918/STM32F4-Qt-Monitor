/*******************************************************************************
* �ĵ�: comm.h
* ����: ��ع
*
* ����: STM32F4����2ͨ��DMA���տ����ж� + HC05����ͷ�ļ�
*		
* ʱ�䣺2021/5/13
*
*	ת��ʹ��ʱ��ע������
*******************************************************************************/
#ifndef __HC05_H
#define __HC05_H
#include "comm.h"

typedef struct {
	char send_cmd[MAX_RECV_LEN];			// ����AT����
	char need_rtn[MAX_RECV_LEN];			// ��ҪӦ������
	char recv_data[MAX_RECV_LEN];			// ��������
	char name[40];										// ģ������
	char addr[20];										// ģ���ַ
	UART_HandleTypeDef *puart;				// ����ģ�����Ӵ���ָ��
	uint8_t *pbuff;										// ���ջ���
	uint32_t *plen;										// ���ջ�����Ч����
	uint32_t rtn_timeout;							// Ӧ��ʱ
	uint32_t cmd_len;									// �������ݳ���
	uint32_t recv_len;								// �������ݳ���
	uint32_t rtn_len;									// Ӧ�����ݳ��ȣ�0Ϊ����ҪӦ��
	uint8_t ntry;											// �����ظ�����
	uint8_t bat;											// �Ƿ���ATģʽ
	uint8_t bok;											// �豸�Ƿ����
	uint8_t brtn;											// �Ƿ�Ӧ��AT����
	uint8_t bconn;										// �Ƿ�������
	uint8_t bready;										// �Ƿ����
	uint8_t bfindname;								// �Ƿ��ѯ����
} ST_HC05;

extern volatile ST_HC05 hc05;				// ����ģ�����ݶ���
extern uint8_t rx2_dma_buff[MAX_RECV_LEN];
extern uint8_t recv2_buff[MAX_RECV_LEN];
extern uint32_t recv2_len; 									// �������ݳ���

void HC05_Init(void);								// ��ʼ������ģ��
void HC05_AtMode(uint8_t mode);			// ������˳�ATģʽ
void HC05_Proc(void);								// �������ݴ���
uint8_t HC05_IsOK(void);						// ģ���Ƿ�Ӧ��
uint8_t HC05_SetRole(uint8_t role);	// ��������ģʽ
uint8_t HC05_SetName(char *name);		// ������������
uint8_t HC05_GetName(void);					// ��ѯ��������
uint8_t HC05_GetAddr(void);					// ��ѯ����ģ���ַ
uint8_t HC05_SetBPS(int bps);				// ����������ͨ�����ʣ�Ĭ��38400bps
uint8_t HC05_IsConn(void);					// ��ѯ����ģ������״̬
uint8_t HC05_SetPSWD(char *pswd);		// ���������������
void HC05IdleCallBack(UART_HandleTypeDef *huart);
void StartRecvUart2(void);

#endif	// __HC05_H
