#include "bsp_adc.h"
#include <stdlib.h>
#include <string.h>
#include "File.h"


static  volatile uint32_t adc_conv_value[ADC_CH_COUNT * ADC_SMP_TIME];

///** ad值 -> 电压(V) **/
//const float  adc_vol_ratio[]=
//{
//  5.475/4096,
//  5.475/4096,
//  5.475/4096,
//  3.285*12/4096,
//};
//
///** ad值 -> 电流(mA) **/
//
//const float  adc_cur_ratio[]=
//{
//  21.9/4096,
//  21.9/4096,
//  21.9/4096,
//  21.9/4096,
//};

/** ad值 -> 电压(V) **/
const float  adc_vol_ratio[]=
{
  5.5/4096,
  5.5/4096,
  5.5/4096,
  3.3*12/4096,
};

/** ad值 -> 电流(mA) **/

const float  adc_cur_ratio[]=
{
  22.0/4096,
  22.0/4096,
  22.0/4096,
  22.0/4096,
};

const uint8_t adc_map[]=
{
  0,3,2,1,4
};



static uint32_t adc0_time_out;// channel 0 
static uint32_t adc1_time_out;// channel 1
static uint32_t adc2_time_out;// channel 2

adc_para_t adc0_para,adc1_para,adc2_para;

uint32_t adc_dma_tc =0;

static GPDMA_Channel_CFG_Type ADC_GPDMACfg;

