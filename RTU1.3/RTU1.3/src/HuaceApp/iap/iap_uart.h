/**********************************************************************
* $Id$		debug_frmwrk.h			2011-06-02
*//**
* @file		debug_frmwrk.h
* @brief	Contains some utilities that used for debugging through UART
* @version	1.0
* @date		02. June. 2011
* @author	NXP MCU SW Application Team
* 
* Copyright(C) 2011, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
**********************************************************************/
#ifndef __IAP_UART_H_
#define __IAP_UART_H_

#include "lpc177x_8x_uart.h"

#define IAP_UART_PORT	(LPC_UART_TypeDef *)LPC_UART0

void Send_in_Process(unsigned char ch1, unsigned char ch2, unsigned char ch);
void iap_uart_init(unsigned int baudrate);
void IAP_Change_BaudRate(unsigned int baudrate);       //XULIANG 2012-05-10//
unsigned short uGetChar(void);
unsigned short Time_Delay(void);
void SendString(unsigned char *buff, unsigned int num);
//zxf unsigned char uGetString(unsigned char ch1, unsigned char ch2, unsigned char *buff, unsigned char num);
unsigned char uGetString(unsigned char ch1, unsigned char* buffa, unsigned char num);//zxf



#endif /* __IAP_UART_H_ */

