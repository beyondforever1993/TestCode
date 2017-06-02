/*
�ļ�����: Interrupt.c
����:
    1.�����ж���صĺ���
����: ������
�޸ļ�¼:
    2017-3-23 �ļ�����
��ע:
    1.Bsp��ʼ��ʱ������������IntInit()���������ж����ȼ����� 
    2.��ʼ����ɺ�,�����ж�ʱ�������IntEn()ʹ��astIRQnInfo[]������bIrEnΪ1���ж�
    3.��������ĳ�жϣ�ֻ��������²���:
      һ. ��astIRQnInfo������ն˺�(UART0_IRQn etc.)����Ӧ���ȼ�����ռ���ȼ����Ƿ�ʹ���ж�
      ��. ����IsrRegester()ע��ISR
ע��: void
*/

#include "include.h"

#define NVIC_PRIO_GROUP            5

#define PrintIntErr(Func)          printf("Irq %s Config Err!\r\n", #Func);


typedef const struct{
const IRQn_Type IRQn;
const uint32_t  PreemptPriority;//��ռ���ȼ�
const uint32_t  SubPriority;
const bool      bIrEn;
const bool      bIsObj;
}stIRQnInfoDef;

typedef struct{
pIsrFunc pIsrFunc;
bool bIsObj;
}stIsrDef;

static stIsrDef astIsr[IRQnMax] = {NULL,};
static stIRQnInfoDef astIRQnInfo[] ={
    {   /*Uart0 Interrupt Number*/  /*��ռ���ȼ�*/      /*��Ӧ���ȼ�*/  /*�ж��Ƿ�ʹ��*/    /*�Ƿ���Ҫ�ں˶������*/
        UART0_IRQn,                 0,                  5,              true,               false,
    },
    {   /*Uart1 Interrupt Number*/  /*��ռ���ȼ�*/      /*��Ӧ���ȼ�*/  /*�ж��Ƿ�ʹ��*/    /*�Ƿ���Ҫ�ں˶������*/
        UART1_IRQn,                 0,                  5,              true,               false,
    },
    {   /*Uart2 Interrupt Number*/  /*��ռ���ȼ�*/      /*��Ӧ���ȼ�*/  /*�ж��Ƿ�ʹ��*/    /*�Ƿ���Ҫ�ں˶������*/
        UART2_IRQn,                 0,                  5,              true,               false,
    },
    {   /*Uart3 Interrupt Number*/  /*��ռ���ȼ�*/      /*��Ӧ���ȼ�*/  /*�ж��Ƿ�ʹ��*/    /*�Ƿ���Ҫ�ں˶������*/
        UART3_IRQn,                 0,                  5,              true,               false,
    },
    {   /*Timer0 Interrupt Number*/  /*��ռ���ȼ�*/      /*��Ӧ���ȼ�*/  /*�ж��Ƿ�ʹ��*/   /*�Ƿ���Ҫ�ں˶������*/
        TIMER0_IRQn,                 0,                  6,              true,              true,
    },
    {   /*Timer2 Interrupt Number*/  /*��ռ���ȼ�*/      /*��Ӧ���ȼ�*/  /*�ж��Ƿ�ʹ��*/   /*�Ƿ���Ҫ�ں˶������*/
        TIMER2_IRQn,                 1,                  6,              true,              false,
    },
    {   /*PWM1 Interrupt Number*/  /*��ռ���ȼ�*/      /*��Ӧ���ȼ�*/  /*�ж��Ƿ�ʹ��*/   /*�Ƿ���Ҫ�ں˶������*/
        PWM1_IRQn,                   1,                  10,              true,              false,
    },
};

