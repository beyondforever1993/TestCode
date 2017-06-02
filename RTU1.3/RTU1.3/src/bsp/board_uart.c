/***********************************************************************//**
* @file		board_uart.c
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
#include "includes.h"

#define  BSP_SER_REG_U0_BASE_ADDR                ((CPU_INT32U)0x4000C000)
#define  BSP_SER_REG_U1_BASE_ADDR                ((CPU_INT32U)0x40010000)
#define  BSP_SER_REG_U2_BASE_ADDR                ((CPU_INT32U)0x40098000)
#define  BSP_SER_REG_U3_BASE_ADDR                ((CPU_INT32U)0x4009C000)
#define  BSP_SER_REG_U4_BASE_ADDR                ((CPU_INT32U)0x400AC000)
#define  BSP_SER_REG_U0_RBR              (*(CPU_REG32 *)(BSP_SER_REG_U0_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U0_THR              (*(CPU_REG32 *)(BSP_SER_REG_U0_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U0_DLL              (*(CPU_REG32 *)(BSP_SER_REG_U0_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U0_DLM              (*(CPU_REG32 *)(BSP_SER_REG_U0_BASE_ADDR + 0x0004))
#define  BSP_SER_REG_U0_IER              (*(CPU_REG32 *)(BSP_SER_REG_U0_BASE_ADDR + 0x0004))
#define  BSP_SER_REG_U0_IIR              (*(CPU_REG32 *)(BSP_SER_REG_U0_BASE_ADDR + 0x0008))
#define  BSP_SER_REG_U0_FCR              (*(CPU_REG32 *)(BSP_SER_REG_U0_BASE_ADDR + 0x0008))
#define  BSP_SER_REG_U0_LCR              (*(CPU_REG32 *)(BSP_SER_REG_U0_BASE_ADDR + 0x000C))
#define  BSP_SER_REG_U0_LSR              (*(CPU_REG32 *)(BSP_SER_REG_U0_BASE_ADDR + 0x0014))
#define  BSP_SER_REG_U0_FDR              (*(CPU_REG32 *)(BSP_SER_REG_U0_BASE_ADDR + 0x0028))
#define  BSP_SER_REG_U0_TER              (*(CPU_REG32 *)(BSP_SER_REG_U0_BASE_ADDR + 0x0030))

#define  BSP_SER_REG_U1_RBR              (*(CPU_REG32 *)(BSP_SER_REG_U1_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U1_THR              (*(CPU_REG32 *)(BSP_SER_REG_U1_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U1_DLL              (*(CPU_REG32 *)(BSP_SER_REG_U1_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U1_DLM              (*(CPU_REG32 *)(BSP_SER_REG_U1_BASE_ADDR + 0x0004))
#define  BSP_SER_REG_U1_IER              (*(CPU_REG32 *)(BSP_SER_REG_U1_BASE_ADDR + 0x0004))
#define  BSP_SER_REG_U1_IIR              (*(CPU_REG32 *)(BSP_SER_REG_U1_BASE_ADDR + 0x0008))
#define  BSP_SER_REG_U1_FCR              (*(CPU_REG32 *)(BSP_SER_REG_U1_BASE_ADDR + 0x0008))
#define  BSP_SER_REG_U1_LCR              (*(CPU_REG32 *)(BSP_SER_REG_U1_BASE_ADDR + 0x000C))
#define  BSP_SER_REG_U1_LSR              (*(CPU_REG32 *)(BSP_SER_REG_U1_BASE_ADDR + 0x0014))
#define  BSP_SER_REG_U1_FDR              (*(CPU_REG32 *)(BSP_SER_REG_U1_BASE_ADDR + 0x0028))
#define  BSP_SER_REG_U2_RBR              (*(CPU_REG32 *)(BSP_SER_REG_U2_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U2_THR              (*(CPU_REG32 *)(BSP_SER_REG_U2_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U2_DLL              (*(CPU_REG32 *)(BSP_SER_REG_U2_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U2_DLM              (*(CPU_REG32 *)(BSP_SER_REG_U2_BASE_ADDR + 0x0004))
#define  BSP_SER_REG_U2_IER              (*(CPU_REG32 *)(BSP_SER_REG_U2_BASE_ADDR + 0x0004))
#define  BSP_SER_REG_U2_IIR              (*(CPU_REG32 *)(BSP_SER_REG_U2_BASE_ADDR + 0x0008))
#define  BSP_SER_REG_U2_FCR              (*(CPU_REG32 *)(BSP_SER_REG_U2_BASE_ADDR + 0x0008))
#define  BSP_SER_REG_U2_LCR              (*(CPU_REG32 *)(BSP_SER_REG_U2_BASE_ADDR + 0x000C))
#define  BSP_SER_REG_U2_LSR              (*(CPU_REG32 *)(BSP_SER_REG_U2_BASE_ADDR + 0x0014))
#define  BSP_SER_REG_U2_FDR              (*(CPU_REG32 *)(BSP_SER_REG_U2_BASE_ADDR + 0x0028))
#define  BSP_SER_REG_U3_RBR              (*(CPU_REG32 *)(BSP_SER_REG_U3_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U3_THR              (*(CPU_REG32 *)(BSP_SER_REG_U3_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U3_DLL              (*(CPU_REG32 *)(BSP_SER_REG_U3_BASE_ADDR + 0x0000))
#define  BSP_SER_REG_U3_DLM              (*(CPU_REG32 *)(BSP_SER_REG_U3_BASE_ADDR + 0x0004))
#define  BSP_SER_REG_U3_IER              (*(CPU_REG32 *)(BSP_SER_REG_U3_BASE_ADDR + 0x0004))
#define  BSP_SER_REG_U3_IIR              (*(CPU_REG32 *)(BSP_SER_REG_U3_BASE_ADDR + 0x0008))
#define  BSP_SER_REG_U3_FCR              (*(CPU_REG32 *)(BSP_SER_REG_U3_BASE_ADDR + 0x0008))
#define  BSP_SER_REG_U3_LCR              (*(CPU_REG32 *)(BSP_SER_REG_U3_BASE_ADDR + 0x000C))
#define  BSP_SER_REG_U3_LSR              (*(CPU_REG32 *)(BSP_SER_REG_U3_BASE_ADDR + 0x0014))
#define  BSP_SER_REG_U3_FDR              (*(CPU_REG32 *)(BSP_SER_REG_U3_BASE_ADDR + 0x0028))
#define  BSP_SER_U0_PINS                 (DEF_BIT_02 | DEF_BIT_03)
#define  BSP_SER_U1_PINS                 (DEF_BIT_00 | DEF_BIT_01)//xf (DEF_BIT_15 | DEF_BIT_16)
#define  BSP_SER_REG_U4_IIR              (*(CPU_REG32 *)(BSP_SER_REG_U4_BASE_ADDR + 0x0008))
#define  BSP_SER_REG_U4_LSR              (*(CPU_REG32 *)(BSP_SER_REG_U4_BASE_ADDR + 0x0014))

///*
static  BSP_OS_SEM   BSP_Ser0TxWait;
static  BSP_OS_SEM   BSP_Ser0Lock;
//static  BSP_OS_SEM   BSP_Ser1TxWait;
static  BSP_OS_SEM   BSP_Ser1Lock;
//static  BSP_OS_SEM   BSP_Ser2TxWait;
static  BSP_OS_SEM   BSP_Ser2Lock;
//static  BSP_OS_SEM   BSP_Ser3TxWait;
static  BSP_OS_SEM   BSP_Ser3Lock;
//*/
#define BUFFNUM1 1024
//UART0 串口数据发送相关变量定义
unsigned char bSendBufferForReceiver[BUFFNUM1];
unsigned short wpSendToReceiverOut = 0;
unsigned short wpSendToReceiverIn = 0;
BOOL CmcCommandEmpty = TRUE;
//UART2 GPS数据发送相关变量定义//add by xxw 20140730
unsigned char bSendBufferForReceiver_gps[BUFFNUM1];
unsigned short wpSendToReceiverOut_gps = 0;
unsigned short wpSendToReceiverIn_gps = 0;
BOOL CmcCommandEmpty_gps = TRUE;
//UART3 BT数据发送相关变量定义
unsigned char bSendBufferForReceiver_bt[BUFFNUM1];
unsigned short wpSendToReceiverOut_bt = 0;
unsigned short wpSendToReceiverIn_bt = 0;
BOOL CmcCommandEmpty_bt = TRUE;
//UART4 RADIO数据发送相关变量定义
unsigned char bSendBufferForReceiver_radio[BUFFNUM1];
unsigned short wpSendToReceiverOut_radio = 0;
unsigned short wpSendToReceiverIn_radio = 0;
BOOL CmcCommandEmpty_radio = TRUE;

