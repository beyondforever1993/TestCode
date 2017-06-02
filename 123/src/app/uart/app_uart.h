#ifndef __APP_UART_H_
#define __APP_UART_H_

#pragma pack(1)
typedef struct{
uint8_t  usr_baud;  //1:4800 2:9600 3:19200 4:57600 5:115200
}stComInfoDef;
#pragma pack()

extern void UartPrintBuffer(uint8_t *pData, uint16_t usDataLen);
#if TEST_EN
extern void UartDeal(uint8_t *pData, uint16_t usDataLen);
#endif

#endif /* __IAP_UART_H_ */

