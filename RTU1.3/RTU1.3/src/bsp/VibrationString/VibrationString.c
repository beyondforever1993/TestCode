#include "includes.h"

struct DATA_MEASURE MeasureDat;
pwm_para_t pwm_para;
static uint32_t pwm_task_timeout;

static void Pwm_SetFre(UINT32 freq);
static void Pwm_measure_init(void);
static void Pwm_measure_Deinit(void);
static void Pwm_ScanFreq(UINT32 StartFreq,UINT32 EndFreq);
static void Stri_measure_section(UINT32 frq_low, UINT32 frq_high);

void pwm_task(void)
{
//	Stri_measure();
//	OSTimeDlyHMSM(0, 0, 8, 0);
	struct DATA_STRUCT dat;
	struct TIME_STRUCT time_now;

        UINT8 buf[30];
	
	if( pwm_para.frq_changed)
	{
		pwm_para.frq_changed = 0;
		pwm_task_timeout = pwm_para.frq * 2400;
	}	
	
	if(pwm_para.frq != 0)
	{
		if(pwm_task_timeout ==0)
		{
			pwm_task_timeout = pwm_para.frq * 2400;
			rtc_get_time(&time_now);
			
			Stri_measure();
			
			dat.y = time_now.y;
			dat.m = time_now.m;
			dat.d = time_now.d;
			dat.H = time_now.H;
			dat.M = time_now.M;
			dat.S = time_now.S;		
			
			dat.type = pwm_para.type;
			
			sprintf((char *)dat.data,"%s,%d",pwm_para.sname,pwm_para.freq_Value);
                                               
                        sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d\r\n",dat.y,dat.m,dat.d,\
                          dat.H,dat.M,dat.S);
                        DebugMsg(buf);
			
			g_RtuStatus.led_dwload = 1;
			g_RtuStatus.stri = 1;
                        file_write(&dat);
		}
	}
}

void Stri_measure(void)
{
//总时间2.252411s	间隔446 1393
	Stri_measure_section(846-2,1793+2);
	if(pwm_para.freq_Value > FRQ_LOW && pwm_para.freq_Value < FRQ_HIGH)
	{
		GPIO_OutputValue(BRD_PWM_SHUTD_PORT, BRD_PWM_SHUTD_MASK, SWITCH_LOW);
		return;	
	}
	
	Stri_measure_section(1793-2,3800);
	if(pwm_para.freq_Value > FRQ_LOW && pwm_para.freq_Value < FRQ_HIGH)
	{
		GPIO_OutputValue(BRD_PWM_SHUTD_PORT, BRD_PWM_SHUTD_MASK, SWITCH_LOW);
		return;	
	}		
	
	Stri_measure_section(400,846+2);
	if(pwm_para.freq_Value > FRQ_LOW && pwm_para.freq_Value < FRQ_HIGH)
	{
		GPIO_OutputValue(BRD_PWM_SHUTD_PORT, BRD_PWM_SHUTD_MASK, SWITCH_LOW);
		return;	
	}	
}

static void Stri_measure_section(UINT32 frq_low, UINT32 frq_high)
{
	Pwm_ScanFreq(frq_low, frq_high);
	OSTimeDlyHMSM(0, 0, 0, 10);
	Pwm_measure_init();
	OSTimeDlyHMSM(0, 0, 0, 100);
	Pwm_measure_Deinit();
}