BOOL hard_enter = FALSE;
unsigned char task_current;

void hard_fault_handler_c(unsigned int * hardfault_args)
{
   while(1);
}

void UART0_IRQHandler (void)
{
  unsigned char  iir;
  //OS_CPU_SR  cpu_sr;
  //OS_ENTER_CRITICAL();
  //OSIntNesting++;
  //OS_EXIT_CRITICAL();
  iir = BSP_SER_REG_U0_IIR & 0x0F;
  while (iir != 1)
  {
    switch (iir)
    {
    case  2:   //发送中斷
      //BSP_OS_SemPost(&BSP_Ser0TxWait);
      if(wpSendToReceiverOut == wpSendToReceiverIn){

        CmcCommandEmpty = TRUE ;
        UART_IntConfig(LPC_UART0, UART_INTCFG_THRE, DISABLE);
      }
      else{
        LPC_UART0->THR = bSendBufferForReceiver[wpSendToReceiverOut];
        wpSendToReceiverOut++;
        wpSendToReceiverOut %= BUFFNUM1;
      }
      break;
    case  4:   //接收中斷                                                                      */
      g_DeviceCOM.Buf[g_DeviceCOM.WrSp] = BSP_SER_REG_U0_RBR;
      INCREASE_POINTER(g_DeviceCOM.WrSp);
      //while(tt>5)
      //  tt--;
      break;
    case  6:
      iir = BSP_SER_REG_U0_LSR;
      break;
    default:
      break;
    }
    iir = BSP_SER_REG_U0_IIR & 0x0F;
  }
  //OSIntExit();
}

