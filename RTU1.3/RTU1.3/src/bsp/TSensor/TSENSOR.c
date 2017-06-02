#include "TSENSOR.h"

uint32_t cap_st[CAPATURE_TIMES];
uint32_t cap_tt[CAPATURE_TIMES];

static uint32_t cap_st_idx,cap_tt_idx;

static void tsensor_timer_init(void);  //TSENSOR 专用

static uint32_t tsensor_timeout;
static TSENSOR_OPS sensor_ops;

/** 热敏电阻R/T对照表 3950 **/
const int32_t  tsensor_table[]=
{
264279,250344,237130,224603,212733,201487,190836,180750,171201,162163,   // -40  --  -31
153610,145516,137858,130614,123761,117280,111149,105351, 99867, 94681,   // -30  --  -21
 89776, 85137, 80750, 76600, 72676, 68963, 65451, 62129, 58986, 56012,   // -20  --  -11
 53198, 50534, 48013, 45627, 43368, 41229, 39204, 37285, 35468, 33747,   // -10  --  -1
 32116, 30570, 29105, 27716, 26399, 25150, 23965, 22842, 21776, 20764,   //  0   --  9
 19783, 18892, 18026, 17204, 16423, 15681, 14976, 14306, 13669, 13063,   //  10  --  19
 12487, 11939, 11418, 10921, 10449, 10000,  9571,  9164,  8775,  8405,   //  20  --  29
  8052,  7716,  7396,  7090,  6798,  6520,  6255,  6002,  5760,  5529,   //  30  --  39
  5309,  5098,  4897,  4704,  4521,  4345,  4177,  4016,  3863,  3716,   //  40  --  49
  3588,  3440,  3311,  3188,  3069,  2956,  2848,  2744,  2644,  2548,   //  50  --  59
  2457,  2369,  2284,  2204,  2126,  2051,  1980,  1911,  1845,  1782,   //  60  --  69
  1721,  1663,  1606,  1552,  1500,  1450,  1402,  1356,  1312,  1269,   //  70  --  79
  1228,  1188,  1150,  1113,  1078,  1044,  1011,   979,   948,   919,   //  80  --  89
   891,   863,   837,   811,   787,   763,   740,   718,   697,   676,   //  90  --  99
   657,   637,   619,   601,   584,   567,                               // 100  --  105
};  

#define TSENSOR_TABLE_CNT (sizeof(tsensor_table)/sizeof(*tsensor_table))

void tsensor_init(void)
{
    /** GPIO 初始化 **/
  CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);
  
  /** 变量初始化 **/
  tsensor_timeout = 0;
  cap_st_idx = 0;
  cap_tt_idx = 0;
  
  sensor_ops = TSENSOR_DISCHARGE2;
  
  memset(cap_st,0,sizeof(cap_st));
  
  memset(cap_tt,0,sizeof(cap_tt));
  
  /** 引脚初始化 **/
  
  GPIO_SetDir(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN, GPIO_DIRECTION_OUTPUT);
  
  PINSEL_SetPinMode(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PNUM,PINSEL_BASICMODE_PLAINOUT);
  
  PINSEL_SetSlewMode(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PNUM,ENABLE);

  PINSEL_SetHysMode (TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PNUM,DISABLE);

  
  GPIO_SetDir(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN, GPIO_DIRECTION_OUTPUT);
  
  PINSEL_SetPinMode(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PNUM,PINSEL_BASICMODE_PLAINOUT);
  
  PINSEL_SetSlewMode(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PNUM,ENABLE);
  
  PINSEL_SetHysMode (TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PNUM,DISABLE);

  
  
  GPIO_SetDir(TSENSOR_IN_CAP_PORT, TSENSOR_IN_CAP_PIN, GPIO_DIRECTION_INPUT);
  
  PINSEL_SetPinMode(TSENSOR_IN_CAP_PORT, TSENSOR_IN_CAP_PNUM,PINSEL_BASICMODE_PLAINOUT);  
  
  PINSEL_SetSlewMode(TSENSOR_IN_CAP_PORT, TSENSOR_IN_CAP_PNUM,ENABLE);
  
  PINSEL_SetHysMode (TSENSOR_IN_CAP_PORT, TSENSOR_IN_CAP_PNUM,DISABLE);
  /** 定时器初始化 **/
  
  tsensor_timer_init();
}

void tsensor_task(void)
{
  if( tsensor_timeout > 0)
    return;
  
  switch(sensor_ops)
  {
  case TSENSOR_DISCHARGE:
    {
      sensor_ops = TSENSOR_CHARGE_S;
      
      GPIO_SetDir(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN, GPIO_DIRECTION_INPUT);
      GPIO_SetDir(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN, GPIO_DIRECTION_OUTPUT);
      
      GPIO_OutputValue(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN,1);
      
      TSENSOR_TIMER->TC = 0;
      tsensor_timeout = 2;
    }
    break;
  case TSENSOR_CHARGE_S:     //通过标称电阻充电
    {
      sensor_ops = TSENSOR_DISCHARGE2;
      
      
      GPIO_SetDir(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN, GPIO_DIRECTION_OUTPUT);
      GPIO_SetDir(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN, GPIO_DIRECTION_OUTPUT);
      
      GPIO_OutputValue(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN,0);
      GPIO_OutputValue(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN,0);
      tsensor_timeout = 1;
      
    }
    break;
    
  case TSENSOR_DISCHARGE2:
    {
      sensor_ops = TSENSOR_CHARGE_T;
      
      GPIO_SetDir(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN, GPIO_DIRECTION_INPUT);
      GPIO_SetDir(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN, GPIO_DIRECTION_OUTPUT);

      GPIO_OutputValue(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN,1);
      
      TSENSOR_TIMER->TC = 0;
      
      tsensor_timeout = 1;
    }
    break;
  case TSENSOR_CHARGE_T:
    {
      sensor_ops = TSENSOR_DISCHARGE;
      
      GPIO_SetDir(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN, GPIO_DIRECTION_OUTPUT);
      GPIO_SetDir(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN, GPIO_DIRECTION_OUTPUT);
      
      GPIO_OutputValue(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN,0);
      GPIO_OutputValue(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN,0);
      tsensor_timeout = 4;

    }
    break;
  }
}

