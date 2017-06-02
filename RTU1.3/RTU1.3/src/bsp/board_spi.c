
#include "includes.h"

extern unsigned char g_aSPI_Data_Buf[];
extern unsigned char buf_sp;
extern unsigned char Radio_Type;
// SSP Configuration structure variable
SSP_CFG_Type SSP_ConfigStruct;

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
* @brief 		SSP1 Interrupt used for reading and writing handler
* @param		None
* @return 		None
***********************************************************************/
void SSP1_IRQHandler(void)
{
  uint16_t tmp;
  /*uint8_t dataword;
  if(SSP_GetDataSize(LPC_SSP1)>8)
  dataword = 1;
	else
  dataword = 0;*///delete by xxw 20140815 消除警告 这个没有用
  tmp = SSP_GetRawIntStatusReg(LPC_SSP1);
  // Check overrun error
  if (tmp & SSP_RIS_ROR)
  {
    // Clear interrupt
    SSP_ClearIntPending(LPC_SSP1, SSP_INTCLR_ROR);
    
  }
  if (tmp & SSP_RIS_RT)
  {
    // Clear interrupt
    SSP_ClearIntPending(LPC_SSP1, SSP_INTCLR_RT);
    
  }
  while (SSP_GetStatus(LPC_SSP1, SSP_STAT_RXFIFO_NOTEMPTY))
  {
    // Read data from SSP data
    tmp = SSP_ReceiveData(LPC_SSP1);
    if(Radio_Type == R_F45M_RECV_RADIO)//-45M	
      g_aSPI_Data_Buf[buf_sp++] = ~(uint8_t)tmp;
    else
      g_aSPI_Data_Buf[buf_sp++] = (uint8_t)tmp;
  }
}

void SPI_Init(void)
{
  uint32_t tmp;
  
  Board_RRADIO_Control_Init();
  PINSEL_ConfigPin(0, 6, 2);
  
  PINSEL_ConfigPin(0, 7, 2);
  PINSEL_SetFilter(0, 7, 0);
  
  PINSEL_ConfigPin(0, 8, 2);
  PINSEL_SetFilter(0, 8, 0);
  
  PINSEL_ConfigPin(0, 9, 2);
  PINSEL_SetFilter(0, 9, 0);
  
  //* Initializing SSP device section ------------------------------------------------------ */
  // initialize SSP configuration structure to default
  SSP_ConfigStructInit(&SSP_ConfigStruct);
  SSP_ConfigStruct.CPHA = SSP_CPHA_SECOND;
  SSP_ConfigStruct.CPOL = SSP_CPOL_LO;
  // Initialize SSP peripheral with parameter given in structure above
  SSP_Init(LPC_SSP1, &SSP_ConfigStruct);
  
  // Enable SSP peripheral
  SSP_Cmd(LPC_SSP1, ENABLE);
  
  // preemption = 1, sub-priority = 1 //
  NVIC_SetPriority(SSP1_IRQn, ((0x01<<4)|0x0e));
  // Enable SSP0 interrupt //
  NVIC_EnableIRQ(SSP1_IRQn);
  
  // Clear all remaining data in RX FIFO //
  while (LPC_SSP1->SR & SSP_SR_RNE)
  {
    tmp = (uint32_t) SSP_ReceiveData(LPC_SSP1);
  }
  tmp = tmp;//add by xxw 20140815 消除警告
  // Clear status
  LPC_SSP1->ICR = SSP_ICR_BITMASK;
  SSP_IntConfig(LPC_SSP1, SSP_INTCFG_ROR|SSP_INTCFG_RT|SSP_INTCFG_RX, ENABLE);
}