void Pwm_init(void)
{
	uint32_t MatchVal;
	PWM_TIMERCFG_Type PWMCfgDat;
	PWM_MATCHCFG_Type PWMMatchCfgDat;
	PWM_CAPTURECFG_Type PWMCaptureCfgDat;
	
	GPIO_OutputValue(BRD_PWM_POWER_12V_PORT, BRD_PWM_POWER_12V_MASK, SWITCH_LOW);

	/* PWM block section -------------------------------------------- */
	/* Initialize PWM peripheral, timer mode
	* PWM prescale value = 1 (absolute value - tick value) */		/*	*/
	PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
	PWMCfgDat.PrescaleValue = PRESCALEVALUE;
	PWM_Init(_USING_PWM_NO, PWM_MODE_TIMER, (void *) &PWMCfgDat);

	// Initialize PWM pin connect
	PINSEL_ConfigPin (PWM_VMT_PORT, PWM_VMT_PIN, 1);
	// Initialize PWM pin connect
	PINSEL_ConfigPin (PWM_VMT_PORT, PWM_CAP0_PIN, 1);	


	/* Set match value for PWM match channel 0 = 256, update immediately */
	MatchVal = 4294967295;		//2^32
	PWM_MatchUpdate(_USING_PWM_NO, 0, MatchVal, PWM_MATCH_UPDATE_NOW);

	/* PWM Timer/Counter will be reset when channel 0 matching
	* no interrupt when match
	* no stop when match */
	PWMMatchCfgDat.IntOnMatch = ENABLE;
	PWMMatchCfgDat.MatchChannel = 0;
	PWMMatchCfgDat.ResetOnMatch = ENABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(_USING_PWM_NO, &PWMMatchCfgDat);

	/* Configure each PWM channel: --------------------------------------------- */
	/* - Single edge
	* - PWM Duty on each PWM channel determined by
	* the match on channel 0 to the match of that match channel.
	* Example: PWM Duty on PWM channel 1 determined by
	* the match on channel 0 to the match of match channel 1.
	*/

	/* Configure PWM channel edge option
	* Note: PWM Channel 1 is in single mode as default state and
	* can not be changed to double edge mode */


	/* Set up match value */
	
	PWM_MatchUpdate(_USING_PWM_NO, PWM_VMT_CHANNEL, MatchVal/2, PWM_MATCH_UPDATE_NOW);

	/* Configure match option */
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = PWM_VMT_CHANNEL;
	PWMMatchCfgDat.ResetOnMatch = DISABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(_USING_PWM_NO, &PWMMatchCfgDat);
	
	PWMCaptureCfgDat.CaptureChannel = 0;
	PWMCaptureCfgDat.RisingEdge = ENABLE;
	PWMCaptureCfgDat.FallingEdge = DISABLE;
	PWMCaptureCfgDat.IntOnCaption = ENABLE;
	PWM_ConfigCapture(_USING_PWM_NO, &PWMCaptureCfgDat);

	/* Enable PWM Channel Output */
	PWM_ChannelCmd(_USING_PWM_NO, PWM_VMT_CHANNEL, DISABLE);

	/* Reset and Start counter */
	PWM_ResetCounter(_USING_PWM_NO);

	PWM_CounterCmd(_USING_PWM_NO, ENABLE);

	/* Start PWM now */
	//PWM_Cmd(_USING_PWM_NO, ENABLE);
    /* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(PWM1_IRQn, ((0x01<<3)|0x01));

//	/* Enable interrupt for timer 0 */
//	NVIC_EnableIRQ(PWM1_IRQn);  
}

static void Pwm_measure_init(void)
{
	//PWM_COUNTERCFG_Type PWMCfgDat;
	PWM_CAPTURECFG_Type PWMCaptureCfgDat;

//	/* PWM block section -------------------------------------------- */
//	/* Initialize PWM peripheral, timer mode
//	* PWM prescale value = 1 (absolute value - tick value) */		/*	*/
//	PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
//	PWMCfgDat.PrescaleValue = PRESCALEVALUE;
//	PWM_Init(_USING_PWM_NO, PWM_MODE_TIMER, (void *) &PWMCfgDat);	

//	// Initialize PWM pin connect
//	PINSEL_ConfigPin (PWM_VMT_PORT, PWM_CAP0_PIN, 1);
	
	PWMCaptureCfgDat.CaptureChannel = 0;
	PWMCaptureCfgDat.RisingEdge = DISABLE;
	PWMCaptureCfgDat.FallingEdge = ENABLE;
	PWMCaptureCfgDat.IntOnCaption = ENABLE;
	
	PWM_ConfigCapture(_USING_PWM_NO, &PWMCaptureCfgDat);

//	/* Enable PWM Channel Output */
//	PWM_ChannelCmd(_USING_PWM_NO, PWM_VMT_CHANNEL, ENABLE);

	/* Reset and Start counter */
	PWM_ResetCounter(_USING_PWM_NO);

//	PWM_CounterCmd(_USING_PWM_NO, ENABLE);

	/* Start PWM now */
	//PWM_Cmd(_USING_PWM_NO, ENABLE);	
	
//	NVIC_SetPriority(PWM1_IRQn, ((0x01<<3)|0x01));
	NVIC_EnableIRQ(PWM1_IRQn);
	
	MeasureDat.CapMatch_cnt = 0;
	pwm_para.freq_Value = 0;
}

static void Pwm_measure_Deinit(void)
{
	PWM_CAPTURECFG_Type PWMCaptureCfgDat;
	
	PWMCaptureCfgDat.CaptureChannel = 0;
	PWMCaptureCfgDat.RisingEdge = DISABLE;
	PWMCaptureCfgDat.FallingEdge = DISABLE;
	PWMCaptureCfgDat.IntOnCaption = DISABLE;
	PWM_ConfigCapture(_USING_PWM_NO, &PWMCaptureCfgDat);	
	
	NVIC_DisableIRQ(PWM1_IRQn);
}

