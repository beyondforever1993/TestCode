/*
文件名称: pwm.c
功能:
    1.包含pwm驱动相关的函数
作者: 杜在连
修改记录:
    2017-5-6 文件初创
备注:   void
注意:   
PWM channel 1 cannot be a double edged output.

Channel    SingleEdge PWM (PWMSELn = 0)     Double Edge PWM (PWMSELn = 1)
            Set by      Reset by            Set by      Reset by
1   Match 0     Match 1                     Match 0     Match 1//channel 1 不支持Double Edge
2   Match 0     Match 2                     Match 1     Match 2
3   Match 0     Match 3                     Match 2     Match 3
4   Match 0     Match 4                     Match 3     Match 4
5   Match 0     Match 5                     Match 4     Match 5
6   Match 0     Match 6                     Match 5     Match 6

1. 文件初创时，仅考虑了SingleEdge 的情况，如有需要，可根据需要修改代码
2. 该文件中的捕获数据为最大CAP_MAX次的平均值
3. 文件初创时只考虑了占空比为1:1的情况
4. 文件初创时未考虑PWM输出使能中断的情况
*/
#include "include.h"

/******************************************宏定义*************************************************/
#define CAP_MAX         10

/*******************************************声明**************************************************/
#pragma pack(1)
typedef const struct{// PWM0/PWM1 配置信息结构体定义
LPC_PWM_TypeDef     *PWMx;
uint32_t    ulPwrEn;
uint32_t    ulPM;//the maximum value for the PWM Prescale Counter.(Timer Counter 递增的周期为PCLK * (PM + 1))
bool        bMastEn;//主从使能，仅PWM0有效
IRQn_Type   IRQn;
pIsrFunc    pIsr;//中断服务函数
}stInfoDef;

typedef struct {
uint32_t *pulBuffer;
uint8_t  ucLen;
}stBufferDef;

typedef const struct{// PWM channel 配置信息结构体定义
enPwmChDef   enCh;
stInfoDef    *pstInfo;
stPinCfgDef  *pstPin;
stBufferDef  *pstBuffer;
uint16_t usCCR;//捕获控制  PWM_CCR_CAP_RISING(n) 、 PWM_CCR_CAP_FALLING(n) 、PWM_CCR_INT_ON_CAP(n)的组合(PWM0只有一路捕获，PWM1有两路捕获)
}stChInfoDef;
#pragma pack()
static void Pwm1Isr(void);

/*****************************************变量定义************************************************/
static uint32_t aulBuffer[1][CAP_MAX] = {0};//LPC1778最多3路PWM捕获

static stInfoDef astInfo[] = //由于LPC1778只有两路PWM，故该数组元素个数不得超过2
{
{//pwm0
LPC_PWM0,
CLKPWR_PCONP_PCPWM0, 
60- 1,// 100k
0,//主从使能，仅PWM0有效
PWM0_IRQn,//未使能中断
NULL,
},
{//pwm1
LPC_PWM1,
CLKPWR_PCONP_PCPWM1, 
60- 1,// 100k
0,//主从使能，仅PWM0有效
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
{//PWM1 channel0 捕获
    PwmCh0,
    &astInfo[1],
    &astPinCfg[0],
    &astBuffer[0],
    PWM_CCR_CAP_FALLING(0) | PWM_CCR_INT_ON_CAP(0),
},
{//PWM1 channel1 输出
    PwmCh1,
    &astInfo[1],
    &astPinCfg[1],
    NULL,//输出不用缓存
    0,
},
};

/******************************************函数定义***********************************************/
/****************************************static函数定义*********************************************/
/*
名称: RegInit()
功能:
    1.Pwm Reg Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void 
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
    {//只有channel0有主从配置
        PWMx->TCR |= PWM_TCR_MDIS_ENABLE;
    }
    if (pstInfo->pIsr)
    {
        IsrRegester(pstInfo->IRQn, pstInfo->pIsr);
    }
    PWMx->MCR |= PWM_MCR_RESET_ON_MATCH(0);//每个channel都必须reset on match 0
    PWMx->CCR |= pstChInfo->usCCR;
    PWMx->TCR |= PWM_TCR_COUNTER_ENABLE;// | PWM_TCR_PWM_ENABLE;
    return;
}

/*
名称: GpioInit()
功能:
    1.Pwm Gpio Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void 
*/
static void GpioInit(stPinCfgDef *pstPin)
{//UART GPIO Init
    LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;
    GpioPinCfg(*pstPin);
    return;
}

/*
名称: PwmInit()
功能:
    1.Pwm Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void 
*/
static void PwmChInit(stChInfoDef *pstChInfo)
{
    GpioInit(pstChInfo->pstPin);
    RegInit(pstChInfo);
    return;
}

#if 0
/*
名称: Pwm0Isr()
功能:
    1.Pwm0 ISR
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void 
*/
static void Pwm0Isr(void)
{
    
    return;
}
#endif

/*
名称: GetBuffer()
功能:
    1.获取指定PWM的指定channel的缓存指针
参数:   
    1. pstPwmInfo: 指向astPwmInfo[]数组中的元素的指针
    2. enCh: PWM channel
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void 
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
名称: Pwm1Isr()
功能:
    1.Pwm1 ISR
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void 
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


/****************************************extern函数定义*********************************************/

/*
名称: PwmInit()
功能:
    1.Pwm Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数完成了PWM初始化，启动PWM timer counter
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
名称: PwmUpdata()
功能:
    1.更改PWM Match Registers 中的值，并使之有效
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void 
*/
void PwmUpdata(enPwmIdDef enPwmId, enPwmChDef enPwmCh)
{
    
    return;
}

/*
名称: PwmStart()
功能:
    1.启动Pwm输出
参数:   
    1.enPwmId (PWM0 or PWM1)
    2.enPwmCh (channel1 - channel6)
    3.ulRate  频率(HZ)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.enPwmCh必须 >= 1
*/
void PwmStart(enPwmIdDef enPwmId, enPwmChDef enPwmCh, uint32_t ulRate)
{
    LPC_PWM_TypeDef *PWMx    = astInfo[enPwmId].PWMx;
    uint32_t ulCLK = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);
        
    ulCLK /= PWMx->PR + 1;//获取PWM频率
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
名称: PwmGetFreq()
功能:
    1.获取pwm捕获到的方波频率值
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数只完成了PWM初始化，未启动PWM
*/
uint32_t PwmGetFreq(enPwmIdDef enPwmId, enPwmChDef enCh)
{
    uint8_t ucCnt = 0;//记录在给定范围内的采用次数
    uint8_t ucLen = 0;//到目前为止采样的总次数
    uint8_t i = 0;
    uint32_t *pulBuffer = 0;//指向缓存的指针
    stBufferDef *pstBuffer = NULL;

    pstBuffer = GetBuffer(&astInfo[enPwmId], enCh);
    ucLen = pstBuffer->ucLen;
    pulBuffer = pstBuffer->pulBuffer;
    for (i = 0; i < ucLen; i++)
    {
    }
    return 0;
}
