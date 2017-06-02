/***
 * @file		Mci_CidCard.c
 * @purpose		This example describes how to using UART in IrDA mode
 * @version		1.0
 * @date		06. October. 2010
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
//#include "includes.h"
#include "lpc177x_8x.h"
#include "system_LPC177x_8x.h"
#include "lpc_types.h"
#include "core_cm3.h"
//#include "lpc177x_8x_mci.h"
#include "lpc177x_8x_timer.h"
#include "usb_cnfg.h"
#include "usb_desc.h"
#include "usb_hw.h"
#include "usb_t9.h"
#include "usb_hooks.h"
#include "usb_dev_desc.h"
#include "usb_hooks.h"
#include "usb_buffer.h"

#include "scsi_ll.h"
#include "scsi.h"
#include "scsi_cmd.h"

#include "disk.h"
#include "lun.h"
#include  "ucos_ii.h"
#include "sd_dsk_desc.h"
#include "sd_spi_mode.h"
#include "sd_ll_spi1.h"
#include "debug_frmwrk.h"

#include "global.h"


#define TIMER0_TICK_PER_SEC   2
#define UPDATE_SHOW_DLY       ((Int32U)(0.5 * TIMER0_TICK_PER_SEC))
volatile Boolean TickFlag = FALSE;

#pragma location="USB_DMA_RAM"
#pragma data_alignment=4
__no_init unsigned char Lun0Buffer[512];

/*variable for clitical section entry control*/
unsigned int CriticalSecCntr;
/* test_5555 **/
uint8_t buf_test[1024];
uint32_t test_i;
static uint32_t test_cnt ;
FIL file_test;
void func_ff_test(void)
{
  UINT bw;
  FRESULT res;
  /* test_5555 **/
  for( test_i = 0; test_i <1023;test_i ++)
  {
    buf_test[test_i] = '0'+ test_i%10;
  }
  buf_test[1023] = 0;

  /** test_5555 **/
  res = f_open(&file_test, "test_20150121.txt", (FA_READ|FA_WRITE));

  if( (uint32_t)(file_test.fs) == 0)
  {
//    while(1);
  }
  if( res != FR_OK)
  {
//    while(1);
  }
  f_lseek(&file_test,file_test.fsize);
    if( (uint32_t)(file_test.fs) == 0)
  {
//    while(1);
  }
  f_write(&file_test,buf_test,sizeof buf_test,&bw);
  if( bw == 0)
  {
//    while (1);
  }
    if( (uint32_t)(file_test.fs) == 0)
  {
//    while(1);
  }
  //f_sync(&file_test);

  if(bw == 0)
  {
//    while(1);
  }

  f_close(&file_test);

  test_cnt++;


}
