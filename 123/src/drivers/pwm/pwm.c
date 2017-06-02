/*
�ļ�����: pwm.c
����:
    1.����pwm������صĺ���
����: ������
�޸ļ�¼:
    2017-5-6 �ļ�����
��ע:   void
ע��:   
PWM channel 1 cannot be a double edged output.

Channel    SingleEdge PWM (PWMSELn = 0)     Double Edge PWM (PWMSELn = 1)
            Set by      Reset by            Set by      Reset by
1   Match 0     Match 1                     Match 0     Match 1//channel 1 ��֧��Double Edge
2   Match 0     Match 2                     Match 1     Match 2
3   Match 0     Match 3                     Match 2     Match 3
4   Match 0     Match 4                     Match 3     Match 4
5   Match 0     Match 5                     Match 4     Match 5
6   Match 0     Match 6                     Match 5     Match 6

1. �ļ�����ʱ����������SingleEdge �������������Ҫ���ɸ�����Ҫ�޸Ĵ���
2. ���ļ��еĲ�������Ϊ���CAP_MAX�ε�ƽ��ֵ
3. �ļ�����ʱֻ������ռ�ձ�Ϊ1:1�����
4. �ļ�����ʱδ����PWM���ʹ���жϵ����
*/
#include "include.h"

/******************************************�궨��*************************************************/
#define CAP_MAX         10

/*******************************************����**************************************************/
#pragma pack(1)
typedef const struct{// PWM0/PWM1 ������Ϣ�ṹ�嶨��
LPC_PWM_TypeDef     *PWMx;
uint32_t    ulPwrEn;
uint32_t    ulPM;//the maximum value for the PWM Prescale Counter.(Timer Counter ����������ΪPCLK * (PM + 1))
bool        bMastEn;//����ʹ�ܣ���PWM0��Ч
IRQn_Type   IRQn;
pIsrFunc    pIsr;//�жϷ�����
}stInfoDef;

typedef struct {
uint32_t *pulBuffer;
uint8_t  ucLen;
}stBufferDef;

typedef const struct{// PWM channel ������Ϣ�ṹ�嶨��
enPwmChDef   enCh;
stInfoDef    *pstInfo;
stPinCfgDef  *pstPin;
stBufferDef  *pstBuffer;
uint16_t usCCR;//�������  PWM_CCR_CAP_RISING(n) �� PWM_CCR_CAP_FALLING(n) ��PWM_CCR_INT_ON_CAP(n)�����(PWM0ֻ��һ·����PWM1����·����)
}stChInfoDef;
#pragma pack()
static void Pwm1Isr(void);

/*****************************************��������************************************************/
static uint32_t aulBuffer[1][CAP_MAX] = {0};//LPC1778���3·PWM����

static stInfoDef astInfo[] = //����LPC1778ֻ����·PWM���ʸ�����Ԫ�ظ������ó���2
{
{//pwm0
LPC_PWM0,
CLKPWR_PCONP_PCPWM0, 
60- 1,// 100k
0,//����ʹ�ܣ���PWM0��Ч
PWM0_IRQn,//δʹ���ж�
NULL,
},
{//pwm1
LPC_PWM1,
CLKPWR_PCONP_PCPWM1, 
60- 1,// 100k
0,//����ʹ�ܣ���PWM0��Ч
PWM1_IRQn,
Pwm1Isr,
},
};


static stBufferDef astBuffer[] = {
{
    &aulBuffer[0][0], 0,
}
};

const static stPinCfgDef astPinCfg[] = 
{
{
    BRD_PWM_IN_PORT, BRD_PWM_IN_PIN, BRD_PWM_IN_FUNC,
},
{
    BRD_PWM_OUT_PORT, BRD_PWM_OUT_PIN, BRD_PWM_OUT_FUNC,
}
};

static stChInfoDef astPwmChInfo[] = {
{//PWM1 channel0 ����
    PwmCh0,
    &astInfo[1],
    &astPinCfg[0],
    &astBuffer[0],
    PWM_CCR_CAP_FALLING(0) | PWM_CCR_INT_ON_CAP(0),
},
{//PWM1 channel1 ���
    PwmCh1,
    &astInfo[1],
    &astPinCfg[1],
    NULL,//������û���
    0,
},
};

/******************************************��������***********************************************/
/****************************************static��������*********************************************/
/*
����: RegInit()
����:
    1.Pwm Reg Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void 
*/
static void RegInit(stChInfoDef *pstChInfo)
{
    stInfoDef *pstInfo = pstChInfo->pstInfo;
    LPC_PWM_TypeDef *PWMx    = pstInfo->PWMx;

    LPC_SC->PCONP |= pstInfo->ulPwrEn;
    PWMx->TCR = 0;
    PWMx->TC = 0;
    PWMx->TCR |= PWM_TCR_COUNTER_RESET;//timer counter rest
    PWMx->IR  |= PWM_IR_BITMASK;//clear interrupt flg
    DelayUs(1);
    PWMx->TCR &= ~PWM_TCR_COUNTER_RESET;//timer counter rest finish
    PWMx->PR = pstInfo->ulPM;
    
    if ((PWMx == LPC_PWM0) && (pstInfo->bMastEn))
    {//ֻ��channel0����������
        PWMx->TCR |= PWM_TCR_MDIS_ENABLE;
    }
    if (pstInfo->pIsr)
    {
        IsrRegester(pstInfo->IRQn, pstInfo->pIsr);
    }
    PWMx->MCR |= PWM_MCR_RESET_ON_MATCH(0);//ÿ��channel������reset on match 0
    PWMx->CCR |= pstChInfo->usCCR;
    PWMx->TCR |= PWM_TCR_COUNTER_ENABLE;// | PWM_TCR_PWM_ENABLE;
    return;
}

