/*
�ļ�����: ad.c
����:
    1.����ad������صĺ���
����: ������
�޸ļ�¼:
    2017-4-30 �ļ�����
��ע:   void
ע��:   
    1. �ļ�����ʱֻ���ǵ���software-controlled mode��Ӧ�ó�������ÿ��ֻ��ִ��1 channel��ADCת��,�ұ������ϲ��������
    2. �ļ�����ʱδ����ʹ��ADC�жϵ����
*/
#include "include.h"



/******************************************�궨��*************************************************/

/*******************************************����**************************************************/

/*****************************************��������************************************************/
stPinCfgDef astAdcGpio[] = {//�����channel��GPIO���ò���
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

/******************************************��������***********************************************/
/****************************************static��������*********************************************/
#define SetCh(ch)           do{LPC_ADC->CR &= ~ADC_CR_CH_MASK; LPC_ADC->CR |= ADC_CR_CH_SEL(ch);}while(0)
#define StartCh(ch)         do{SetCh(ch); LPC_ADC->CR |= ADC_CR_START_NOW;}while(0)//����ָ��Channel��ADC
#define WaitEnd(ch)         do{while(0 == (LPC_ADC->DR[ch] & ADC_DR_DONE_FLAG));}while(0)//�ȴ�ת�����
#define GetVal(ch)          (ADC_GDR_RESULT(LPC_ADC->DR[ch]))

/*
����: GpioInit()
����:
    1.AD Gpio Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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

/****************************************extern��������*********************************************/

/*
����: AdInit()
����:
    1.AD Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú��������ADC�Ĵ����ĳ�ʼ����δ�����κ�Channel��ADC
*/
void AdcInit(void)
{
    uint32_t ADCPClk = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);
    
    LPC_SC->PCONP |= CLKPWR_PCONP_PCADC;
    ADCPClk     = ADCPClk / 10400000 + 1;//��ȡ12.4M�ķ�Ƶֵ
    LPC_ADC->CR = ADC_CR_PDN | ADC_CR_CLKDIV(ADCPClk);//ADC power on δ�����κ�channel���������ģʽ, CAP/MAT rising edge����
    GpioInit();
    return;
}

/*
����: AdcGetVal()
����:
    1. ����channel��ADת����
    2. �ȴ�ת�����
    3. ������
����:   
    1. enCh: ADC channel
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
uint16_t AdcGetVal(enAdcChDef enCh)
{
    StartCh(enCh);
    WaitEnd(enCh);
    return GetVal(enCh);
}

/*
����: GatVoltage()
����:
    1. ��ȡ��Դ��ѹ
����:   void
����ֵ: 
    ��Դ��ѹ
����:   void
���:   void
��ע:   void
ע��:   
    1.���������ص�ֵΪʵ�ʵ�Դ��ѹֵ * 100
*/
uint32_t GatVoltage(void)
{
    uint32_t ulRes = 0;

    ulRes = AdcGetVal(AdcCh4);
    ulRes = (12 * ulRes * 100) / 0xfff;//
    return ulRes;
}