void adc_init(void)
{
  
  /** 变量初始化 **/
  memcpy(&adc0_para,&g_Byte128[32+137],32);
  memcpy(&adc1_para,&g_Byte128[32+169],32);
  memcpy(&adc2_para,&g_Byte128[32+201],32);
  
  adc0_time_out = adc0_para.frq * 2400;
  adc1_time_out = adc1_para.frq * 2400;
  adc2_time_out = adc2_para.frq * 2400;
  
  /** GPIO 初始化 **/
  CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCGPIO, ENABLE);
  
  
  /** ADC时钟及引脚初始化 ----------------------------------------------------*/
  
  //ADC_DeInit(LPC_ADC);
  
  /** ADC1 **/
  PINSEL_ConfigPin(BSP_ADC1_PORT, BSP_ADC1_PIN, BSP_ADC1_FUNC);
  
  PINSEL_SetAnalogPinMode(BSP_ADC1_PORT, BSP_ADC1_PIN,ENABLE);
  
  /** ADC2 **/
  PINSEL_ConfigPin(BSP_ADC2_PORT, BSP_ADC2_PIN, BSP_ADC2_FUNC);
  
  PINSEL_SetAnalogPinMode(BSP_ADC2_PORT, BSP_ADC2_PIN, ENABLE);
  
  /** ADC3 **/
  PINSEL_ConfigPin(BSP_ADC3_PORT, BSP_ADC3_PIN, BSP_ADC3_FUNC);
  
  PINSEL_SetAnalogPinMode(BSP_ADC3_PORT, BSP_ADC3_PIN, ENABLE);
  
  /** ADC4 **/
  PINSEL_ConfigPin(BSP_ADC4_PORT, BSP_ADC4_PIN, BSP_ADC4_FUNC);
  
  PINSEL_SetAnalogPinMode(BSP_ADC4_PORT, BSP_ADC4_PIN, ENABLE);
  
  PINSEL_ConfigPin(0, 23, 1);
  PINSEL_SetAnalogPinMode(0,23,ENABLE);
  
  PINSEL_ConfigPin(0, 24, 1);
  PINSEL_SetAnalogPinMode(0,24,ENABLE);
  
  PINSEL_ConfigPin(0, 25, 1);
  PINSEL_SetAnalogPinMode(0,25,ENABLE);
  //  
  PINSEL_ConfigPin(0, 26, 1);
  PINSEL_SetAnalogPinMode(0,26,ENABLE);
  
  /** ADC模块初始化 **/
  
  ADC_Init(LPC_ADC, 100000);
  
  //  ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
  //  ADC_IntConfig(LPC_ADC, ADC_ADINTEN1, ENABLE);
  //  ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, ENABLE);
  //  ADC_IntConfig(LPC_ADC, ADC_ADINTEN3, ENABLE);
  
  ADC_ChannelCmd(LPC_ADC, 0, ENABLE);
  ADC_ChannelCmd(LPC_ADC, 1, ENABLE);
  ADC_ChannelCmd(LPC_ADC, 2, ENABLE);
  ADC_ChannelCmd(LPC_ADC, 3, ENABLE);
  
  /** GPDMA 初始化 ----------------------------------------------------------*/
  
  /* Disable GPDMA interrupt */
  NVIC_DisableIRQ(DMA_IRQn);
  
  /* Initialize GPDMA controller */
  GPDMA_Init();
 
  // channel 
  ADC_GPDMACfg.ChannelNum = ADC_DMA_CH;
  // Source memory - unused
  ADC_GPDMACfg.SrcMemAddr = 0;
  // Destination memory
  ADC_GPDMACfg.DstMemAddr = (uint32_t) adc_conv_value;
  // Transfer size
  ADC_GPDMACfg.TransferSize = ADC_CH_COUNT * ADC_SMP_TIME;
  // Transfer width - unused
  ADC_GPDMACfg.TransferWidth = 0;
  // Transfer type
  ADC_GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_P2M;
  // Source connection
  ADC_GPDMACfg.SrcConn = GPDMA_CONN_ADC;
  // Destination connection - unused
  ADC_GPDMACfg.DstConn = 0;
  // Linker List Item - unused
  ADC_GPDMACfg.DMALLI = 0;
  
  GPDMA_Setup(&ADC_GPDMACfg);
  
  /** GPDMA 中断配置 **/
  /* preemption = 0, sub-priority = 1 */
  NVIC_SetPriority(DMA_IRQn, ((0x00<<3)|0x01));
  
  /* Enable GPDMA interrupt */
  NVIC_EnableIRQ(DMA_IRQn);
  
  
  // Enable GPDMA channel 
  GPDMA_ChannelCmd(ADC_DMA_CH, ENABLE);
  
  /** ADC 启动 **/
  //ADC_StartCmd(LPC_ADC, ADC_START_CONTINUOUS);
  ADC_BurstCmd(LPC_ADC, ENABLE);
  
}
/**
通道取值1 2 3 4 
**/
uint16_t adc_get_val(unsigned char ch)
{
  uint32_t temp ;
  uint8_t i;
  uint8_t valid_count = 0;
  
  uint32_t result = 0;
  
  
  if( (ch == 0)|| (ch > 4) )
  {
    return 0;//通道号错误
  }
  
  adc_dma_tc = 0;
  
  GPDMA_Setup(&ADC_GPDMACfg);
  
  GPDMA_ChannelCmd(ADC_DMA_CH, ENABLE);
  
  /* Wait for GPDMA processing complete */
  while ((adc_dma_tc == 0));
  
  /** TODO:修改为使用信号量 **/
  
  GPDMA_ChannelCmd(ADC_DMA_CH, DISABLE);
  
  
  for( i = 0; i < ADC_CH_COUNT * ADC_SMP_TIME; i++)
  {
    temp = adc_conv_value[i];
    
    if( ADC_GDR_DONE_FLAG & temp)
    {
      if( (ADC_GDR_CH(temp) == (ch-1)) && !(ADC_GDR_OVERRUN_FLAG & temp) )
      {
        valid_count++;
        
        result += ADC_GDR_RESULT(temp);
      }
    }
  }
  
  if( valid_count != 0)
  {
    return (uint16_t)(result/valid_count);
  }
  
  return 0;
  
}

/**
获取电压值,单位: V(伏特)
通道取值1 2 3 4 

**/

float adc_get_voltage(unsigned char ch)
{
  if( (ch == 0)|| (ch > 4) )
  {
    return 0.0f;//通道号错误
  }
  
  return (float)(adc_get_val(adc_map[ch]) * adc_vol_ratio[ch - 1]);
}

