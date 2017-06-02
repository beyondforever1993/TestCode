#include "include.h"
/*
文件名称: timer.c
功能:
    1.Timer 相关的函数
作者: 杜在连
修改记录:
    2017-4-3    文件初创
    2017-4-30   添加支持输入捕获的功能
    2017-5-2    用TimerIdDef TimerId替换原有函数中的LPC_TIM_TypeDef *TIMx
备注:
    1.该文件中的函数具体用法如下:
        一. 在stTimerInfo[]中根据需要添加timer相关的配置信息,若使能中断,需要在astIRQnInfo[]中添加相关配置信息
        二. 在BSP init函数中调用TimerInit()函数
        三. 在需要时，通过调用TimerStartCh()函数启动指定Channel的定时器，可以调用TimerReFresh()函数刷新定时器(类似喂狗)
        四. 调用TimerIsOut()即可判断是否已超时
        五. 用作捕获时，该文件暂未提供ISR
注意: 
    1. 根据项目需要，timer暂时只用作定时及输入捕获,若后续需要其他功能,可根据需要完善代码
    2. 现有程序默认 TC will be reset if MR matches it,并且只考虑了中断和AutoStop的情况,如有需要可根据需要完善代码
    3. timer0-timer3有各有四个channel,每个timer的各channel有各自独立的match Register,其余寄存器为各channel共用,
       为避免影响其他channel,不到万不得已，不得动公用寄存器
    4. 若上层程序调用TimerIsrReg()函数注册了上层指定的Timer Isr,则 TimerIsOut()功能将不可用，且必须在指定中断中调用TimerGetCh()以清除中断请求标志
*/

/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/
typedef enum{
Redge = 1,//rising edge
Fedge = 2,//falling edge
Bedge = 3,//both edge
}enEdgeDef;


#pragma pack(1)

typedef struct{
stPinCfgDef stPinCfg;
enEdgeDef   enEde;
bool        bIrEn;//是否使能捕获中断
}stPinInfoDef;

typedef const struct{
LPC_TIM_TypeDef * TIMx; //timer寄存器指针(LPC_TIM2 etc.)
TimerChDef TimerCh; //timer channel(TimerCH0 etc.)  注意:若用作捕获，则该字段最大为TimerCH1
uint32_t ulPwrEn; //Power enable flag(CLKPWR_PCONP_PCTIM0 etc.)
TIM_PRESCALE_OPT enPresOpt;/*Timer/Counter prescale option(us or match Val)*/
uint32_t ulPresValue; // 当enPreOpt为TIM_PRESCALE_TICKVAL时，该字段为设置到PC中的预分频值(1 - 0xffffffff)
                            // 当enPreOpt为TIM_PRESCALE_USVAL时，该字段与ulMatchValue一起决定触发中断的间隔(us) (ulPresValue * ulMatchValue)
IRQn_Type IRQn;//timer Interrupt Numbers(TIMER0_IRQn etc.)
pIsrFunc IsrFunc;//用作捕获时，该字段无效。若使能中断，需要上层通过调用TimerIsrReg()函数注册ISR
bool bAutoStop;//Auto Stop enable(true or false)
stPinInfoDef *pstPin;//若该字段非NULL，则该Channel用作捕获,否则用作定时
}stTimerInfoDef;
#pragma pack()

/*****************************************变量定义************************************************/
static volatile uint16_t usTimerFlg;//各channel延时时间到的bit标志 

static stPinInfoDef astPinInfo[] = {//输入引脚相关信息
{//热敏电阻
     {TSENSOR_IN_CAP_PORT, TSENSOR_IN_CAP_PIN, TSENSOR_IN_CAP_FUNC,},
     Bedge,//捕获触发edge
     true,//捕获中断使能
}
};

/******************************************函数定义***********************************************/
/****************************************static函数定义*********************************************/
/*Timer ISR 定义*/
#define TimerxISR(n)     void Timer##n##ISR(void)\
                         {\
                            uint32_t ulTmp = LPC_TIM##n->IR & 0x0f;\
                            LPC_TIM##n->IR = ulTmp;\
                            usTimerFlg |= (ulTmp << (TimerCHMax * n));\
                         }
TimerxISR(0)

static LPC_TIM_TypeDef *const pstTimerReg[] = {
LPC_TIM0,
LPC_TIM1,
LPC_TIM2,
LPC_TIM3,
};

