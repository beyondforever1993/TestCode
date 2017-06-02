/*************************************************************************
*
*    Used with ICCARM and AARM.
*
*    (c) Copyright IAR Systems 2008
*
*    File name   : sd_spi_mode.c
*    Description : SD/MMC driver
*
*    History :
*    1. Date        : April 10, 2008
*       Author      : Stanimir Bonev
*       Description : Create
*    $Revision: 28532 $
**************************************************************************/
#include "sd_spi_mode.h"

DiskCtrlBlk_t _SdDskCtrlBlk;
Int8U  _SdSdCsd[16];
Boolean       _bHC;    // 大容量标志 > 2GB 

extern volatile uint32_t CardRCA;

extern volatile en_Mci_CardType MCI_CardType;

BSP_OS_SEM BSP_MMC_RLock;
BSP_OS_SEM BSP_MMC_WLock;

/*************************************************************************
* Function Name: SdDiskInit
* Parameters:  none
*
* Return: none
*
* Description: Init MMC/SD disk
*
*************************************************************************/
void SdDiskInit (void)
{
  switch (MCI_disk_initialize()) //xf
  {
  case 0:// 正常
    _SdDskCtrlBlk.DiskStatus = DiskCommandPass;
    g_File.bSDState = 0; 
    g_LedMod.Sate = 2;
    break;
  default:
    _SdDskCtrlBlk.DiskStatus = DiskNotPresent;
    g_File.bSDState = 1;
    DebugMsg("error: MMC Not Present !!!\r\n");
    break;
  }
}

/**
SD MMC 初始化 

返回值：
0 -> 成功
非0 -> 失败
**/
int MCI_disk_initialize(void)
{
  int err = 0;
  int i;
  uint8_t capity,memory_size;
  char size[3]={0,0,0};
  st_Mci_CardId cid;
  
  _SdDskCtrlBlk.DiskType = DiskMMC;
  
#if MCI_DMA_ENABLED
  /* on DMA channel 0, source is memory, destination is MCI FIFO. */
  /* On DMA channel 1, source is MCI FIFO, destination is memory. */
  GPDMA_Init();
#endif
  
  if ( MCI_Init(LOW_LVL) != MCI_FUNC_OK )
  {
    err++; /* fatal error */
  }
  
  if ( err || MCI_GetCID(&cid) != MCI_FUNC_OK )
  {
    err++; /* fatal error */
  }
  else
  {
    /** PNM_L
    4G --> "M04G"
    8G --> "M08G"
    **/
    
    size[0] = (cid.PNM_L >> 8) & 0xff;
    size[1] = cid.PNM_L & 0xff;
    
    capity = (size[0]-'0')*10+( size[1] - '0');
    
    switch(capity)
    {
    case 4:
      memory_size = 1;
      _SdDskCtrlBlk.BlockNumb = 7733248;
      break;
    case 8:
      memory_size = 2;
      _SdDskCtrlBlk.BlockNumb = 7733248*2;
      break;
    case 16:
      memory_size = 3;
      _SdDskCtrlBlk.BlockNumb = 7733248*4;
      break;
    case 32:
      memory_size = 4;
      _SdDskCtrlBlk.BlockNumb = 7733248*8;
      break;
    default://ERROR
      memory_size = 0;
      _SdDskCtrlBlk.BlockNumb = 0;
      break;
    }
    g_Byte128[51] = 0x21;
    g_Byte128[58] = memory_size;
    WriteFlash();
   // DebugMsg("Capity of the iNAND is :%d G...\r\n", capity);
  }
  
  /* RCA ---------------------------------------------------------------------*/
  
  if ( err || MCI_SetCardAddress() != MCI_FUNC_OK )
  {
    err++; /* fatal error */
  }
  
  /* CSD ---------------------------------------------------------------------*/
  
  if(MCI_GetCSD((uint32_t *)_SdSdCsd) != MCI_FUNC_OK)
  {
    return(_SdNoResponse);
  }
  
  /** Block Size **/
  _SdDskCtrlBlk.BlockSize = 1<<_CSD_GET_READ_BL_LEN();
  
  /* Select Card -------------------------------------------------------------*/
  
  MCI_Cmd_SelectCard();
  
  MCI_SetBusWidth(SD_4_BIT);
  
  while(MCI_CheckStatus(CARD_STATE_TRAN) != MCI_FUNC_OK)
  {
    for( i = 0;i < 0x100000; i++);
  }
  
  /* Operation Mode ----------------------------------------------------------*/
  
  MCI_SetOutputMode(0);/* Clear Open Drain output control for mmc */
  
  /* Clock -------------------------------------------------------------------*/
  if ( !err )
  {
    MCI_Set_MCIClock(MCI_NORMAL_RATE );
  }
  
  if( !err )
  {
    //初始化成功
    _SdDskCtrlBlk.DiskStatus = DiskCommandPass;
    g_File.bSDState = 0; 
    g_LedMod.Sate = 2;
  }
  else
  {
    //初始化失败
    _SdDskCtrlBlk.DiskStatus = DiskNotReady;
    g_File.bSDState = 1;
    DebugMsg("error: MMC Init Failed !!!\r\n");
  }
  
  return err;
}