float tsensor_get_t(void)
{
  uint32_t sum_st = 0 ,sum_tt = 0;
  uint8_t cnt_st,cnt_tt;
  int32_t res_t;
  
  cnt_st = 0;
  cnt_tt = 0;
  
  for( int i = 0; i < 10; i++)
  {
    if( (cap_st[i] >= TSENSOR_CAP_MIN) && (cap_st[i] <= TSENSOR_CAP_MAX))
    {
      sum_st += cap_st[i];
      cnt_st ++;
    }
  }
  
  for( int i = 0; i < 10; i++)
  {
    if( (cap_tt[i] >= TSENSOR_CAP_MIN) && (cap_tt[i] <= TSENSOR_CAP_MAX))
    {
      sum_tt += cap_tt[i];
      cnt_tt ++;
    }
  }
  
  if( ( cnt_st == 0) || (cnt_tt == 0))
  {
    return 0.0f;
  }
  
    sum_tt /= cnt_tt;
    sum_st /= cnt_st;
  
  res_t =(uint32_t)( RES_ST_VALUE * 1.0f * sum_tt / sum_st);
  
  /** 2分查找 **/
  int16_t cmp_mid,cmp_low,cmp_high;

  cmp_low = 0;
  cmp_high = TSENSOR_TABLE_CNT -1;
  cmp_mid = (cmp_low + cmp_high)/2;
  
  while( cmp_low < cmp_high)
  {  
    if( res_t > tsensor_table[cmp_mid])
    {
      cmp_high = cmp_mid;
    }
    else if (res_t < tsensor_table[cmp_mid])
    {
      cmp_high--;
      cmp_low = cmp_mid;
    }
    else
    {
      break;
    }
    
    cmp_mid = (cmp_low + cmp_high)/2;
  }
  
  if( cmp_mid ==(TSENSOR_TABLE_CNT -1) )
  {
    return (float)(TSENSOR_TABLE_CNT -1-40);
  }
  else
  {
    res_t -= tsensor_table[cmp_mid];
    
    return (float)(cmp_mid - 40 + res_t * 1.0f / ( tsensor_table[cmp_mid+1] - tsensor_table[cmp_mid]));
  }

  //return  ( RES_ST_VALUE * 1.0f * (sum_tt- 42300)  / (sum_st- 41700));
  
  //timer2_start();
}


static void tsensor_timer_init(void)
{
  TIM_TIMERCFG_Type TIM_ConfigStruct;
  TIM_CAPTURECFG_Type TIM_CaptureConfigStruct;

  /** 定时器配置 -------------------------------------*/
  
  CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCTIM2, ENABLE);
  
  TSENSOR_TIMER->MCR = 0x00;
  
  //TIM_DeInit(TSENSOR_TIMER);
  //TIM_Cmd(TSENSOR_TIMER, DISABLE);
  
  TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_TICKVAL;
  TIM_ConfigStruct.PrescaleValue  = 1;
  
  // Set configuration for Tim_config and Tim_MatchConfig
  TIM_Init(TSENSOR_TIMER, TIM_TIMER_MODE, &TIM_ConfigStruct);
  
  /** 捕捉配置 ---------------------------------------*/
  
  //Config pin as CAPATURE function
  PINSEL_ConfigPin(TSENSOR_IN_CAP_PORT, TSENSOR_IN_CAP_PNUM, 3);
        
  // use channel 0
  TIM_CaptureConfigStruct.CaptureChannel = 0;

  TIM_CaptureConfigStruct.RisingEdge = ENABLE;

  TIM_CaptureConfigStruct.FallingEdge = ENABLE;
  
  // Generate capture interrupt
  TIM_CaptureConfigStruct.IntOnCaption = ENABLE;
  
  TIM_ConfigCapture(TSENSOR_TIMER, &TIM_CaptureConfigStruct);

  /** 定时器中断配置 ----------------------------------*/
  
  /* preemption = 1, sub-priority = 1 */
  NVIC_SetPriority(TSENSOR_TIM_IRQn, ((0x01<<3)|0x01));
  
  /* Enable interrupt for timer 0 */
  NVIC_EnableIRQ(TSENSOR_TIM_IRQn);
  
  /** 开定时器 ----------------------------------------*/
  
  TIM_Cmd(TSENSOR_TIMER, ENABLE);
  
}
#if 1
void TIMER2_IRQHandler(void)
{
  if ( TIM_GetIntCaptureStatus(TSENSOR_TIMER, TIM_CR0_INT))
  {
    switch(sensor_ops)
    {
    case TSENSOR_CHARGE_S:
      {
        cap_st[cap_st_idx++] = TSENSOR_TIMER->CR0;
        cap_st_idx %= CAPATURE_TIMES;
      }
      break;
  
    case TSENSOR_CHARGE_T:
      {
        cap_tt[cap_tt_idx++] = TSENSOR_TIMER->CR0; 
        cap_tt_idx %= CAPATURE_TIMES;
      }
      break;
    }
    TIM_ClearIntCapturePending(TSENSOR_TIMER, TIM_CR0_INT);
  }
}
#endif
void tsensor_time_handler(void)
{//25ms中断
  if( tsensor_timeout > 0)
    tsensor_timeout--;
}