void  UART1_IRQHandler (void)
{
  unsigned char  iir;
  iir = BSP_SER_REG_U1_IIR & 0x0F;
  while (iir != 1)
  {
    switch (iir)
    {
    case  2:   //发送中斷
      //BSP_OS_SemPost(&BSP_Ser1TxWait);
      break;

    case  4:   //接收中斷                                                                      */
      Module_Data_Buffer[Module_Data_WrSp] = BSP_SER_REG_U1_RBR; //edit by xhz 2011.8.31
      INCREASE_MOUDLE_DATA_POINTER(Module_Data_WrSp);
      break;
    case  6:
      iir = BSP_SER_REG_U1_LSR;
      break;
    default:
      break;
    }
    iir = BSP_SER_REG_U1_IIR & 0x0F;
  }
}

void  UART2_IRQHandler (void)
{
  unsigned char  iir;
  //unsigned char tmp;
  iir = BSP_SER_REG_U2_IIR & 0x0F;
  while (iir != 1)
  {
    switch (iir)
    {
    case  2:   //发送中斷
      //BSP_OS_SemPost(&BSP_Ser2TxWait);//add by xxw 20140730
      if(wpSendToReceiverOut_gps == wpSendToReceiverIn_gps)
      {
        CmcCommandEmpty_gps = TRUE ;
        UART_IntConfig(LPC_UART2, UART_INTCFG_THRE, DISABLE);
        //LPC_UART2->IER &= ((~UART_INTCFG_THRE) & UART_IER_BITMASK);
      }
      else
      {
        LPC_UART2->THR = bSendBufferForReceiver_gps[wpSendToReceiverOut_gps];
        wpSendToReceiverOut_gps++;
        wpSendToReceiverOut_gps %= BUFFNUM1;
      }
      break;
    case  4:   //接收中斷
      if(1)// (Get_Sourcelist_Flag == 0)
      {
        g_DeviceGPS.Buf[g_DeviceGPS.WrSp] = BSP_SER_REG_U2_RBR;
        INCREASE_POINTER(g_DeviceGPS.WrSp);
      }else{
        //tmp = BSP_SER_REG_U2_RBR;//delete by xxw 20140815 消除警告
      }
      break;
    case  6:
      iir = BSP_SER_REG_U2_LSR;
      break;
    default:
      break;
    }
    iir = BSP_SER_REG_U2_IIR & 0x0F;
  }
}
void  UART3_IRQHandler (void)
{
  unsigned char  iir;
  iir = BSP_SER_REG_U3_IIR & 0x0F;
  while (iir != 1)
  {
    switch (iir)
    {
    case  2:   //发送中斷
      //BSP_OS_SemPost(&BSP_Ser3TxWait);    //add by xxw 20140730
      if(wpSendToReceiverOut_bt == wpSendToReceiverIn_bt)
      {
        CmcCommandEmpty_bt = TRUE ;
        UART_IntConfig(LPC_UART3, UART_INTCFG_THRE, DISABLE);
        //LPC_UART3->IER &= ((~UART_INTCFG_THRE) & UART_IER_BITMASK);
      }
      else
      {
        LPC_UART3->THR = bSendBufferForReceiver_bt[wpSendToReceiverOut_bt];
        wpSendToReceiverOut_bt++;
        wpSendToReceiverOut_bt %= BUFFNUM1;
      }
      break;
    case  4:   //接收中斷
      //BTUARTRxDataBuf[BTUARTRxDataBufWritePos++] = BSP_SER_REG_U3_RBR;
      //BTUARTRxDataBufWritePos %= BTUARTMAXRXBUF;
      g_DeviceBT.Buf[g_DeviceBT.WrSp] = BSP_SER_REG_U3_RBR;
      INCREASE_POINTER(g_DeviceBT.WrSp);
      
      break;
    case  6:
      iir = BSP_SER_REG_U3_LSR;
      break;
    default:
      break;
    }
    iir = BSP_SER_REG_U3_IIR & 0x0F;
  }
}
/*******************2011-10-31 XULIANG******************/
void  UART4_IRQHandler (void)
{
  unsigned char  iir;
  iir = LPC_UART4->IIR & 0x0F;
  while (iir != 1)
  {
    //GPIO_OutputValue(BRD_LED_4_PORT, BRD_LED_4_MASK, LED_OFF);
    switch (iir)
    {
    case  2:   //发送中斷
      //BSP_OS_SemPost(&BSP_Ser0TxWait);
      if(wpSendToReceiverOut_radio == wpSendToReceiverIn_radio){

        CmcCommandEmpty_radio = TRUE ;
        UART4_IntConfig(LPC_UART4, UART_INTCFG_THRE, DISABLE);
      }
      else{
        LPC_UART4->THR = bSendBufferForReceiver_radio[wpSendToReceiverOut_radio];
        wpSendToReceiverOut_radio++;
        wpSendToReceiverOut_radio %= BUFFNUM1;
      }
      break;
    case  4:   //接收中斷
      Radio_Data_Process_Buffer[Radio_Data_Len] = LPC_UART4->RBR;
      INCREASE_RADIO_DATA_POINTER(Radio_Data_Len);
      break;
    case  6:
      iir = BSP_SER_REG_U4_LSR;
      break;
    default:
      break;
    }
    iir= LPC_UART4->IIR & 0x0F;
    //GPIO_OutputValue(BRD_LED_4_PORT, BRD_LED_4_MASK, LED_ON);
  }
}
/*******************2011-10-31 XULIANG******************/
void  BSP_SerInit (unsigned char id, unsigned int baud_rate) //xf
{
  unsigned short  div;                                            /* Baud rate divisor                                  */
  unsigned char  divlo;
  unsigned char  divhi;
  unsigned int  pclk_freq;

  UART_CFG_Type UARTConfigStruct;

  UART_FIFO_CFG_Type UARTFIFOConfigStruct;

  pclk_freq  = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);

  div       = (unsigned short)(((2 * pclk_freq / 16 / baud_rate) + 1) / 2);
  divlo     =  div & 0x00FF;                                  // Split divisor into LOW and HIGH bytes              */
  divhi     = (div >> 8) & 0x00FF;
  if(0 == id)
  {
    BSP_OS_SemWait(&BSP_Ser0Lock, 0);
    //OS_ENTER_CRITICAL();
    /* Set up clock and power for UART module */
    CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART0, ENABLE);
    /*
    * Initialize UART0 pin connect
    * P0.2: U0_TXD
    * P0.3: U0_RXD
    */
    PINSEL_ConfigPin(0,2,1);
    PINSEL_ConfigPin(0,3,1);

    UART_ConfigStructInit(&UARTConfigStruct);

    UARTConfigStruct.Baud_rate = baud_rate;

    // Initialize IAP_UART_PORT peripheral with given to corresponding parameter
    UART_Init(LPC_UART0, &UARTConfigStruct);

    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 peripheral
    UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct);

    UART_TxCmd(LPC_UART0, ENABLE);

    // Enable UART Rx interrupt //
    UART_IntConfig(LPC_UART0, UART_INTCFG_RBR, ENABLE);
    UART_IntConfig(LPC_UART0, UART_INTCFG_THRE, ENABLE);



    NVIC_SetPriority(UART0_IRQn, ((0x01<<4)|0x06)); // preemption = 1, sub-priority = 1
    NVIC_EnableIRQ(UART0_IRQn);                     // Enable Interrupt for UART0 channel


    BSP_OS_SemPost(&BSP_Ser0Lock);

  }
  else if(1 == id)
  {
    BSP_OS_SemWait(&BSP_Ser1Lock, 0);

    /* Set up clock and power for UART module */
    CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART1, ENABLE);
    /*
    * Initialize UART1 pin connect
    * P0.15: U1_TXD
    * P0.16: U1_RXD
    */
    PINSEL_ConfigPin(0,15,1);
    PINSEL_ConfigPin(0,16,1);
    // --------------------- SETUP UART1 ----------------------
    BSP_SER_REG_U1_LCR = DEF_BIT_07;                            // Set divisor access bit
    BSP_SER_REG_U1_DLL = divlo;                                 // Load divisor
    BSP_SER_REG_U1_DLM = divhi;
    BSP_SER_REG_U1_LCR = 0x03;                                  // 8 Bits, 1 Stop, No Parity
      BSP_SER_REG_U1_FCR = DEF_BIT_00;                            // Enable FIFO, flush Rx & Tx
    BSP_SER_REG_U1_IER = 0x01;                                  //使能接收中断，禁止发送中断
    BSP_SER_REG_U1_FDR = DEF_BIT_NONE;


    NVIC_SetPriority(UART1_IRQn, ((0x01<<4)|0x07)); // preemption = 1, sub-priority = 1
    NVIC_EnableIRQ(UART1_IRQn);                     // Enable Interrupt for UART1 channel

    BSP_OS_SemPost(&BSP_Ser1Lock);
  }
  else if(2 == id)
  {
    BSP_OS_SemWait(&BSP_Ser2Lock, 0);

    /* Set up clock and power for UART module */
    CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART2, ENABLE);
    /*
    * Initialize UART2 pin connect
    * P0.10: U2_TXD
    * P0.11: U2_RXD
    */
    PINSEL_ConfigPin(0,10,1);
    PINSEL_ConfigPin(0,11,1);
    // --------------------- SETUP UART2 ----------------------
    /*BSP_SER_REG_U2_LCR = DEF_BIT_07;                            // Set divisor access bit
    BSP_SER_REG_U2_DLL = divlo;                                 // Load divisor
    BSP_SER_REG_U2_DLM = divhi;
    BSP_SER_REG_U2_LCR = 0x03;                                  // 8 Bits, 1 Stop, No Parity
    BSP_SER_REG_U2_IER = 0x03;                                  //使能接收中断，禁止发送中断  //edit 2014.06.18 使能发送中断
    BSP_SER_REG_U2_FDR = DEF_BIT_NONE;
    BSP_SER_REG_U2_FCR = DEF_BIT_00;                            // Enable FIFO, flush Rx & Tx
    */
    UART_ConfigStructInit(&UARTConfigStruct);
    // Re-configure baudrate to 115200bps
    UARTConfigStruct.Baud_rate = baud_rate;//115200;

    // Initialize IAP_UART_PORT peripheral with given to corresponding parameter
    UART_Init(LPC_UART2, &UARTConfigStruct);

    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 peripheral
    UART_FIFOConfig(LPC_UART2, &UARTFIFOConfigStruct);

    // Enable UART Rx interrupt //
    UART_IntConfig(LPC_UART2, UART_INTCFG_RBR, ENABLE);
    UART_IntConfig(LPC_UART2, UART_INTCFG_THRE, ENABLE);
    // Enable UART line status interrupt //
    //UART_IntConfig(LPC_UART0, UART_INTCFG_RLS, ENABLE);
    // Enable UART Transmit
    UART_TxCmd(LPC_UART2, ENABLE);

    NVIC_SetPriority(UART2_IRQn, ((0x01<<4)|0x05)); // preemption = 1, sub-priority = 1
    NVIC_EnableIRQ(UART2_IRQn);                     // Enable Interrupt for UART2 channel

    BSP_OS_SemPost(&BSP_Ser2Lock);
  }
  else if(3 == id)
  {
    BSP_OS_SemWait(&BSP_Ser3Lock, 0);

    /* Set up clock and power for UART module */
    CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART3, ENABLE);
    /*
    * Initialize UART3 pin connect
    * P4.28: U3_TXD
    * P4.29: U3_RXD
    */
    PINSEL_ConfigPin(4,28,2);
    PINSEL_ConfigPin(4,29,2);
    /*                              //add by xxw 20140730                            // --------------------- SETUP UART3 ----------------------
    BSP_SER_REG_U3_LCR = DEF_BIT_07;                            // Set divisor access bit
    BSP_SER_REG_U3_DLL = divlo;                                 // Load divisor
    BSP_SER_REG_U3_DLM = divhi;
    BSP_SER_REG_U3_LCR = 0x03;                                  // 8 Bits, 1 Stop, No Parity
    BSP_SER_REG_U3_IER = 0x03;                                  //使能接收中断，禁止发送中断  //edit 2014.06.18 使能发送中断
    BSP_SER_REG_U3_FDR = DEF_BIT_NONE;
    BSP_SER_REG_U3_FCR = DEF_BIT_00;                            // Enable FIFO, flush Rx & Tx
    */
    UART_ConfigStructInit(&UARTConfigStruct);
    // Re-configure baudrate to 115200bps
    UARTConfigStruct.Baud_rate = baud_rate;//115200;

    // Initialize IAP_UART_PORT peripheral with given to corresponding parameter
    UART_Init(LPC_UART3, &UARTConfigStruct);

    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 peripheral
    UART_FIFOConfig(LPC_UART3, &UARTFIFOConfigStruct);

    // Enable UART Rx interrupt //
    UART_IntConfig(LPC_UART3, UART_INTCFG_RBR, ENABLE);
    UART_IntConfig(LPC_UART3, UART_INTCFG_THRE, ENABLE);
    // Enable UART line status interrupt //
    //UART_IntConfig(LPC_UART0, UART_INTCFG_RLS, ENABLE);
    // Enable UART Transmit
    UART_TxCmd(LPC_UART3, ENABLE);

    NVIC_SetPriority(UART3_IRQn, ((0x01<<4)|0x08)); // preemption = 1, sub-priority = 1
    NVIC_EnableIRQ(UART3_IRQn);                     // Enable Interrupt for UART3 channel

    BSP_OS_SemPost(&BSP_Ser3Lock);
  }
  /**************************2011-10-31 XULIANG*************************/
  else if(4 == id)
  {
    //BSP_OS_SemWait(&BSP_Ser3Lock, 0);

    /* Set up clock and power for UART module */
    CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART4, ENABLE);
    /*
    * Initialize UART4 pin connect
    * P5.4: U4_TXD
    * P5.3: U4_RXD
    */
    PINSEL_ConfigPin(5,4,4);
    PINSEL_ConfigPin(5,3,4);

    /* Initialize UART Configuration parameter structure to default state:
    * Baudrate = 9600bps
    * 8 data bit
    * 1 Stop bit
    * None parity
    */
    UART_ConfigStructInit(&UARTConfigStruct);
    // Re-configure baudrate to 115200bps
    UARTConfigStruct.Baud_rate = baud_rate;//115200;

    // Initialize IAP_UART_PORT peripheral with given to corresponding parameter
    UART4_Init(LPC_UART4, &UARTConfigStruct);

    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 peripheral
    //UART_FIFOConfig(LPC_UART4, &UARTFIFOConfigStruct);

    // Enable UART Rx interrupt //
    UART4_IntConfig(LPC_UART4, UART_INTCFG_RBR, ENABLE);
    // Enable UART line status interrupt //
    ///UART_IntConfig(LPC_UART0, UART_INTCFG_RLS, ENABLE);
    // Enable UART Transmit
    UART4_TxCmd(LPC_UART4, ENABLE);


    NVIC_SetPriority(UART4_IRQn, ((0x01<<3)|0x01)); // preemption = 1, sub-priority = 1
    NVIC_EnableIRQ(UART4_IRQn);                     // Enable Interrupt for UART0 channel
  }
  /*********************2011-10-31 XULIANG*******************/
}


