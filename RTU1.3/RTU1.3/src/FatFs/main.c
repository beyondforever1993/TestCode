
/*****************************************************************************
 *   This example is writing and reading data from the eeprom. The example
 *   can be configured to access the eeprom on the OEM Board or on the
 *   base board.
 *
 *   Copyright(C) 2010, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "mcu_regs.h"
#include "lpc_types.h"
#include "pinsel.h"
#include "string.h"
#include "timer.h"
#include "uart.h"
#include "ssp.h"
#include "sdram.h"
#include "diskio.h"
#include "ff.h"
#include "lpc177x_8x_timer.h"

#include "system_lpc177x_8x.h"
#include "lpc177x_8x_systick.h"
#include "lpc177x_8x_gpio.h"
#include "bsp.h"
#include "board_gpio.h"
#include "ucos_ii.h"
#include "lpc177x_8x_eeprom.h"
#include <Global.h>
#include <includes.h>

#include "stdio.h"

#if defined(__GNUC__)

/*
 * This sample application hasn't been ported to GCC/CodeRed tools.
 * There is an assembler file (Lib_MCU/readfifo.s) that must be ported.
 */

#error Currently only compiles with Keil tools
#endif


/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

#define UART_PORT (LPC_UART0)



/* UART buffer */
//static char buff[512];

/******************************************************************************
 * Local Functions
 *****************************************************************************/

//static FATFS lFatfs[1];
//static FILINFO Finfo;

/******************************************************************************
 * Main method
 *****************************************************************************/

int main2 (void)
{
  DIR dir;
  BYTE res;
  FIL file;
  UINT numRead = 0;
  UINT numWritten = 0;
  
  ///*
   SystemCoreClockUpdate();// get SystemCoreClock
    SYSTICK_InternalInit((uint32_t)(1000.0F / (float)OS_TICKS_PER_SEC)); // init os tick timer
    SYSTICK_IntCmd(ENABLE);	// enable os tick timer interrupt
    SYSTICK_Cmd(ENABLE);    // enable os tick timer counter
    
    BSP_SerInit(PORT_ID_COM, 9600);  //for DbgMsg
    
  //*/
  
/*
  
  TIM_TIMERCFG_Type timerCfg;

	// UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
       
  SystemInit();

  // initialize timer
  TIM_ConfigStructInit(TIM_TIMER_MODE, &timerCfg);
  TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerCfg);

  // pinsel for UART
	PINSEL_ConfigPin(0,2,1);
	PINSEL_ConfigPin(0,3,1);


	UART_ConfigStructInit(&UARTConfigStruct);
  UARTConfigStruct.Baud_rate = 115200;

	// Initialize UART0 peripheral with given to corresponding parameter
	UART_Init(UART_PORT, &UARTConfigStruct);

	// Enable UART Transmit
	UART_TxCmd(UART_PORT, ENABLE);

  SysTick_Config(SystemCoreClock/10);

*/
  DebugMsg("FatFS Demo\r\n");


  res = f_mount(0, &lFatfs[0]);
  if (res != FR_OK) {
    sprintf(buff, "Failed to mount 0: %d \r\n", res);
    DebugMsg(buff);
    return 0;
  }

  DebugMsg("Opening '/'\r\n");
  res = f_opendir(&dir, "/");
  if (res) {
    sprintf(buff, "Failed to open /: %d \r\n", res);
    DebugMsg(buff);
    return 0;
  }


  DebugMsg("Listing content of '/'\r\n");
  for(;;) {
    res = f_readdir(&dir, &Finfo);
    if ((res != FR_OK) || !Finfo.fname[0]) break;
    
    DebugMsg(&(Finfo.fname[0]));
    DebugMsg("\r\n");
  
  }
/*
  DebugMsg("\r\nOpening a file\r\n");
  res = f_open (&file, "ip.txt", FA_READ);
  if (res) {
    sprintf(buff, "Failed to open ip.txt: %d \r\n", res);
    DebugMsg(buff);
    return 0;
  }

  //UART_SendString(UART_PORT,(uint8_t*)"Reading content of the file\r\n");
  res =  f_read (&file, buff, 100, &numRead);			// Read data from a file 
  if (res || numRead <= 0) {
    sprintf(buff, "Failed to read ip.txt: %d \r\n", res);
    //UART_SendString(UART_PORT,(uint8_t*)buff);
    return 0;
  }

  buff[numRead] = '\0';
  //UART_SendString(UART_PORT,(uint8_t*)buff);

  //UART_SendString(UART_PORT,(uint8_t*)"\r\nClosing file\r\n");
  res =  f_close (&file);
  if (res) {
    sprintf(buff, "Failed to close ip.txt: %d \r\n", res);
    //ART_SendString(UART_PORT,(uint8_t*)buff);
    return 0;
  }
*/
  while(1)
  {
  DebugMsg("\r\nOpening a file for writing\r\n");
  res = f_open (&file, "new.txt", (FA_CREATE_ALWAYS|FA_READ|FA_WRITE));
  if (res) {
    sprintf(buff, "Failed to open new.txt: %d \r\n", res);
    DebugMsg(buff);
    return 0;
  }

  DebugMsg("Writing  to the file\r\n");
  res = f_write (&file, "Data written to file\r\n", 22, &numWritten);
  if (res) {
    sprintf(buff, "Failed to write to new.txt: %d \r\n", res);
    DebugMsg(buff);
    return 0;
  }


  DebugMsg("Closing file\r\n");
  res =  f_close (&file);
  if (res) {
    sprintf(buff, "Failed to close new.txt: %d \r\n", res);
    DebugMsg(buff);
    return 0;
  }
  }

   while(1);

}



