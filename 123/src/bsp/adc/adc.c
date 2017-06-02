/*
文件名称: ad.c
功能:
    1.包含ad驱动相关的函数
作者: 杜在连
修改记录:
    2017-4-30 文件初创
备注:   void
注意:   
    1. 文件初创时只考虑到了software-controlled mode的应用场景，故每次只能执行1 channel的ADC转换,且必须由上层软件触发
    2. 文件初创时未考虑使用ADC中断的情况
*/
#include "include.h"



/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/

/*****************************************变量定义************************************************/
stPinCfgDef astAdcGpio[] = {//定义各channel的GPIO配置参数
{//channel0
    BRD_AD0_PORT,
    BRD_AD0_PIN,
    BRD_AD0_FUNC | IOCON_ANALOG_MODE,
},
{//channel1
    BRD_AD1_PORT,
    BRD_AD1_PIN,
    BRD_AD1_FUNC | IOCON_ANALOG_MODE,
},
{
    BRD_AD2_PORT,
    BRD_AD2_PIN,
    BRD_AD2_FUNC | IOCON_ANALOG_MODE,
},
{
    BRD_AD3_PORT,
    BRD_AD3_PIN,
    BRD_AD3_FUNC | IOCON_ANALOG_MODE,
},

};

/******************************************函数定义***********************************************/
/****************************************static函数定义*********************************************/
#define SetCh(ch)           do{LPC_ADC->CR &= ~ADC_CR_CH_MASK; LPC_ADC->CR |= ADC_CR_CH_SEL(ch);}while(0)
#define StartCh(ch)         do{SetCh(ch); LPC_ADC->CR |= ADC_CR_START_NOW;}while(0)//启动指定Channel的ADC
#define WaitEnd(ch)         do{while(0 == (LPC_ADC->DR[ch] & ADC_DR_DONE_FLAG));}while(0)//等待转换完成
#define GetVal(ch)          (ADC_GDR_RESULT(LPC_ADC->DR[ch]))

/*
名称: GpioInit()
功能:
    1.AD Gpio Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void GpioInit(void)
{
    uint8_t i = 0;

    for (i = 0; i < SizeOfArray(astAdcGpio); i++)
    {
        GpioPinCfg(astAdcGpio[i]);
    }
    return;
}

/****************************************extern函数定义*********************************************/

/*
名称: AdInit()
功能:
    1.AD Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数仅完成ADC寄存器的初始化，未启动任何Channel的ADC
*/
void AdcInit(void)
{
    uint32_t ADCPClk = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);
    
    LPC_SC->PCONP |= CLKPWR_PCONP_PCADC;
    ADCPClk     = ADCPClk / 10400000 + 1;//获取12.4M的分频值
    LPC_ADC->CR = ADC_CR_PDN | ADC_CR_CLKDIV(ADCPClk);//ADC power on 未启动任何channel，软件控制模式, CAP/MAT rising edge触发
    GpioInit();
    return;
}

/*
名称: AdcGetVal()
功能:
    1. 开启channel的AD转换，
    2. 等待转换完成
    3. 输出结果
参数:   
    1. enCh: ADC channel
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
uint16_t AdcGetVal(enAdcChDef enCh)
{
    StartCh(enCh);
    WaitEnd(enCh);
    return GetVal(enCh);
}

/*
名称: GatVoltage()
功能:
    1. 获取电源电压
参数:   void
返回值: 
    电源电压
输入:   void
输出:   void
备注:   void
注意:   
    1.本函数返回的值为实际电源电压值 * 100
*/
uint32_t GatVoltage(void)
{
    uint32_t ulRes = 0;

    ulRes = AdcGetVal(AdcCh4);
    ulRes = (12 * ulRes * 100) / 0xfff;//
    return ulRes;
}