#define IRQHandler(irq)             void irq##_IRQHandler(void)\
                                     {\
                                        if (astIsr[irq##_IRQn].bIsObj)\
                                        {\
                                            CPU_SR_ALLOC();\
                                            CPU_CRITICAL_ENTER();\
                                            OSIntEnter();\
                                            CPU_CRITICAL_EXIT();\
                                        }\
                                        if (astIsr[irq##_IRQn].pIsrFunc)\
                                        {\
                                            astIsr[irq##_IRQn].pIsrFunc();\
                                        }\
                                        else\
                                        {\
                                            PrintIntErr(irq##_IRQHandler);\
                                        }\
                                        if (astIsr[irq##_IRQn].bIsObj)\
                                        {\
                                            OSIntExit();\
                                        }\
                                        return;\
                                    }
IRQHandler(WDT)
IRQHandler(TIMER0)
IRQHandler(TIMER1)
IRQHandler(TIMER2)
IRQHandler(TIMER3)
IRQHandler(UART0)
IRQHandler(UART1)
IRQHandler(UART2)
IRQHandler(UART3)
IRQHandler(PWM1)
IRQHandler(I2C0)
IRQHandler(I2C1)
IRQHandler(I2C2)
IRQHandler(SSP0)
IRQHandler(SSP1)
IRQHandler(PLL0)
IRQHandler(RTC)
IRQHandler(EINT0)
IRQHandler(EINT1)
IRQHandler(EINT2)
IRQHandler(EINT3)
IRQHandler(ADC)
IRQHandler(BOD)
IRQHandler(USB)
IRQHandler(CAN)
IRQHandler(DMA)
IRQHandler(I2S)
IRQHandler(ENET)
IRQHandler(MCI)
IRQHandler(MCPWM)
IRQHandler(QEI)
IRQHandler(PLL1)
IRQHandler(USBActivity)
IRQHandler(CANActivity)
IRQHandler(UART4)
IRQHandler(SSP2)
IRQHandler(LCD)
IRQHandler(GPIO)
IRQHandler(PWM0)
IRQHandler(EEPROM)


/*
����: IsrRegester()
����:
    1.����ָ���жϵ�ISR
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void IsrRegester(const IRQn_Type IRQn, pIsrFunc IsrFunc)
{
    astIsr[IRQn].pIsrFunc = IsrFunc;
    return;
}

/*
����: IntCfg()
����:
    1.����astIRQnInfo ��NVIC_PRIO_GROUP����ָ���жϵ����ȼ�
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void IntCfg(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority)
{
    uint32_t ulPriority = 0;
    
    ulPriority = NVIC_EncodePriority(NVIC_PRIO_GROUP, PreemptPriority, SubPriority);
    NVIC_SetPriority(IRQn, ulPriority);
    return;
}

/*
����: IntInit()
����:
    1.�����ж����ȼ����� NVIC_PRIO_GROUP
    2.����astIRQnInfo�����ж����ȼ�
����:   void
����ֵ: void
����:   
    1.astIRQnInfo: �����жϺš��ж����ȼ��Ľṹ������
���:   void
��ע:   void
ע��:   void
*/
void IntInit(void)
{
    uint8_t i = 0;
    
    NVIC_DeInit();//�ر������ж�,��������жϱ�־���ж����ȼ�
    NVIC_SetPriorityGrouping(NVIC_PRIO_GROUP);
    for (i = 0; i < SizeOfArray(astIRQnInfo); i++)
    {
        IntCfg(astIRQnInfo[i].IRQn, astIRQnInfo[i].PreemptPriority, astIRQnInfo[i].SubPriority);
    }
    return;
}

/*
����: IntInit()
����:
    1.ʹ��astIRQnInfo������bIrEnΪ1���ж�
����:   void
����ֵ: void
����:   
    1.astIRQnInfo: �����жϺš��ж����ȼ����ж�ʹ�ܱ�־�Ľṹ������
���:   void
��ע:   void
ע��:   void
*/
void IntEn(void)
{
    uint8_t i     = 0;
    uint8_t ucCnt = SizeOfArray(astIRQnInfo);
    bool  bEn   = false;

    NVIC_DeInit();//ʧ�������ж�
    for (i = 0; i < ucCnt; i++)
    {
        bEn = astIRQnInfo[i].bIrEn;
        if (bEn)
        {
            NVIC_EnableIRQ(astIRQnInfo[i].IRQn);
        }
    }
    
    return;
}
