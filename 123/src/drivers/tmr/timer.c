#include "include.h"
/*
�ļ�����: timer.c
����:
    1.Timer ��صĺ���
����: ������
�޸ļ�¼:
    2017-4-3    �ļ�����
    2017-4-30   ���֧�����벶��Ĺ���
    2017-5-2    ��TimerIdDef TimerId�滻ԭ�к����е�LPC_TIM_TypeDef *TIMx
��ע:
    1.���ļ��еĺ��������÷�����:
        һ. ��stTimerInfo[]�и�����Ҫ���timer��ص�������Ϣ,��ʹ���ж�,��Ҫ��astIRQnInfo[]��������������Ϣ
        ��. ��BSP init�����е���TimerInit()����
        ��. ����Ҫʱ��ͨ������TimerStartCh()��������ָ��Channel�Ķ�ʱ�������Ե���TimerReFresh()����ˢ�¶�ʱ��(����ι��)
        ��. ����TimerIsOut()�����ж��Ƿ��ѳ�ʱ
        ��. ��������ʱ�����ļ���δ�ṩISR
ע��: 
    1. ������Ŀ��Ҫ��timer��ʱֻ������ʱ�����벶��,��������Ҫ��������,�ɸ�����Ҫ���ƴ���
    2. ���г���Ĭ�� TC will be reset if MR matches it,����ֻ�������жϺ�AutoStop�����,������Ҫ�ɸ�����Ҫ���ƴ���
    3. timer0-timer3�и����ĸ�channel,ÿ��timer�ĸ�channel�и��Զ�����match Register,����Ĵ���Ϊ��channel����,
       Ϊ����Ӱ������channel,�����򲻵��ѣ����ö����üĴ���
    4. ���ϲ�������TimerIsrReg()����ע�����ϲ�ָ����Timer Isr,�� TimerIsOut()���ܽ������ã��ұ�����ָ���ж��е���TimerGetCh()������ж������־
*/

/******************************************�궨��*************************************************/

/*******************************************����**************************************************/
typedef enum{
Redge = 1,//rising edge
Fedge = 2,//falling edge
Bedge = 3,//both edge
}enEdgeDef;


#pragma pack(1)

typedef struct{
stPinCfgDef stPinCfg;
enEdgeDef   enEde;
bool        bIrEn;//�Ƿ�ʹ�ܲ����ж�
}stPinInfoDef;

typedef const struct{
LPC_TIM_TypeDef * TIMx; //timer�Ĵ���ָ��(LPC_TIM2 etc.)
TimerChDef TimerCh; //timer channel(TimerCH0 etc.)  ע��:��������������ֶ����ΪTimerCH1
uint32_t ulPwrEn; //Power enable flag(CLKPWR_PCONP_PCTIM0 etc.)
TIM_PRESCALE_OPT enPresOpt;/*Timer/Counter prescale option(us or match Val)*/
uint32_t ulPresValue; // ��enPreOptΪTIM_PRESCALE_TICKVALʱ�����ֶ�Ϊ���õ�PC�е�Ԥ��Ƶֵ(1 - 0xffffffff)
                            // ��enPreOptΪTIM_PRESCALE_USVALʱ�����ֶ���ulMatchValueһ����������жϵļ��(us) (ulPresValue * ulMatchValue)
IRQn_Type IRQn;//timer Interrupt Numbers(TIMER0_IRQn etc.)
pIsrFunc IsrFunc;//��������ʱ�����ֶ���Ч����ʹ���жϣ���Ҫ�ϲ�ͨ������TimerIsrReg()����ע��ISR
bool bAutoStop;//Auto Stop enable(true or false)
stPinInfoDef *pstPin;//�����ֶη�NULL�����Channel��������,����������ʱ
}stTimerInfoDef;
#pragma pack()

/*****************************************��������************************************************/
static volatile uint16_t usTimerFlg;//��channel��ʱʱ�䵽��bit��־ 