/**
读取块数据放入pBuf指向的缓冲中

返回值：
0 -> 成功
1 -> 失败
**/
extern uint32_t dmaRdCh_TermianalCnt;
Int8U MMC_ReadBlock_S(pInt8U pBuf,Int32U BlockNum, Int32U BlockCnt)
{
  uint32_t cnt_next;
  uint32_t i,j=0;
  CPU_BOOLEAN err;
  
  err = BSP_OS_SemWait(&BSP_MMC_RLock,0);
  
  if( err != DEF_OK)
    return 1;
  
  while (LPC_GPDMA->EnbldChns & (GPDMA_DMACEnbldChns_Ch(MCI_DMA_READ_CHANNEL)));
  
  cnt_next = dmaRdCh_TermianalCnt + 1;
  
  while (MCI_CheckStatus(CARD_STATE_TRAN) != MCI_FUNC_OK)
  {
    for( i = 0;i< 0x10000; i++);
    j++;
    if( j == 10)
    {
      BSP_OS_SemPost(&BSP_MMC_RLock);
      return 1;
    }
  }
  
  i = MCI_ReadBlock(pBuf,BlockNum,BlockCnt);
  
  if( i != MCI_FUNC_OK)
  {
    BSP_OS_SemPost(&BSP_MMC_RLock);
    return 1;
  }
  
  j = 0;
  
  while( cnt_next != dmaRdCh_TermianalCnt)
  {
    
   j++;
   
   if(  LPC_GPDMA->EnbldChns & (GPDMA_DMACEnbldChns_Ch(MCI_DMA_READ_CHANNEL)) == 0)
   {
     BSP_OS_SemPost(&BSP_MMC_RLock);
     return 1;
   }
   
   if( j > 1200000)
   {
     GPDMA_ChannelCmd(MCI_DMA_READ_CHANNEL, DISABLE);
     BSP_OS_SemPost(&BSP_MMC_RLock);
     return 1;
   }
   
  }
  
  BSP_OS_SemPost(&BSP_MMC_RLock);
  
  return 0;
}

/**
将 pBuf指向的缓冲数据写入到 (BlockNum ,BlockNum + Blockcnt -1) 块中

返回值：
0 -> 成功
1 -> 失败
**/

extern uint32_t dmaWrCh_TermianalCnt;

