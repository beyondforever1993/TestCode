#include "includes.h"

extern UINT16 g_LowLatency; //edit 2014.07.16

//定时器0中断函数  	 25ms
void TIMER0_IRQHandler(void)
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
        Timer_Flag.Simcard_Timeout++;//SIMCARD检测计时器
    }
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
   

    rain_count_time_handler();
    
    tsensor_time_handler();
    
    rs485_timer_handler();
    
    adc_time_handler();
    
    rs232_time_handler();
    
    bd_time_handler();
	
	pwm_timer_handler();
    
    //edit 2014.07.16 定时40s发送RTK模式设置指令
    g_LowLatency++;
    if (g_LowLatency > 1600)
        g_LowLatency = 1600;
    
    //bd
    g_RtuStatus.dog_BD ++;
    if(g_RtuStatus.dog_BD > DOG_BD_TIME)
    {
        g_RtuStatus.dog_BD = DOG_BD_TIME;
    }
    
    g_RtuStatus.dog_TM ++;
    if(g_RtuStatus.dog_TM > DOG_TM_TIME)
    {
        g_RtuStatus.dog_TM = DOG_TM_TIME;
    }
    
    g_RtuStatus.dog_file_rd ++;
    if(g_RtuStatus.dog_file_rd > DOG_FILE_RD_TIME)
    {
        g_RtuStatus.dog_file_rd = DOG_FILE_RD_TIME;
    }
    
    /** 使用RTC的整点加中断触发存储 **/
    g_RtuStatus.dog_save_conf ++;
    if(g_RtuStatus.dog_save_conf > DOG_SAVE_CONF_TIME)
    {
        g_RtuStatus.dog_save_conf = DOG_SAVE_CONF_TIME;
    }
    
    g_RtuStatus.dog_heart ++;
    if(g_RtuStatus.dog_heart > DOG_HEART_TIME)
    {
        g_RtuStatus.dog_heart = DOG_HEART_TIME;
    }
	
	g_SetRTCCount++;
	if(g_SetRTCCount > RTCSETTIME)
	{
		g_SetRTCCount = RTCSETTIME;
	}
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
    Timer_Flag.Simcard_Timeout = 0;
}


//定时器初始化
void  Init_Timer0(void)
{
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	TIM_MATCHCFG_Type TIM_MatchConfigStruct ;
    //PINSEL_CFG_Type PinCfg;

    // Conifg P1.28 as MAT0.0
	//PinCfg.Funcnum = 3;
	//PinCfg.OpenDrain = 0;
	//PinCfg.Pinmode = 0;
	//PinCfg.Portnum = 1;
	//PinCfg.Pinnum = 28;
	//PINSEL_ConfigPin(&PinCfg);

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
    /* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(BRD_TIM_INTR_USED, ((0x01<<3)|0x01));

	/* Enable interrupt for timer 0 */
	NVIC_EnableIRQ(BRD_TIM_INTR_USED);
}