static stPinInfoDef astPinInfo[] = {//�������������Ϣ
{//��������
     {TSENSOR_IN_CAP_PORT, TSENSOR_IN_CAP_PIN, TSENSOR_IN_CAP_FUNC,},
     Bedge,//���񴥷�edge
     true,//�����ж�ʹ��
}
};

/******************************************��������***********************************************/
/****************************************static��������*********************************************/
/*Timer ISR ����*/
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
        /*timer �Ĵ���ָ��*/  /*timer channel*/ /*clock enable flag*/  /*prescale option*/ /*Prescale value*/  
        LPC_TIM0,              UART0_TIMER,      CLKPWR_PCONP_PCTIM0,   TIM_PRESCALE_USVAL, 1000,                
        /*timer Interrupt Numbers*/ /*Timer Isr*/  /*Auto Stop enable*/    /*������ʱ*/
          TIMER0_IRQn,                Timer0ISR,        false,               NULL,
    }, 
    {//timer0, channel1
        /*timer �Ĵ���ָ��*/  /*timer channel*/ /*clock enable flag*/  /*prescale option*/ /*Prescale value*/  
        LPC_TIM0,              UART1_TIMER,       CLKPWR_PCONP_PCTIM0,   TIM_PRESCALE_USVAL, 1000,                
        /*timer Interrupt Numbers*/  /*Timer Isr*/ /*Auto Stop enable*/   /*������ʱ*/
          TIMER0_IRQn,                 Timer0ISR,       false,               NULL,
    },
    
    {//timer0, channel2
        /*timer �Ĵ���ָ��*/  /*timer channel*/ /*clock enable flag*/  /*prescale option*/ /*Prescale value*/  
        LPC_TIM0,               UART2_TIMER,       CLKPWR_PCONP_PCTIM0,   TIM_PRESCALE_USVAL, 1000,                
        /*timer Interrupt Numbers*/ /*Timer Isr*/  /*Auto Stop enable*/    /*������ʱ*/
          TIMER0_IRQn,                Timer0ISR,          false,             NULL,
    },
    {//timer0, channel3
        /*timer �Ĵ���ָ��*/  /*timer channel*/ /*clock enable flag*/  /*prescale option*/ /*Prescale value*/  
        LPC_TIM0,               UART3_TIMER,       CLKPWR_PCONP_PCTIM0,   TIM_PRESCALE_USVAL, 1000,                
        /*timer Interrupt Numbers*/ /*Timer Isr*/  /*Auto Stop enable*/    /*������ʱ*/
          TIMER0_IRQn,                Timer0ISR,          false,             NULL,
    },
    {//timer2, channel0
        /*timer �Ĵ���ָ��*/  /*timer channel*/ /*clock enable flag*/  /*prescale option*/      /*Prescale value*/  
        LPC_TIM2,               TEMP_TIMER,       CLKPWR_PCONP_PCTIM2,   TIM_PRESCALE_TICKVAL,      1,                
        /*timer Interrupt Numbers*/ /*Timer Isr*/  /*Auto Stop enable*/    /*��������*/    
           TIMER2_IRQn,                NULL,          false,                &astPinInfo[0],
    },
};

