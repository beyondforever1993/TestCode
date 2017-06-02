#include "RTC.h"
#include "RTC_DS1339.h"
#include "Global.h"

HARDWARE_VERSION HardvareVersion;

void rtc_lpc1778_init(void)
{
  if( LPC_RTC->GPREG0 != 0xAAAA5555)
  {
    LPC_RTC->GPREG0 = 0xAAAA5555;
    
    RTC_Init(LPC_RTC);

    RTC_ResetClockTickCounter(LPC_RTC);
    
    RTC_Cmd(LPC_RTC, ENABLE);
    
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR, 0);
    
    LPC_RTC->GPREG1 = 0;//addr_wr
    LPC_RTC->GPREG2 = 0;//addr_rd
    LPC_RTC->GPREG3 = 0;//rain_count_acc
    LPC_RTC->GPREG4 = 0;//reserv
  }

  g_RtuConfig.addr_wr = LPC_RTC->GPREG1;
  g_RtuConfig.addr_rd = LPC_RTC->GPREG2;
  
//  /** 中断配置 **/
// 
//  RTC_CntIncrIntConfig (LPC_RTC, RTC_TIMETYPE_HOUR, ENABLE);
//  
//  NVIC_EnableIRQ(RTC_IRQn);
}

void rtc_lpc1778_get_time(struct TIME_STRUCT * time)
{
  time->S = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);
  time->M = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MINUTE);
  time->H = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_HOUR);
  time->d = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH);
  time->m = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_MONTH);
  time->y = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_YEAR);
}

void rtc_lpc1778_set_time(struct TIME_STRUCT * time)
{
  NVIC_DisableIRQ(RTC_IRQn);
  
  RTC_ClearIntPending(LPC_RTC, RTC_INT_ALARM|RTC_INT_COUNTER_INCREASE);
  
  RTC_Init(LPC_RTC);

  RTC_ResetClockTickCounter(LPC_RTC);
  
  RTC_Cmd(LPC_RTC, ENABLE);
  
  /* Set current time for RTC */

  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_SECOND,      time->S);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MINUTE,      time->M);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_HOUR,        time->H);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH,  time->d);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MONTH,       time->m);
  RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR,        time->y);
  
  NVIC_EnableIRQ(RTC_IRQn);
}

//void RTC_IRQHandler(void)
//{
//  if(RTC_GetIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE))
//  {//整点中断
//    // g_RtuConfig.addr_rd g_RtuConfig.addr_wr  指针保存标志置位
//    g_RtuStatus.dog_save_conf = DOG_SAVE_CONF_TIME;
//  }
//  
//  RTC_ClearIntPending(LPC_RTC, RTC_INT_ALARM|RTC_INT_COUNTER_INCREASE);
//}

void rtc_init(void)
{
	if(rtc_ds1339_init() == SUCCESS)
	{
		HardvareVersion = V13;
		rtc_lpc1778_init();
	}
	else
	{
		HardvareVersion = V11;
		rtc_lpc1778_init();
	}
}

void rtc_log(void)
{
	if(HardvareVersion == V13)
		DebugMsg("External RTC\r\n");
	else
		DebugMsg("Internal RTC\r\n");
}

void rtc_get_time(struct TIME_STRUCT * time)
{
	if(HardvareVersion == V13)
		rtc_ds1339_get_time(time);
	else
		rtc_lpc1778_get_time(time);	
}

void rtc_set_time(struct TIME_STRUCT * time)
{
	if(HardvareVersion == V13)
		rtc_ds1339_set_time(time);
	else
		rtc_lpc1778_set_time(time);	
}