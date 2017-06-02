#include "lpc17xx_timer.h"
#include "lpc17xx_libcfg.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "debug_frmwrk.h"
#include <bsp.h>
//Add by XHZ 2011.8.19 
#include "Timer.h"
#include "Huace_Msg.h"
#include "Global_Variable.h"
#include "Communication_Module.h"
#include "Basic.h"
#include "Apis_Cors.h"

//定时器0中断函数  	 25ms 
static void BSP_SerISR_Handler_TMR0(void)
{ 
  
	if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)== SET)
	{
	        Timer_Flag.Wait_Time_Cnt++; //AT命令等待计时器
                Timer_Flag.TimeOut_Cnt++; //AT命令超时计时器
                Timer_Flag.Msg_Data_Timeout++;//华测数据组包计时器
		Timer_Flag.Signal_Weak_Timeout++;	//信号强度低 计时器
		Timer_Flag.Set_Work_Mode_Timeout++;	 //设置工作模式 计时器
                Timer_Flag.Set_Dial_Parameter_Timeout++;//设置拨号连接参数 计时器
                Timer_Flag.Set_Protocol_Parameter_Timeout++;//设置APIS和CORS参数 计时器	
		Timer_Flag.Set_Radio_Freq_Timeout++;	 //设置电台频率  计时器
		Timer_Flag.Request_GPGGA_Timeout++; //请求GPGGA数据 计数器
                Timer_Flag.No_Diff_Data_Timeout++; //无差分数据计时器
		Timer_Flag.GPGGA_Timeout++;	//心跳包计数器 
		Timer_Flag.CORS_No_ACK_Timeout++; //CORS登录数据包响应计时器
		Timer_Flag.Apis_Beat++;	 //心跳包计时器
        }
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);

}

void  Init_Timer_Flag(void)
{  
        Timer_Flag.TimeOut_Cnt = 0;
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.Msg_Data_Timeout = 0;
	Timer_Flag.Signal_Weak_Timeout = 0;
	Timer_Flag.Set_Work_Mode_Timeout = 0;
        Timer_Flag.Set_Dial_Parameter_Timeout = 0;
        Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
	Timer_Flag.Set_Radio_Freq_Timeout = 0;
	Timer_Flag.Request_GPGGA_Timeout = 0;
        Timer_Flag.No_Diff_Data_Timeout = 0;
	Timer_Flag.GPGGA_Timeout = 0;
	Timer_Flag.CORS_No_ACK_Timeout = 0;
	Timer_Flag.Apis_Beat = 0;
}


//定时器初始化
void  Init_Timer0(void)
{
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	TIM_MATCHCFG_Type TIM_MatchConfigStruct ;
        PINSEL_CFG_Type PinCfg;
        
        // Conifg P1.28 as MAT0.0
	PinCfg.Funcnum = 3;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 1;
	PinCfg.Pinnum = 28;
	PINSEL_ConfigPin(&PinCfg);
        
	// Initialize timer 0, prescale count time of 50uS
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 50;

	// use channel 0, MR0
	TIM_MatchConfigStruct.MatchChannel = 0;
	// Enable interrupt when MR0 matches the value in TC register
	TIM_MatchConfigStruct.IntOnMatch   = TRUE;
	//Enable reset on MR0: TIMER will reset if MR0 matches it
	TIM_MatchConfigStruct.ResetOnMatch = TRUE;
	//Stop on MR0 if MR0 matches it
	TIM_MatchConfigStruct.StopOnMatch  = FALSE;
	//Nothiing if MR0 matches it
	TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	// Set Match value, count value of 50 (500 * 50uS = 25ms） 
	TIM_MatchConfigStruct.MatchValue  = 500;	 //25ms

	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);
	TIM_ConfigMatch(LPC_TIM0,&TIM_MatchConfigStruct);

	/* preemption = 1, sub-priority = 1 */
	//NVIC_SetPriority(TIMER0_IRQn, ((0x01<<3)|0x01));
	/* Enable interrupt for timer 0 */
	//NVIC_EnableIRQ(TIMER0_IRQn);
	// To start timer 0
	TIM_Cmd(LPC_TIM0,ENABLE);
       BSP_IntVectSet((CPU_INT08U   )BSP_INT_SRC_NBR_TMR0,
                     (CPU_FNCT_VOID)BSP_SerISR_Handler_TMR0 );
  
      BSP_IntEn(BSP_INT_SRC_NBR_TMR0);   
}