/*
����: TimerChInit()
����:
    1.��ʼ�� timer ��ĳ��channel
����:   
    1.ptTimerInfo: ָ�����channel ������Ϣ��ָ��
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú���������ɼ�����timer,������Ҫ������TimerStartCh()����ָ����channel���ɴ����ж�
*/
static void TimerChInit(stTimerInfoDef *const ptTimerInfo)
{
    LPC_TIM_TypeDef *TIMx = ptTimerInfo->TIMx;
    TimerChDef TimerCh = ptTimerInfo->TimerCh;

    LPC_SC->PCONP |= ptTimerInfo->ulPwrEn;
    TIMx->PC = 0;
    TIMx->CTCR = 0;//ֻ����timer
    TIMx->TC = 0;

    if (ptTimerInfo->pstPin)
    {//��������
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
        TIMx->MCR = 0;//������match
    }
    else
    {//������ʱ
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
����: TimerClrFlg()
����:
    1.���ָ��Channel �ĳ�ʱ��־
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void TimerClrFlg(TimerIdDef TimerId, TimerChDef TimerCh)
{
    usTimerFlg &= ~(1 << (TimerId * TimerCHMax + TimerCh));
    return;
}

/****************************************extern��������*********************************************/
/*
����: TimerInit()
����:
    1.��ʼ�� timer
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú����ж�timer ������stop����,��Ҫ����TimerStart()����timer
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
����: TimerStop()
����:
    1. ָֹͣ��Timer
����:   
    1. TimerId:     timer id(Timer0 etc.)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void TimerStopCh(TimerIdDef TimerId, TimerChDef TimerCh)
{
    LPC_TIM_TypeDef *TIMx = pstTimerReg[TimerId];

    TimerClrFlg(TimerId, TimerCh);
    TIMx->MCR &= ~TIM_INT_ON_MATCH(TimerCh);//enable interrupt
    return;
}

/*
����: TimerStart()
����:
    1. ����ָ��Timer��ָ��Channel
    2. ���PC(Prescale Counter register )��TC(Prescale Counter register )
����:   
    1. TIMx:    ָ��timer �Ĵ�����ָ��(LPC_TIM0 etc.)
    2. TimerCh: timer channel(TimerCH0 etc.)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: TimerReFre()
����:
    1. ˢ��ָ��timer��ָ��channel(��ʹ���жϣ���channel���жϽ���ulVal ��ʱ�䵥λ���ٴδ���)
����:   
    1. TimerId: timer id(Timer0 etc.)
    2. TimerCh: timer channel(TimerCH0 etc.)
    3. ulVal:   Ҫ��ʱ��ʱ�䵥λ�������㷨��stTimerInfoDef�Ķ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void TimerReFresh(TimerIdDef TimerId, TimerChDef TimerCh, uint32_t ulVal)
{
    LPC_TIM_TypeDef *TIMx = pstTimerReg[TimerId];

    TIMx->MR[TimerCh] = ulVal + TIMx->TC;
    return;
}

/*
����: TimerIsrReg()
����:
    1. ע���ϲ�ָ����TIMER ISR
����:   
    1. TimerId: timer id(Timer0 etc.)
    2. TimerCh: timer channel(TimerCH0 etc.)
    3. pIsr:    �ϲ�ָ����Timer
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.���øú�������TimerStartCh()֮ǰ
*/
void TimerIsrReg(TimerIdDef TimerId, pIsrFunc pIsr)
{
    IsrRegester((IRQn_Type)(TIMER0_IRQn + TimerId), pIsr);
    return;
}

/*
����: TimerIsOut()
����:
    1. �ж�timer �Ƿ�ʱ
����:   
    1. TimerId:     timer id(Timer0 etc.)
    2. TimerCh: timer channel(TimerCH0 etc.)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
bool TimerIsOut(TimerIdDef TimerId, TimerChDef TimerCh)
{
    return (usTimerFlg & (1 << (TimerId * TimerCHMax + TimerCh))) ? true : false;
}

/*
����: TimerGetCh()
����:
    1. ��ȡ�ѳ�ʱ��Timer Channel
����:   
    1. TimerId:     timer id(Timer0 etc.)
    2. aTimerCh:    ָ�򷵻ص�channel��ָ��
����ֵ: 
    1.�ѳ�ʱ��Timer Channel����
����:   void
���:   void
��ע:   void
ע��:   
    1.���ǵ������ж���ж�ͬʱ�������ʴ˴��������ʾ
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
����:   TimerTst()
����:   Timer Test
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void TimerTst(void)
{
    TimerStartCh(LPC_TIM0, TimerCH0, 10);
    while(!TimerIsOut(LPC_TIM0, TimerCH0));
    return;
}
#endif
