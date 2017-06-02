#ifndef __BSP_UART_H
#define __BSP_UART_H

#define UART_DBG    0

#define UART_COM_CH             UART_0
#define UART_GPRS_CH            UART_1
#define UART_485_CH             UART_2
#define UART_BD_CH              UART_3

#define UART0_BPS               _115200Bps
#define UART1_BPS               _115200Bps
#define UART2_BPS               
#define UART3_BPS               

#define UART_PRINT_CH           LPC_UART0//���ڴ�ӡchannel Ϊ0��ʾ�ر�
#define UART_BUFF_SZ            255 //����FIFO�����ֽ�Ϊ�������ʴ˴����ֵΪ255

#define BSP_UART_DBG            0

#define UART_SAVE_DIR           (uint8_t)0//ֱ��д�뻺��
#define UART_SAVE_FIFO          (uint8_t)1//д��FIFO

#define UART_BUF_MAX            (2048 - 2)

typedef void (*pUartIsr)(void);

extern void UartSend(UART_ID_Type UartID, uint8_t const *buff, uint16_t ucLen);
extern void UartBspInit(UART_ID_Type UartID, BandRateDef enBaudRate);
//extern uint8_t UartGetPacket(UART_ID_Type UartID, uint8_t *const pucData, uint16_t *pusLen, stPackDef *pstPack);
extern void UartChangeBaudRate(UART_ID_Type UartID, BandRateDef enBaudRate);
extern void UartDisableCh(UART_ID_Type UartID);
extern void UartEnableCh(UART_ID_Type UartID);

/*
���ڴ������ݵ���Ϣ������GetBuffer()�������ж��������ڴ��ָ��,����Ϣ�����е����ݶ�ȡ��ɺ�,�������
UartFreeBuf()�����ͷ��ڴ�
*/
extern void UartFreeBuf(UART_ID_Type UartID, uint8_t *pucBuf);

#if BSP_UART_DBG
extern void UartTest(void);
#endif

#endif
