/**********************************************************************
* $Id$		iap_uart.c			2011-06-02
*//**
* @file		iap_uart.c
* @brief	Contains some utilities that used for debugging through UART
* @version	1.0
* @date		02. June. 2011
* @author	NXP MCU SW Application Team
* 
* Copyright(C) 2011, NXP Semiconductor
* All rights reserved.
*
#include <LPC177x_8x.H>
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
#include "bsp.h"
#include "lpc177x_8x_timer.h"
#include "iap_uart.h"
//#include "led.h"
#include "lpc177x_8x_pinsel.h"
#include "string.h"

//unsigned char *ReceiverID = "1234567890";
//extern unsigned char ReceiverID[];
//extern volatile uint8_t packet_end_flag;
//extern volatile uint8_t update_timeout_flag;	   //XULIANG 2011-11-11
/*********************************************************************//**
 * @brief		Initialize IAP UART port
 * @param[in]	None
 * @return		None
 **********************************************************************/
void iap_uart_init(unsigned int baudrate)
{
   	UART_CFG_Type UARTConfigStruct;
	/*
	 * Initialize UART0 pin connect
	 * P0.2: TXD
	 * P0.3: RXD
	 */
	PINSEL_ConfigPin (0, 2, 1);
	PINSEL_ConfigPin (0, 3, 1);

	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);
	// Re-configure baudrate to 115200bps
	UARTConfigStruct.Baud_rate = baudrate;//9600;//115200;

	// Initialize IAP_UART_PORT peripheral with given to corresponding parameter
	UART_Init(IAP_UART_PORT, &UARTConfigStruct);

	// Enable UART Transmit
	UART_TxCmd(IAP_UART_PORT, ENABLE);
}
/*************** //XULIANG 2012-05-10//******************************/
void IAP_Change_BaudRate(unsigned int baudrate)
{
   UART_Change_BaudRate(IAP_UART_PORT,  baudrate);
}
/******************************************************************************/
//zxf
unsigned char uGetString(unsigned char ch1, unsigned char* buffa, unsigned char num)
{
	unsigned char i, ch,j;
	unsigned int uRead;
	
	i = 0;
	j = 0;
	while(1)
	{
		uRead = uGetChar();
		if(uRead == 0xFFFF) 
			break;

		ch = (unsigned char)uRead;
		if(i == 0 && ch == '@')
	 		buffa[i++] = ch;
		else if(i == 1 && ch == ch1) 
			buffa[i++] = ch;			 
		else if(i == 2 && ch == 255- ch1) 
			buffa[i++] = ch;
		else if(i > 2) buffa[i++] = ch;
		else
		{
			j++;
			i = 0;
			if(j >= 20)  
			  return 0; 
		}

		if(i >= num)
			break;
	}
	return i;				
}
/* //zxf
unsigned char uGetString(unsigned char ch1, unsigned char ch2, unsigned char *buff, unsigned char num)
{
    unsigned char i,ch;
	unsigned short uRead;
    unsigned char j; //edit 2012.10.24
	i = 0;
	j = 0;	 //edit 2012.10.24
	while(1)
	{
	   uRead = uGetChar();
	   if(0xFFFF == uRead) break;
	   ch = (unsigned char)uRead;
	   if(0 == i && '$' == ch) 
	   		buff[i++] = ch;
	   else if(1 == i && '$' == ch) 
	   		buff[i++] = ch;
	   else if(2 == i && ch1 == ch)
	   	 	buff[i++] = ch;
	   else if(3 == i)
	   {
	      if(ch == ch2) 
		  		buff[i++] = ch;
		  else 
		  		return 0;
	   }
	   else if(i > 3) 
	   		buff[i++] = ch;
	   else  
	   		i = 0;
	   if(i >= num) 
	   		break;
	   else
	   { 
	     //edit 2012.10.24
	     if(0 == i && '$' != ch)
		 {
		   	 j++;  
			 if(j > 100) 
			 {
			    j = 0;
			 	break; 

			 }
		 }
	   }
	  
	 }
	 return i;
}
*/

