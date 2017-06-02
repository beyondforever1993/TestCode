#include "WDT.h"

void wdt_init(void)
{
  if (WWDT_GetStatus(WWDT_TIMEOUT_FLAG))
  {
    // Clear WDT TimeOut
    WWDT_ClrTimeOutFlag();
  } 
  
  // Initialize WDT, IRC OSC, interrupt mode
  WWDT_Init(WDT_TIME_OUT);
  
  WWDT_Enable(ENABLE);
  
  WWDT_SetMode(WWDT_RESET_MODE, ENABLE);
  
  // Start watchdog with timeout given
  WWDT_Start(WDT_TIME_OUT);
  
  WWDT_Feed();
}