/*
����: GpioInit()
����:
    1.Pwm Gpio Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void 
*/
static void GpioInit(stPinCfgDef *pstPin)
{//UART GPIO Init
    LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;
    GpioPinCfg(*pstPin);
    return;
}

/*
����: PwmInit()
����:
    1.Pwm Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void 
*/
static void PwmChInit(stChInfoDef *pstChInfo)
{
    GpioInit(pstChInfo->pstPin);
    RegInit(pstChInfo);
    return;
}

#if 0
/*
����: Pwm0Isr()
����:
    1.Pwm0 ISR
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void 
*/
static void Pwm0Isr(void)
{
    
    return;
}
#endif

/*
����: GetBuffer()
����:
    1.��ȡָ��PWM��ָ��channel�Ļ���ָ��
����:   
    1. pstPwmInfo: ָ��astPwmInfo[]�����е�Ԫ�ص�ָ��
    2. enCh: PWM channel
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void 
*/
static stBufferDef *GetBuffer(stInfoDef *pstPwmInfo, enPwmChDef enCh)
{
    uint8_t i = 0;
    stBufferDef  *pstBuffer = NULL;

    for (i = 0; i < SizeOfArray(astPwmChInfo); i++)
    {
        if ((astPwmChInfo[i].enCh == enCh) && (astPwmChInfo[i].pstInfo == pstPwmInfo))
        {
            pstBuffer = astPwmChInfo[i].pstBuffer;
            break;
        }
    }
    
    return pstBuffer;
}

/*
����: Pwm1Isr()
����:
    1.Pwm1 ISR
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void 
*/
static void Pwm1Isr(void)
{
    enPwmChDef   enCh  = PwmCh0;//channel
    stBufferDef  *pstBuffer = NULL;
    uint16_t usFlg = LPC_PWM1->IR;

    LPC_PWM1->IR  = usFlg;//clear interrupt flg
    if (usFlg & PWM_INTSTAT_CAP0)
    {
        enCh = PwmCh0;
    }
    else if (usFlg & PWM_INTSTAT_CAP1)
    {
        enCh = PwmCh1;
    }
    else
    {
        goto Return;
    }

    pstBuffer = GetBuffer(&astInfo[1], enCh);
    if (pstBuffer->pulBuffer)
    {
        pstBuffer->pulBuffer[pstBuffer->ucLen++] = LPC_PWM1->CR[enCh];
    }
    pstBuffer->ucLen %= CAP_MAX;
Return:
    return; 
}


/****************************************extern��������*********************************************/

/*
����: PwmInit()
����:
    1.Pwm Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú��������PWM��ʼ��������PWM timer counter
*/
void PwmInit(void)
{
    uint8_t i = 0;
    
    for (i = 0; i < SizeOfArray(astPwmChInfo); i++)
    {
        PwmChInit(&astPwmChInfo[i]);
    }
    return;
}

/*
����: PwmUpdata()
����:
    1.����PWM Match Registers �е�ֵ����ʹ֮��Ч
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void 
*/
void PwmUpdata(enPwmIdDef enPwmId, enPwmChDef enPwmCh)
{
    
    return;
}

/*
����: PwmStart()
����:
    1.����Pwm���
����:   
    1.enPwmId (PWM0 or PWM1)
    2.enPwmCh (channel1 - channel6)
    3.ulRate  Ƶ��(HZ)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.enPwmCh���� >= 1
*/
void PwmStart(enPwmIdDef enPwmId, enPwmChDef enPwmCh, uint32_t ulRate)
{
    LPC_PWM_TypeDef *PWMx    = astInfo[enPwmId].PWMx;
    uint32_t ulCLK = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);
        
    ulCLK /= PWMx->PR + 1;//��ȡPWMƵ��
    PWMx->TC = 0;
    PWMx->MRL[0] = ulCLK / ulRate;
    PWMx->LER  |= PWM_LER_EN_MATCHn_LATCH(0);
    if(enPwmCh > 3)
    {
        PWMx->MRH[enPwmCh] = PWMx->MRL[0] / 2;
    }
    else
    {
        PWMx->MRL[enPwmCh] = PWMx->MRL[0] / 2;
    }

    PWMx->LER  |= PWM_LER_EN_MATCHn_LATCH(enPwmCh);
    PWMx->PCR  |= PWM_PCR_PWMENAn(enPwmCh);
    return;
}

/*
����: PwmGetFreq()
����:
    1.��ȡpwm���񵽵ķ���Ƶ��ֵ
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú���ֻ�����PWM��ʼ����δ����PWM
*/
uint32_t PwmGetFreq(enPwmIdDef enPwmId, enPwmChDef enCh)
{
    uint8_t ucCnt = 0;//��¼�ڸ�����Χ�ڵĲ��ô���
    uint8_t ucLen = 0;//��ĿǰΪֹ�������ܴ���
    uint8_t i = 0;
    uint32_t *pulBuffer = 0;//ָ�򻺴��ָ��
    stBufferDef *pstBuffer = NULL;

    pstBuffer = GetBuffer(&astInfo[enPwmId], enCh);
    ucLen = pstBuffer->ucLen;
    pulBuffer = pstBuffer->pulBuffer;
    for (i = 0; i < ucLen; i++)
    {
    }
    return 0;
}
