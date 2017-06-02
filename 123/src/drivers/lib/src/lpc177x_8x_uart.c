/**********************************************************************
* $Id$        lpc177x_8x_uart.c            2011-06-02
*//**
* @file        lpc177x_8x_uart.c
* @brief    Contains all functions support for UART firmware library
*            on LPC177x_8x
* @version    1.0
* @date        02. June. 2011
* @author    NXP MCU SW Application Team
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
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors'
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @addtogroup UART
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc177x_8x_uart.h"
#include "lpc177x_8x_clkpwr.h"

typedef struct {
uint16_t usDL;//���ucDiv > 0��usDL���� > 2 
uint8_t  ucDiv : 4;//���� >= 0�� <= 14
uint8_t  ucMul : 4;//���� >= 1�� <= 15
}stDivRegValDef;

const static stDivRegValDef astDivRegVal[] = {
    {//9600bps
        /*DLM/DLL*/  /*DIV*/    /*MUL*/
        0xd9,         4,          5,
    },
    {//19200bps
        /*DLM/DLL*/  /**/
        0x65,         0x0e,       0x0f,
    },
    {//38400bps
        0x47,         0x03,       0x08,
    },
    {//57600Bps
        0x2f,         0x05,       0x0d,
    },
    {//115200Bps
        0x13,         0x05,       0x07,
    },
};

/*
����: UartSetBaudRate()
����:
    1.���ô��ڲ�����
����:   
    1.UARTx: ���Ͷ˿�(LPC_UART0 etc.)����UART1�������˿ڵļĴ�����һ�£����ڴ˴���void *������
    2.buff:  ָ���ͻ����ָ��
    3.ucLen: Ҫ���͵����ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void UartSetBaudRate(void *UARTx, BandRateDef enBandRate)
{
    uint8_t ucFDR = (((uint8_t)(astDivRegVal[enBandRate].ucMul)) << 4) | astDivRegVal[enBandRate].ucDiv;
    if (UARTx != (void *)LPC_UART1)
    {
        ((LPC_UART_TypeDef *)UARTx)->LCR |= UART_LCR_DLAB_EN;
        
        (((LPC_UART_TypeDef *)UARTx)->DLL) = (astDivRegVal[enBandRate].usDL) & 0xff;
        (((LPC_UART_TypeDef *)UARTx)->DLM) = (astDivRegVal[enBandRate].usDL) >> 8;
        
        (((LPC_UART_TypeDef *)UARTx)->FDR) = ucFDR;
        
        (((LPC_UART_TypeDef *)UARTx)->LCR) &= ~UART_LCR_DLAB_EN;
    }
    else
    {
        LPC_UART1->LCR |= UART_LCR_DLAB_EN;
        
        LPC_UART1->DLL = (astDivRegVal[enBandRate].usDL) & 0xff;
        LPC_UART1->DLM = (astDivRegVal[enBandRate].usDL) >> 8;
        
        LPC_UART1->FDR = ucFDR;
        
        LPC_UART1->LCR &= ~UART_LCR_DLAB_EN;
    }
    return;
}