static void Pwm_SetFre(UINT32 freq)
{
	uint32_t MatchVal;
	
	MatchVal = 60000000/PRESCALEVALUE/freq;
	PWM_MatchUpdate(_USING_PWM_NO, 0, MatchVal, PWM_MATCH_UPDATE_NOW);
	PWM_MatchUpdate(_USING_PWM_NO, PWM_VMT_CHANNEL, MatchVal/2, PWM_MATCH_UPDATE_NOW);
}

static void Pwm_ScanFreq(UINT32 StartFreq,UINT32 EndFreq)
{
	UINT32 i,j;
	PWM_MATCHCFG_Type PWMMatchCfgDat;
	
	if(StartFreq > EndFreq)
		return;
	
	GPIO_OutputValue(BRD_PWM_POWER_5V_PORT, BRD_PWM_POWER_5V_MASK, SWITCH_HIGH);
	GPIO_OutputValue(BRD_PWM_SHUTD_PORT, BRD_PWM_SHUTD_MASK, SWITCH_LOW);
        
	
	PWMMatchCfgDat.IntOnMatch = ENABLE;
	PWMMatchCfgDat.MatchChannel = 0;
	PWMMatchCfgDat.ResetOnMatch = ENABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(_USING_PWM_NO, &PWMMatchCfgDat);	
	
	PWM_ChannelCmd(_USING_PWM_NO, PWM_VMT_CHANNEL, ENABLE);
	NVIC_EnableIRQ(PWM1_IRQn); 
	for(i=StartFreq-2;i<EndFreq+2;i++)
	{
		Pwm_SetFre(i);
		MeasureDat.ScanMatch_cnt = 0;
		for(j=0;j<60000;j++)		//超时控制,约6-8个周期
		{
			if(MeasureDat.ScanMatch_cnt >= 1)		//振动n个周期
				break;
		}
	}
	//PWM_DeInit(_USING_PWM_NO);
	PWM_MatchUpdate(_USING_PWM_NO, PWM_VMT_CHANNEL, 0, PWM_MATCH_UPDATE_NOW);
	OSTimeDlyHMSM(0, 0, 0, 3);
	PWM_MatchUpdate(_USING_PWM_NO, 0, 4294967295, PWM_MATCH_UPDATE_NOW);
	GPIO_OutputValue(BRD_PWM_POWER_5V_PORT, BRD_PWM_POWER_5V_MASK, SWITCH_LOW);
	GPIO_OutputValue(BRD_PWM_SHUTD_PORT, BRD_PWM_SHUTD_MASK, SWITCH_HIGH);
        
//        GPIO_OutputValue((3),(1<<(11)),SWITCH_HIGH);
	
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = 0;
	PWMMatchCfgDat.ResetOnMatch = DISABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(_USING_PWM_NO, &PWMMatchCfgDat);	
	
	NVIC_DisableIRQ(PWM1_IRQn);
	PWM_ChannelCmd(_USING_PWM_NO, PWM_VMT_CHANNEL, DISABLE);
}

void PWM1_IRQHandler(void)
{
	if (PWM_GetIntStatus(_USING_PWM_NO, PWM_INTSTAT_CAP0))
	{
		/* Clear the interrupt flag */
		MeasureDat.CapMatch_cnt++;
		if(MeasureDat.CapMatch_cnt <= 10)		//舍弃前10次
		{
			PWM_ResetCounter(_USING_PWM_NO);
		}
		else if(MeasureDat.CapMatch_cnt < 20 )
		{}
		else if(MeasureDat.CapMatch_cnt == 20 )	//采集10次
		{
			MeasureDat.CaptureVal = PWM_GetCaptureValue(_USING_PWM_NO, 0);
			PWM_ResetCounter(_USING_PWM_NO);
			if(MeasureDat.CaptureVal)
				pwm_para.freq_Value = 60000000*10/MeasureDat.CaptureVal;
			MeasureDat.CapMatch_cnt = 0;
		}
		else
		{
			MeasureDat.CapMatch_cnt = 0;
			PWM_ResetCounter(_USING_PWM_NO);
		}
		PWM_ClearIntPending(_USING_PWM_NO, PWM_INTSTAT_CAP0);
	}	
	
	if (PWM_GetIntStatus(_USING_PWM_NO, PWM_INTSTAT_MR0))
	{
		MeasureDat.ScanMatch_cnt++;
		PWM_ClearIntPending(_USING_PWM_NO, PWM_INTSTAT_MR0);
	}	
	
}

void pwm_timer_handler(void)
{
	if( pwm_task_timeout > 0)
    	pwm_task_timeout--;		
}