void  BSP_SerInitSem ()
{  ///*
  BSP_OS_SemCreate(&BSP_Ser0TxWait, 0, "Serial0 Tx Wait");
  BSP_OS_SemCreate(&BSP_Ser0Lock  , 1, "Serial0 Lock");
  //BSP_OS_SemCreate(&BSP_Ser1TxWait, 0, "Serial1 Tx Wait");
  BSP_OS_SemCreate(&BSP_Ser1Lock  , 1, "Serial1 Lock");
  //BSP_OS_SemCreate(&BSP_Ser2TxWait, 0, "Serial2 Tx Wait");
  BSP_OS_SemCreate(&BSP_Ser2Lock  , 1, "Serial2 Lock");
  //BSP_OS_SemCreate(&BSP_Ser3TxWait, 0, "Serial3 Tx Wait");
  BSP_OS_SemCreate(&BSP_Ser3Lock  , 1, "Serial3 Lock");
  //*/
}

void  BSP_SerSendData (unsigned char id, unsigned char *p, unsigned short length)
{
  unsigned char lsr;
  unsigned short i;
  unsigned short nReadyForSend;
  unsigned int COUNT = 0;
  unsigned short nReadyForSend_gps;//add by xxw 20140730
  unsigned int COUNT_gps = 0;
  unsigned short nReadyForSend_bt;
  unsigned int COUNT_bt = 0;
  unsigned short nReadyForSend_radio;
  unsigned int COUNT_radio = 0;
  //OS_CPU_SR  cpu_sr = 0u;
  switch(id)
  {
  case 0:
      BSP_OS_SemWait(&BSP_Ser0Lock, 0);
      for(i = 0; i < length; i++)
      {
          //查询发送 
          while( !(LPC_UART0->LSR & UART_LSR_THRE) );	//While THR is NOT empty, wait!
          LPC_UART0->THR = *(p + i);			//Load new data	 
      }   
      BSP_OS_SemPost(&BSP_Ser0Lock);
    
    /*
    BSP_OS_SemWait(&BSP_Ser0Lock, 0);
    for(i = 0; i < length; i++)
    {
    BSP_SER_REG_U0_THR = *(p + i);
    DEF_BIT_SET(BSP_SER_REG_U0_IER, DEF_BIT_01);
    BSP_OS_SemWait(&BSP_Ser0TxWait, 0);
    DEF_BIT_CLR(BSP_SER_REG_U0_IER, DEF_BIT_01);
  }
    BSP_OS_SemPost(&BSP_Ser0Lock);
    */
    /* 
    BSP_OS_SemWait(&BSP_Ser0Lock, 0);


    //UART_IntConfig(LPC_UART0, UART_INTCFG_THRE, DISABLE);
    for(i = 0; i < length; i++)
    {
      //查询发送
      //while( !(LPC_UART0->LSR & UART_LSR_THRE) );	//While THR is NOT empty, wait!
      //LPC_UART0->THR = *(p + i);			//Load new data	
      while(1){
        if((COUNT ++) > 65535)
        {
          //BSP_SerInit(0,rate_current);
          CmcCommandEmpty = TRUE;
          break;
        }
        nReadyForSend = wpSendToReceiverIn - wpSendToReceiverOut;
        while(nReadyForSend >= BUFFNUM1) nReadyForSend += BUFFNUM1;	
        //nReadyForSend %= BUFFNUM1;   	// 这行代码错误，可能导致软件死机
        if(nReadyForSend < (BUFFNUM1 - 1)) // Wait for send out at least one byte!	(BUFFNUM0 - 1) wangjiejun 2010-06-07
          break;
      }
      COUNT = 0;
      bSendBufferForReceiver[wpSendToReceiverIn] = *(p + i);
      wpSendToReceiverIn++;
      wpSendToReceiverIn %= BUFFNUM1;
      if(CmcCommandEmpty){

        CmcCommandEmpty = FALSE;
        LPC_UART0->THR = *p;
        //OS_ENTER_CRITICAL();
        wpSendToReceiverOut++;
        //OS_EXIT_CRITICAL();
        wpSendToReceiverOut %= BUFFNUM1;
        UART_IntConfig(LPC_UART0, UART_INTCFG_THRE, ENABLE);
      }
      //UART_IntConfig(LPC_UART0, UART_INTCFG_THRE, ENABLE);
    }
    //UART_IntConfig(LPC_UART0, UART_INTCFG_THRE, ENABLE);
    BSP_OS_SemPost(&BSP_Ser0Lock);
    //*/
    break;
  case 1:
    /*
    BSP_OS_SemWait(&BSP_Ser1Lock, 0);
    for(i = 0; i < length; i++)
    {
    BSP_SER_REG_U1_THR = *(p + i);
    DEF_BIT_SET(BSP_SER_REG_U1_IER, DEF_BIT_01);
    BSP_OS_SemWait(&BSP_Ser1TxWait, 0);
    DEF_BIT_CLR(BSP_SER_REG_U1_IER, DEF_BIT_01);
  }
    BSP_OS_SemPost(&BSP_Ser1Lock);
    */
    BSP_OS_SemWait(&BSP_Ser1Lock, 0);
    for(i = 0; i < length; i++)
    {
      //查询发送
      BSP_SER_REG_U1_THR = *(p + i);
      lsr = BSP_SER_REG_U1_LSR;
      while(!(lsr&0x20))            //等待發送結束
        lsr = BSP_SER_REG_U1_LSR;
    }
    BSP_OS_SemPost(&BSP_Ser1Lock);
    //*/
    break;
  case 2:
    /*
    BSP_OS_SemWait(&BSP_Ser2Lock, 0);
    for(i = 0; i < length; i++)
    {
    BSP_SER_REG_U2_THR = *(p + i);
    DEF_BIT_SET(BSP_SER_REG_U2_IER, DEF_BIT_01);
    BSP_OS_SemWait(&BSP_Ser2TxWait, 0);
    DEF_BIT_CLR(BSP_SER_REG_U2_IER, DEF_BIT_01);
  }
    BSP_OS_SemPost(&BSP_Ser2Lock);
    */
    /*
    BSP_OS_SemWait(&BSP_Ser2Lock, 0);
    for(i = 0; i < length; i++)
    {
    //查询发送
    BSP_SER_REG_U2_THR = *(p + i);
    lsr = BSP_SER_REG_U2_LSR;
    while(!(lsr&0x20))            //等待發送結束
    lsr = BSP_SER_REG_U2_LSR;
  }
    BSP_OS_SemPost(&BSP_Ser2Lock);
    */  //edit 2014.06.19 屏蔽该段代码，将查询发送改为中断发送

    //edit 2014.06.19 GPS数据通过中断方式发送//add by xxw 20140730
    for(i = 0; i < length; i++)
    {
      while(1)
      {
        if((COUNT_gps ++) > 65535)
        {
          CmcCommandEmpty_gps = TRUE;
          break;
        }
        nReadyForSend_gps = wpSendToReceiverIn_gps - wpSendToReceiverOut_gps;
        while(nReadyForSend_gps >= BUFFNUM1) nReadyForSend_gps += BUFFNUM1;	
        if(nReadyForSend_gps < (BUFFNUM1 - 1))
          break;
      }
      COUNT_gps = 0;
      bSendBufferForReceiver_gps[wpSendToReceiverIn_gps] = *(p + i);
      wpSendToReceiverIn_gps++;
      wpSendToReceiverIn_gps %= BUFFNUM1;
      if(CmcCommandEmpty_gps)
      {
        CmcCommandEmpty_gps = FALSE;
        LPC_UART2->THR = *p;
        wpSendToReceiverOut_gps++;
        wpSendToReceiverOut_gps %= BUFFNUM1;
        UART_IntConfig(LPC_UART2, UART_INTCFG_THRE, ENABLE);
        //LPC_UART2->IER |= UART_IER_THREINT_EN;  //使能发送中断 for test
      }
    }
    //BSP_OS_SemPost(&BSP_Ser2Lock);
    break;
  case 3:
    /*BSP_OS_SemWait(&BSP_Ser3Lock, 0);
    for(i = 0; i < length; i++)
    {
    //查询发送
    BSP_SER_REG_U3_THR = *(p + i);
    lsr = BSP_SER_REG_U3_LSR;
    while(!(lsr&0x20))            //等待發送結束
    lsr = BSP_SER_REG_U3_LSR;
  }
    BSP_OS_SemPost(&BSP_Ser3Lock);
    */  //edit 2014.06.19 屏蔽该段代码，将查询发送改为中断发送

    //edit 2014.06.19 BT数据通过中断方式发送//add by xxw 20140730
    for(i = 0; i < length; i++)
    {
      while(1)
      {
        if((COUNT_bt ++) > 65535)
        {
          CmcCommandEmpty_bt = TRUE;
          break;
        }
        nReadyForSend_bt = wpSendToReceiverIn_bt - wpSendToReceiverOut_bt;
        while(nReadyForSend_bt >= BUFFNUM1) nReadyForSend_bt += BUFFNUM1;	
        if(nReadyForSend_bt < (BUFFNUM1 - 1))
          break;
      }
      COUNT_bt = 0;
      bSendBufferForReceiver_bt[wpSendToReceiverIn_bt] = *(p + i);
      wpSendToReceiverIn_bt++;
      wpSendToReceiverIn_bt %= BUFFNUM1;
      if(CmcCommandEmpty_bt)
      {
        CmcCommandEmpty_bt = FALSE;
        LPC_UART3->THR = *p;
        wpSendToReceiverOut_bt++;
        wpSendToReceiverOut_bt %= BUFFNUM1;
        UART_IntConfig(LPC_UART3, UART_INTCFG_THRE, ENABLE);
        //LPC_UART3->IER |= UART_IER_THREINT_EN;   //使能发送中断 for test
      }
    }
    break;
    /*********************2011-10-31 XULIANG*****************/
  case 4:
    for(i = 0; i < length; i++)
    {
      //查询发送
      //while( !(LPC_UART0->LSR & UART_LSR_THRE) );	//While THR is NOT empty, wait!
      //LPC_UART0->THR = *(p + i);			//Load new data	
      while(1){
        if((COUNT_radio ++) > 65535)
        {
          //BSP_SerInit(0,rate_current);
          CmcCommandEmpty_radio = TRUE;
          break;
        }
        nReadyForSend_radio = wpSendToReceiverIn_radio - wpSendToReceiverOut_radio;
        while(nReadyForSend_radio >= BUFFNUM1) nReadyForSend_radio += BUFFNUM1;	
        //nReadyForSend %= BUFFNUM1;   	// 这行代码错误，可能导致软件死机
        if(nReadyForSend_radio < (BUFFNUM1 - 1)) // Wait for send out at least one byte!	(BUFFNUM0 - 1) wangjiejun 2010-06-07
          break;
      }
      COUNT_radio = 0;
      bSendBufferForReceiver_radio[wpSendToReceiverIn_radio] = *(p + i);
      wpSendToReceiverIn_radio++;
      wpSendToReceiverIn_radio %= BUFFNUM1;
      if(CmcCommandEmpty_radio){

        CmcCommandEmpty_radio = FALSE;
        LPC_UART4->THR = *p;
        //OS_ENTER_CRITICAL();
        wpSendToReceiverOut_radio++;
        //OS_EXIT_CRITICAL();
        wpSendToReceiverOut_radio %= BUFFNUM1;
        UART4_IntConfig(LPC_UART4, UART_INTCFG_THRE, ENABLE);
      }
    }
    break;
    /*********************2011-10-31 XULIANG*****************/
  default:
    break;
  }
}
int my_putchar(uint8_t ch)//add by xxw 20140820 实现print功能
{
  while( !(LPC_UART0->LSR & UART_LSR_THRE) );	//While THR is NOT empty, wait!
  LPC_UART0->THR = (uint8_t)ch;			//Load new data	
  return ch;
}
int fputc(int ch,struct FILE *f)//add by xxw 20140820 实现print功能
{
  while( !(LPC_UART0->LSR & UART_LSR_THRE) );	//While THR is NOT empty, wait!
  LPC_UART0->THR = (uint8_t)ch;			//Load new data	
  return ch;
}