Int8U MMC_WriteBlock_S(pInt8U pBuf,Int32U BlockNum, Int32U BlockCnt)
{
  uint32_t cnt_next;
  uint32_t i,j;
  CPU_BOOLEAN err;
  
  err = BSP_OS_SemWait(&BSP_MMC_WLock,100);
  
  if( err != DEF_OK)
    return 1;
  
  while (LPC_GPDMA->EnbldChns & (GPDMA_DMACEnbldChns_Ch(MCI_DMA_WRITE_CHANNEL)));
  
  cnt_next = dmaWrCh_TermianalCnt + 1;
  
  while (MCI_CheckStatus(CARD_STATE_TRAN) != MCI_FUNC_OK)
  {
    for( i = 0;i< 0x10000; i++);
    j++;
    if( j == 10)
    {
      BSP_OS_SemPost(&BSP_MMC_WLock);
      return 1;
    }
  }
  
  i = MCI_WriteBlock(pBuf,BlockNum,BlockCnt);
  
  if( i != MCI_FUNC_OK)
  {
    BSP_OS_SemPost(&BSP_MMC_WLock);
    return 1;
  }
  
  j = 0;
  while( cnt_next != dmaWrCh_TermianalCnt)
  {
    j++;
    
   if(  LPC_GPDMA->EnbldChns & (GPDMA_DMACEnbldChns_Ch(MCI_DMA_WRITE_CHANNEL)) == 0)
   {
     BSP_OS_SemPost(&BSP_MMC_WLock);
     return 1;
   }
   
    if( j > 1200000)
   {
     GPDMA_ChannelCmd(MCI_DMA_WRITE_CHANNEL, DISABLE);
     BSP_OS_SemPost(&BSP_MMC_RLock);
     return 1;
   }
   
  }
  
  BSP_OS_SemPost(&BSP_MMC_WLock);
  
  return 0;
}

/*************************************************************************
* Function Name: SdGetDiskCtrlBkl
* Parameters:  none
*
* Return: pDiskCtrlBlk_t
*
* Description: Return pointer to status structure of the disk
*
*************************************************************************/
pDiskCtrlBlk_t SdGetDiskCtrlBkl (void)
{
  return(&_SdDskCtrlBlk);
}

/*************************************************************************
* Function Name: SdDiskIO
* Parameters: pInt8U pData,Int32U BlockStart,
*             Int32U BlockNum, DiskIoRequest_t IoRequest
*
* Return: DiskStatusCode_t
*
* Description: MMC/SD disk I/O
*
*************************************************************************/
DiskStatusCode_t SdDiskIO (pInt8U pData,Int32U BlockStart,
                           Int32U BlockNum, DiskIoRequest_t IoRequest)
{
  
  if((NULL == pData) || (BlockStart+BlockNum > _SdDskCtrlBlk.BlockNumb))
  {
    return(DiskParametersError);
  }
  
  if (_SdDskCtrlBlk.DiskStatus != DiskCommandPass )
  {//SD卡状态错误
    return(_SdDskCtrlBlk.DiskStatus);
  }
  
  switch (IoRequest)
  {
  case DiskWrite:
    
    if(_SdDskCtrlBlk.WriteProtect)
    {
      return(DiskParametersError);
    }
    
    MMC_WriteBlock_S(pData,BlockStart,BlockNum);
    
    break;
  case DiskRead:
    
    MMC_ReadBlock_S(pData,BlockStart,BlockNum);
    
    break;
  case DiskVerify:
    break;
  default:
    return(DiskParametersError);
  }
  
  return(_SdDskCtrlBlk.DiskStatus);
}


#if MCI_DMA_ENABLED
/******************************************************************************
**  DMA Handler
******************************************************************************/
extern void ADC_DMA_IRQHandler();
#include <bsp_adc.h>

void DMA_IRQHandler (void)
{
  	if (GPDMA_IntGetStatus(GPDMA_STAT_INT, MCI_DMA_READ_CHANNEL))
	{
          MCI_DMA_IRQHandler();
	}
        else if (GPDMA_IntGetStatus(GPDMA_STAT_INT, MCI_DMA_WRITE_CHANNEL))
        {
          MCI_DMA_IRQHandler();
        }
        else if (GPDMA_IntGetStatus(GPDMA_STAT_INT, ADC_DMA_CH))
        {
          ADC_DMA_IRQHandler();
        }
  
}
#endif