static stTimerInfoDef stTimerInfo[] = {
    {//timer0, channel0
        /*timer 寄存器指针*/  /*timer channel*/ /*clock enable flag*/  /*prescale option*/ /*Prescale value*/  
        LPC_TIM0,              UART0_TIMER,      CLKPWR_PCONP_PCTIM0,   TIM_PRESCALE_USVAL, 1000,                
        /*timer Interrupt Numbers*/ /*Timer Isr*/  /*Auto Stop enable*/    /*用作定时*/
          TIMER0_IRQn,                Timer0ISR,        false,               NULL,
    }, 
    {//timer0, channel1
        /*timer 寄存器指针*/  /*timer channel*/ /*clock enable flag*/  /*prescale option*/ /*Prescale value*/  
        LPC_TIM0,              UART1_TIMER,       CLKPWR_PCONP_PCTIM0,   TIM_PRESCALE_USVAL, 1000,                
        /*timer Interrupt Numbers*/  /*Timer Isr*/ /*Auto Stop enable*/   /*用作定时*/
          TIMER0_IRQn,                 Timer0ISR,       false,               NULL,
    },
    
    {//timer0, channel2
        /*timer 寄存器指针*/  /*timer channel*/ /*clock enable flag*/  /*prescale option*/ /*Prescale value*/  
        LPC_TIM0,               UART2_TIMER,       CLKPWR_PCONP_PCTIM0,   TIM_PRESCALE_USVAL, 1000,                
        /*timer Interrupt Numbers*/ /*Timer Isr*/  /*Auto Stop enable*/    /*用作定时*/
          TIMER0_IRQn,                Timer0ISR,          false,             NULL,
    },
    {//timer0, channel3
        /*timer 寄存器指针*/  /*timer channel*/ /*clock enable flag*/  /*prescale option*/ /*Prescale value*/  
        LPC_TIM0,               UART3_TIMER,       CLKPWR_PCONP_PCTIM0,   TIM_PRESCALE_USVAL, 1000,                
        /*timer Interrupt Numbers*/ /*Timer Isr*/  /*Auto Stop enable*/    /*用作定时*/
          TIMER0_IRQn,                Timer0ISR,          false,             NULL,
    },
    {//timer2, channel0
        /*timer 寄存器指针*/  /*timer channel*/ /*clock enable flag*/  /*prescale option*/      /*Prescale value*/  
        LPC_TIM2,               TEMP_TIMER,       CLKPWR_PCONP_PCTIM2,   TIM_PRESCALE_TICKVAL,      1,                
        /*timer Interrupt Numbers*/ /*Timer Isr*/  /*Auto Stop enable*/    /*用作捕获*/    
           TIMER2_IRQn,                NULL,          false,                &astPinInfo[0],
    },
};

/*
名称: TimerChInit()
功能:
    1.初始化 timer 的某个channel
参数:   
    1.ptTimerInfo: 指向包含channel 配置信息的指针
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数调用完成即启动timer,若有需要，调用TimerStartCh()启动指定的channel即可触发中断
*/
static void TimerChInit(stTimerInfoDef *const ptTimerInfo)
{
    LPC_TIM_TypeDef *TIMx = ptTimerInfo->TIMx;
    TimerChDef TimerCh = ptTimerInfo->TimerCh;

    LPC_SC->PCONP |= ptTimerInfo->ulPwrEn;
    TIMx->PC = 0;
    TIMx->CTCR = 0;//只用作timer
    TIMx->TC = 0;

    if (ptTimerInfo->pstPin)
    {//用作捕获
        stPinInfoDef *pstPin = ptTimerInfo->pstPin;
        
        GpioPinCfg(pstPin->stPinCfg);
        if (pstPin->bIrEn)
        {
            TIMx->CCR |= ((pstPin->enEde) << (3 * TimerCh)) | TIM_INT_ON_CAP(TimerCh);
        }
        else
        {
            TIMx->CCR |= (pstPin->enEde) << (3 * TimerCh);
        }
        TIMx->MCR = 0;//不启用match
    }
    else
    {//用作定时
        TIMx->MCR &= ~ TIM_MCR_CHANNEL_MASKBIT(TimerCh);
        if (ptTimerInfo->bAutoStop)
        {
            TIMx->MCR |= TIM_STOP_ON_MATCH(TimerCh);
        }
        //TIMx->MCR |= TIM_RESET_ON_MATCH(TimerCh);//TC will be reset if MR matches it.
        if(ptTimerInfo->IsrFunc)
        {
            IsrRegester(ptTimerInfo->IRQn, ptTimerInfo->IsrFunc);
        }
    }
    if (ptTimerInfo->enPresOpt  == TIM_PRESCALE_TICKVAL)
    {//match Val
        TIMx->PR   = ptTimerInfo->ulPresValue - 1  ;
    }
    else
    {//us
        TIMx->PR   = (CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER) / 1000000) * ptTimerInfo->ulPresValue - 1;
    }

    TIMx->TCR |= TIM_ENABLE | TIM_RESET;//timer start
    TIMx->TCR &= ~TIM_RESET;//timer cunt rest
    return;
}

