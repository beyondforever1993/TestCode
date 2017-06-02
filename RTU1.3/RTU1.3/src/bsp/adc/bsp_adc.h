#ifndef BSP_ADC_H
#define BSP_ADC_H
#include "board_gpio.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_adc.h"
#include "lpc177x_8x_gpdma.h"
#include "string.h"
#include "lpc177x_8x_clkpwr.h"
#include "RTC.h"
#include <math.h>


typedef struct 
{
  uint8_t port; // 0,1,2,...
  uint32_t pin; // bit0 -> 1<<0 bit1 -> 1<<1 ...
  uint8_t dir ; // GPIO_DIRECTION_INPUT  GPIO_DIRECTION_OUTPUT
}gpio_t;

typedef struct 
{
  uint32_t type;
  uint32_t frq;
  uint32_t stype;//传感器类型: 1:电压型 2:电流型
  union
  {
    float f;
    char c[4];
  }para0;//x^0
  float para1;//x^1
  float para2;//x^2
  float para3;//x^3
  float para4;//x^4
  char  sname[21];
  
  uint8_t frq_changed;
    
}adc_para_t;

#define BSP_ADC1_PORT     0
#define BSP_ADC1_PIN      23
#define BSP_ADC1_FUNC     1
                          
#define BSP_ADC2_PORT     0
#define BSP_ADC2_PIN      24
#define BSP_ADC2_FUNC     1
                          
#define BSP_ADC3_PORT     0
#define BSP_ADC3_PIN      25
#define BSP_ADC3_FUNC     1
                          
#define BSP_ADC4_PORT     0
#define BSP_ADC4_PIN      26
#define BSP_ADC4_FUNC     1


#define ADC_CH_COUNT 4

#define ADC_DMA_CH  2

#define ADC_SMP_TIME  10

extern uint8_t g_Byte128[];
extern adc_para_t adc0_para,adc1_para,adc2_para;

void adc_init(void);

uint16_t adc_get_val(unsigned char ch);

float adc_get_voltage(unsigned char ch);

float adc_get_current(unsigned char ch);

void adc_task(void);

void adc_time_handler(void);

void ADC_DMA_IRQHandler (void);


#endif