/**
获取电流值,单位：mA

通道取值1 2 3 4 

**/
float adc_get_current(unsigned char ch)
{
  if( (ch == 0)|| (ch > 4) )
  {
    return 0.0f;//通道号错误
  }
  
  return (float)(adc_get_val(adc_map[ch]) * adc_cur_ratio[ch - 1]);
}


void adc_task(void)
{
  float adc_val;
  
  struct DATA_STRUCT dat;
  struct TIME_STRUCT time_now;
  
  if( adc0_para.frq_changed)
  {
    adc0_para.frq_changed = 0;
    
    adc0_time_out = adc0_para.frq * 2400;
  }
  
  if( adc1_para.frq_changed)
  {
    adc1_para.frq_changed = 0;
    
    adc1_time_out = adc1_para.frq * 2400;
  }
  
  if( adc2_para.frq_changed)
  {
    adc2_para.frq_changed = 0;
    
    adc2_time_out = adc2_para.frq * 2400;
  }
  
  /** ADC0 TASK -------------------------------------------------*/
  if( adc0_time_out == 0)
  {
    if( adc0_para.frq != 0)
    {
      adc0_time_out = adc0_para.frq * 2400;
      if( adc0_para.type != 0)
      {
        
        rtc_get_time(&time_now);
        
        if( (time_now.y < 2015) || (time_now.y > 2100))
        {
          return;
        }
        
        dat.y = time_now.y;
        dat.m = time_now.m;
        dat.d = time_now.d;
        dat.H = time_now.H;
        dat.M = time_now.M;
        dat.S = time_now.S;
        
        if( adc0_para.stype == 1)
        {//电压型
          adc_val = adc_get_voltage(1);
        }
        else if( adc0_para.stype == 2)
        {//电流型
          adc_val = adc_get_current(1);
        }
        else
        {
          return;
        }
        
        if( ((uint32_t)(adc_val *1000)) < 1)
        {//加入0.001V/0.001mA下限限制
          return;
        }
        
        dat.type = adc0_para.type;
        
        if(adc0_para.type == 5)
        {//土壤水分计
          //sprintf(dat.data, "%s,0,%c%d.%03d",adc0_para.sname,(adc_val > 0?'+':'-'),abs(adc_val),abs(adc_val * 1000) % 1000);
          sprintf(dat.data, "%s,0,%d.%03d,%c",adc0_para.sname,
                  abs(adc_val),
                   abs(adc_val*1000),
                   adc0_para.para0.c[0]);
        }
        else
        {
          sprintf(dat.data, "%s,0,%d.%03d,%d.%03d,%d.%03d",adc0_para.sname,
                  abs(adc_val),
                  abs(adc_val*1000),
                  abs(adc0_para.para0.f),
                  abs(adc0_para.para0.f*1000),
                  abs(adc0_para.para1),
                  abs(adc0_para.para1*100));
        }
        g_RtuStatus.led_dwload = 1;
        file_write(&dat);
      }
    }
  }
  
  /** ADC1 TASK -------------------------------------------------*/
  if( adc1_time_out == 0)
  {
    if( adc1_para.frq != 0)
    {
      adc1_time_out = adc1_para.frq * 2400;
      if( adc1_para.type != 0)
      {
        
        rtc_get_time(&time_now);
        
        if( (time_now.y < 2015) || (time_now.y > 2100))
        {
          return;
        }
        
        dat.y = time_now.y;
        dat.m = time_now.m;
        dat.d = time_now.d;
        dat.H = time_now.H;
        dat.M = time_now.M;
        dat.S = time_now.S;
        
        if( adc1_para.stype == 1)
        {//电压型
          adc_val = adc_get_voltage(2);
        }
        else if( adc1_para.stype == 2)
        {//电流型
          adc_val = adc_get_current(2);
        }
        else
        {
          return;
        }
        
        if( ((uint32_t)(adc_val *1000)) < 1)
        {//加入0.001V/0.001mA下限限制
          return;
        }
        
        dat.type = adc1_para.type;
        
        if(adc1_para.type == 5)
        {//土壤水分计
          //sprintf(dat.data, "%s,0,%c%d.%03d",adc1_para.sname,(adc_val > 0?'+':'-'),abs(adc_val),abs(adc_val * 1000) % 1000);
                sprintf(dat.data, "%s,1,%d.%03d,%c",adc1_para.sname,
                  abs(adc_val),
                   abs(adc_val*1000),
                   adc1_para.para0.c[0]);
        }
        else
        {
          sprintf(dat.data, "%s,1,%d.%03d,%d.%03d,%d.%03d",adc1_para.sname,
                  abs(adc_val),
                  abs(adc_val*1000),
                  abs(adc1_para.para0.f),
                  abs(adc1_para.para0.f*1000),
                  abs(adc1_para.para1),
                  abs(adc1_para.para1*100));
        }
        
        g_RtuStatus.led_dwload = 1;
        file_write(&dat);
      }
    }
  }
  
  /** ADC2 TASK -------------------------------------------------*/
  if( adc2_time_out == 0)
  {
    if( adc2_para.frq != 0)
    {
      adc2_time_out = adc2_para.frq * 2400;
      if( adc2_para.type != 0)
      {
        rtc_get_time(&time_now);
        
        if( (time_now.y < 2015) || (time_now.y > 2100))
        {
          return;
        }
        
        dat.y = time_now.y;
        dat.m = time_now.m;
        dat.d = time_now.d;
        dat.H = time_now.H;
        dat.M = time_now.M;
        dat.S = time_now.S;
        
        if( adc2_para.stype == 1)
        {//电压型
          adc_val = adc_get_voltage(3);
        }
        else if( adc2_para.stype == 2)
        {//电流型
          adc_val = adc_get_current(3);
        }
        else
        {
          return;
        }
        
        if( ((uint32_t)(adc_val *1000)) < 1)
        {//加入0.001V/0.001mA下限限制
          return;
        }
        
        dat.type = adc2_para.type;
        
        if(adc2_para.type == 5)
        {//土壤水分计
          //sprintf(dat.data, "%s,0,%c%d.%03d",adc2_para.sname,(adc_val > 0?'+':'-'),abs(adc_val),abs(adc_val * 1000) % 1000);
          sprintf(dat.data, "%s,2,%d.%03d,%c",adc2_para.sname,
                  abs(adc_val),
                   abs(adc_val*1000),
                   adc2_para.para0.c[0]);
        }
        else
        {
          sprintf(dat.data, "%s,2,%d.%03d,%d.%03d,%d.%03d",adc2_para.sname,
                  abs(adc_val),
                  abs(adc_val*1000),
                  abs(adc2_para.para0.f),
                  abs(adc2_para.para0.f*1000)%1000,
                  abs(adc2_para.para1),
                  abs(adc2_para.para1*1000)%1000);
        }
        
        g_RtuStatus.led_dwload = 1;
        file_write(&dat);
      }
    }
  }
  
  
  
}

void adc_time_handler(void)
{//25ms 中断
  if( adc0_time_out > 0)
    adc0_time_out--;
  
  if( adc1_time_out > 0)
    adc1_time_out--;
  
  if( adc2_time_out > 0)
    adc2_time_out--;
}


void ADC_DMA_IRQHandler (void)
{
  static uint32_t adc_dma_err;
  
  // Check counter terminal status
  if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, ADC_DMA_CH))
  {
    // Clear terminate counter Interrupt pending
    GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, ADC_DMA_CH);
    
    adc_dma_tc++;
  }
  
  // Check error terminal status
  if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, ADC_DMA_CH))
  {
    // Clear error counter Interrupt pending
    GPDMA_ClearIntPending (GPDMA_STATCLR_INTERR, ADC_DMA_CH);
    
    adc_dma_err++;
  }
}
