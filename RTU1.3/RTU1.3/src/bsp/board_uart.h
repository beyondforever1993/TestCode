/***********************************************************************//**
* @file		board_uart.h
* @purpose		This example describes how to use uart
* 			  	
* @version		1.0
* @date		18. September. 2010
* @author		NXP MCU SW Application Team
*---------------------------------------------------------------------
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
#ifndef __BOARD_UART_H_
#define __BOARD_UATT_H_

#include <integer.h>

extern BOOL CmcCommandEmpty_gps ;

void UART0_IRQHandler (void);
void UART1_IRQHandler (void);
void UART2_IRQHandler (void);
void UART3_IRQHandler (void);
void  BSP_SerInit (unsigned char id, unsigned int baud_rate);
void  BSP_SerSendData (unsigned char id, unsigned char *p, unsigned short length);
void  BSP_SerInitSem ();
int my_putchar(unsigned char ch);
#endif