/******************************************************************************************/
//zxf
unsigned short uGetChar(void)
{
 // g_DeviceBT.Buf[g_DeviceBT.WrSp]
  char c;
  
  if(g_DeviceBT.WrSp != g_DeviceBT.RdSp)
  {
    c = g_DeviceBT.Buf[g_DeviceBT.RdSp];
    INCREASE_POINTER(g_DeviceBT.RdSp);
      return (unsigned short)c;
  }
  else
    return 0xffff;
  
    
}
 
                
                
             /*   
unsigned short uGetChar(void)
{
    unsigned short uReturn;
	unsigned short uCounter = 0;
	
	LPC_TIM2->TCR |= (1<<1); //Reset Counter
	LPC_TIM2->TCR &= ~(1<<1); //release reset  
	// To start timer
	TIM_Cmd(BRD_TIMER_USED, ENABLE);
	///ClearDataLed();
	uCounter = 0;
	while(1)
	{
	    if(LPC_UART0->LSR & UART_LSR_RDR )	/// Receive Data Ready 
		{
		    ///SetDataLed();
			uCounter = 0;
			uReturn = LPC_UART0->RBR;
			return uReturn;
		}
		if(uCounter >= 2)
		{
		    uReturn = 0xFFFF;
			//packet_end_flag = 1;
			//update_timeout_flag = 1;       //XULIANG 2011-11-11
			return uReturn;
		}
	    if (TIM_GetIntStatus(BRD_TIMER_USED, TIM_MR0_INT)== SET)
	    {
		    uCounter++;
			TIM_ClearIntPending(BRD_TIMER_USED, TIM_MR0_INT);
	     }
	}
}
*/
/*********************************************************************************************/
unsigned short Time_Delay(void)
{
    unsigned short uReturn;
	unsigned short uCounter = 0;
	
	LPC_TIM2->TCR |= (1<<1); //Reset Counter
	LPC_TIM2->TCR &= ~(1<<1); //release reset  
	// To start timer
	TIM_Cmd(BRD_TIMER_USED, ENABLE);
	uCounter = 0;
	while(1)
	{
		if(uCounter >= 2)
		{
		    uReturn = 0xFFFF;
			//packet_end_flag = 1;
			//update_timeout_flag = 1;       //XULIANG 2011-11-11
			return uReturn;
		}
	    if (TIM_GetIntStatus(BRD_TIMER_USED, TIM_MR0_INT)== SET)
	    {
		    uCounter++;
			TIM_ClearIntPending(BRD_TIMER_USED, TIM_MR0_INT);
	     }
	}
}
/*********************************************************************************************/
void Send_in_Process(unsigned char ch1, unsigned char ch2, unsigned char len)
{
    unsigned char buff[42];
	unsigned char checksum;
	register unsigned char i,n,nReceiverID;

   	uint8_t* pReceiverTypeIDProdData=NULL;	//add by hefaqiang 20130710

	buff[0] = '$';
	buff[1] = '$';
	buff[2] = ch1;
	buff[3] = ch2;
	buff[4] = 0x00;
	buff[5] = 0x01;
	n = 6;
	if(('S' == ch1) && ('S' == ch2))
	{
/***********************************Add by  hefaqiang******************************/
		//nReceiverID=sizeof(HW_ConfPacket->ReceiverType)+sizeof(HW_ConfPacket->ReceiverID)+sizeof(HW_ConfPacket->ProductDate);
		//pReceiverTypeIDProdData=(uint8_t*)&HW_ConfPacket->ReceiverType;
		//for(i=0;i<nReceiverID;i++)
		//		{
		//			if(i==2)
		//				buff[n++]='S';	  //协议要求要修改这个变量,通知上位机更改波特率  
		//			else
		//				buff[n++]=pReceiverTypeIDProdData[i];
		//		}
 /***********************************End Add by  hefaqiang******************************/
/*
	 for(i = 0; i < strlen(ReceiverID); i++)
	   {
	       buff[n++] = ReceiverID[i];
	   }
*/	   
	   buff[5] = n - 6;
	}
	else
	{
	   buff[n++] = len;
	}
	checksum = 0;
	for(i = 0; i < n; i++) 
			checksum ^= buff[i];
	buff[n++] = checksum;
	buff[n++] = '\r';
	buff[n++] = '\n';
	SendString(buff, n);
}
/**************************************************************************/
void SendString(unsigned char *buff, unsigned int num)
{
    unsigned int i;
	for(i = 0; i < num; i++)
	{
       while( !(LPC_UART0->LSR & UART_LSR_THRE) );	//While THR is NOT empty, wait!
	   LPC_UART0->THR = buff[i];			//Load new data	 
	  // if(1 == update_timeout_flag)
	  // {SetSateLed();}
	}
	//if(1 == update_timeout_flag)
	//{ClearSateLed();}
}
