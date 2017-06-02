/*
文件名称: Interrupt.c
功能:
    1.包含中断相关的函数
作者: 杜在连
修改记录:
    2017-3-23 文件初创
备注:
    1.Bsp初始化时必须在最后调用IntInit()函数设置中断优先级分组 
    2.初始化完成后,允许中断时必须调用IntEn()使能astIRQnInfo[]中所有bIrEn为1的中断
    3.如需配置某中断，只需完成如下操作:
      一. 在astIRQnInfo中添加终端号(UART0_IRQn etc.)和响应优先级、抢占优先级及是否使能中断
      二. 调用IsrRegester()注册ISR
注意: void
*/

#include "include.h"

#define NVIC_PRIO_GROUP            5

#define PrintIntErr(Func)          printf("Irq %s Config Err!\r\n", #Func);


typedef const struct{
const IRQn_Type IRQn;
const uint32_t  PreemptPriority;//抢占优先级
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
    {   /*Uart0 Interrupt Number*/  /*抢占优先级*/      /*响应优先级*/  /*中断是否使能*/    /*是否需要内核对象参与*/
        UART0_IRQn,                 0,                  5,              true,               false,
    },
    {   /*Uart1 Interrupt Number*/  /*抢占优先级*/      /*响应优先级*/  /*中断是否使能*/    /*是否需要内核对象参与*/
        UART1_IRQn,                 0,                  5,              true,               false,
    },
    {   /*Uart2 Interrupt Number*/  /*抢占优先级*/      /*响应优先级*/  /*中断是否使能*/    /*是否需要内核对象参与*/
        UART2_IRQn,                 0,                  5,              true,               false,
    },
    {   /*Uart3 Interrupt Number*/  /*抢占优先级*/      /*响应优先级*/  /*中断是否使能*/    /*是否需要内核对象参与*/
        UART3_IRQn,                 0,                  5,              true,               false,
    },
    {   /*Timer0 Interrupt Number*/  /*抢占优先级*/      /*响应优先级*/  /*中断是否使能*/   /*是否需要内核对象参与*/
        TIMER0_IRQn,                 0,                  6,              true,              true,
    },
    {   /*Timer2 Interrupt Number*/  /*抢占优先级*/      /*响应优先级*/  /*中断是否使能*/   /*是否需要内核对象参与*/
        TIMER2_IRQn,                 1,                  6,              true,              false,
    },
    {   /*PWM1 Interrupt Number*/  /*抢占优先级*/      /*响应优先级*/  /*中断是否使能*/   /*是否需要内核对象参与*/
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
名称: IsrRegester()
功能:
    1.设置指定中断的ISR
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void IsrRegester(const IRQn_Type IRQn, pIsrFunc IsrFunc)
{
    astIsr[IRQn].pIsrFunc = IsrFunc;
    return;
}

/*
名称: IntCfg()
功能:
    1.根据astIRQnInfo 和NVIC_PRIO_GROUP配置指定中断的优先级
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void IntCfg(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority)
{
    uint32_t ulPriority = 0;
    
    ulPriority = NVIC_EncodePriority(NVIC_PRIO_GROUP, PreemptPriority, SubPriority);
    NVIC_SetPriority(IRQn, ulPriority);
    return;
}

/*
名称: IntInit()
功能:
    1.设置中断优先级分组 NVIC_PRIO_GROUP
    2.根据astIRQnInfo配置中断优先级
参数:   void
返回值: void
输入:   
    1.astIRQnInfo: 包括中断号、中断优先级的结构体数组
输出:   void
备注:   void
注意:   void
*/
void IntInit(void)
{
    uint8_t i = 0;
    
    NVIC_DeInit();//关闭所有中断,清除所有中断标志及中断优先级
    NVIC_SetPriorityGrouping(NVIC_PRIO_GROUP);
    for (i = 0; i < SizeOfArray(astIRQnInfo); i++)
    {
        IntCfg(astIRQnInfo[i].IRQn, astIRQnInfo[i].PreemptPriority, astIRQnInfo[i].SubPriority);
    }
    return;
}

/*
名称: IntInit()
功能:
    1.使能astIRQnInfo中所有bIrEn为1的中断
参数:   void
返回值: void
输入:   
    1.astIRQnInfo: 包括中断号、中断优先级及中断使能标志的结构体数组
输出:   void
备注:   void
注意:   void
*/
void IntEn(void)
{
    uint8_t i     = 0;
    uint8_t ucCnt = SizeOfArray(astIRQnInfo);
    bool  bEn   = false;

    NVIC_DeInit();//失能所有中断
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