/*
名称: TimerClrFlg()
功能:
    1.清除指定Channel 的超时标志
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void TimerClrFlg(TimerIdDef TimerId, TimerChDef TimerCh)
{
    usTimerFlg &= ~(1 << (TimerId * TimerCHMax + TimerCh));
    return;
}

/****************************************extern函数定义*********************************************/
/*
名称: TimerInit()
功能:
    1.初始化 timer
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数中对timer 进行了stop操作,需要调用TimerStart()启动timer
*/
void TimerInit (void)
{
    uint8_t i = 0;

    for(i = 0; i < SizeOfArray(stTimerInfo); i++)
    {
        TimerChInit(&stTimerInfo[i]);
    }
    return;
}

/*
名称: TimerStop()
功能:
    1. 停止指定Timer
参数:   
    1. TimerId:     timer id(Timer0 etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void TimerStopCh(TimerIdDef TimerId, TimerChDef TimerCh)
{
    LPC_TIM_TypeDef *TIMx = pstTimerReg[TimerId];

    TimerClrFlg(TimerId, TimerCh);
    TIMx->MCR &= ~TIM_INT_ON_MATCH(TimerCh);//enable interrupt
    return;
}

/*
名称: TimerStart()
功能:
    1. 启动指定Timer的指定Channel
    2. 清空PC(Prescale Counter register )及TC(Prescale Counter register )
参数:   
    1. TIMx:    指向timer 寄存器的指针(LPC_TIM0 etc.)
    2. TimerCh: timer channel(TimerCH0 etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void TimerStartCh(TimerIdDef TimerId, TimerChDef TimerCh, uint32_t ulMatchVal)
{
    LPC_TIM_TypeDef *TIMx = pstTimerReg[TimerId];

    TIMx->MR[TimerCh] = ulMatchVal + TIMx->TC;
    TIMx->IR = TIM_IR_CLR(TimerCh);//clear interrupt flag
    TimerClrFlg(TimerId, TimerCh);
    TIMx->MCR |= TIM_INT_ON_MATCH(TimerCh);//enable interrupt
    return;
}

/*
名称: TimerReFre()
功能:
    1. 刷新指定timer的指定channel(若使能中断，该channel的中断将在ulVal 个时间单位后再次触发)
参数:   
    1. TimerId: timer id(Timer0 etc.)
    2. TimerCh: timer channel(TimerCH0 etc.)
    3. ulVal:   要延时的时间单位，具体算法见stTimerInfoDef的定义
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void TimerReFresh(TimerIdDef TimerId, TimerChDef TimerCh, uint32_t ulVal)
{
    LPC_TIM_TypeDef *TIMx = pstTimerReg[TimerId];

    TIMx->MR[TimerCh] = ulVal + TIMx->TC;
    return;
}

/*
名称: TimerIsrReg()
功能:
    1. 注册上层指定的TIMER ISR
参数:   
    1. TimerId: timer id(Timer0 etc.)
    2. TimerCh: timer channel(TimerCH0 etc.)
    3. pIsr:    上层指定的Timer
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.调用该函数需在TimerStartCh()之前
*/
void TimerIsrReg(TimerIdDef TimerId, pIsrFunc pIsr)
{
    IsrRegester((IRQn_Type)(TIMER0_IRQn + TimerId), pIsr);
    return;
}

/*
名称: TimerIsOut()
功能:
    1. 判断timer 是否超时
参数:   
    1. TimerId:     timer id(Timer0 etc.)
    2. TimerCh: timer channel(TimerCH0 etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
bool TimerIsOut(TimerIdDef TimerId, TimerChDef TimerCh)
{
    return (usTimerFlg & (1 << (TimerId * TimerCHMax + TimerCh))) ? true : false;
}

/*
名称: TimerGetCh()
功能:
    1. 获取已超时的Timer Channel
参数:   
    1. TimerId:     timer id(Timer0 etc.)
    2. aTimerCh:    指向返回的channel的指针
返回值: 
    1.已超时的Timer Channel个数
输入:   void
输出:   void
备注:   void
注意:   
    1.考虑到可能有多个中断同时发生，故此处用数组表示
*/
uint8_t TimerGetCh(TimerIdDef TimerId, TimerChDef *aTimerCh)
{    
    LPC_TIM_TypeDef *TIMx = pstTimerReg[TimerId];
    uint32_t ulTmp = TIMx->IR & 0x0f;
    TimerChDef TimerCh = TimerCH0;
    uint8_t i = 0;
    
    TIMx->IR = ulTmp;//clr flag
    while (ulTmp)
    {
        if (ulTmp & 0x01)
        {
            aTimerCh[i++] = TimerCh;
        }
        TimerCh = (TimerChDef)(TimerCh + 1);
        ulTmp >>= 1;
    }
    return i;
}

#if TIMER_DBG
/*
名称:   TimerTst()
功能:   Timer Test
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void TimerTst(void)
{
    TimerStartCh(LPC_TIM0, TimerCH0, 10);
    while(!TimerIsOut(LPC_TIM0, TimerCH0));
    return;
}
#endif
