/*
文件名称: bsp_uart.c
功能:
    1.包含Uart驱动相关的函数
作者: 杜在连
修改记录:
    2017-3-23 文件初创
备注:
    1.如需添加其他端口驱动，只需在astUartFifo[], astUartInfo[]两个数组对应位置中添加相关配置参数即可,
    2.若配置中断使能,需在astIRQnInfo[]中添加相关配置参数
注意:
    1.该文件中，只配置了接收中断。如有需要可以更改UartBspInit()函数及astIRQnInfo[]数组
    2.文件初创时配置如下:
        起始位：1  
        数据位：8  
        停止位：1  
        校验：  无 
     如有需要可根据需要修改UartBspInit()函数
    3.文件初创时，只考虑了UART0-3的应用，如有需要，请根据LPC1778 User Manual添加对UART4的兼容
*/


#include "include.h"

/******************************************宏定义***********************************************/

/*******************************************声明************************************************/
#pragma pack(1)

typedef const struct {
stPinCfgDef TxPin;
stPinCfgDef RxPin;
}stGpioDef;

typedef const struct {//定义超时检测用timer相关信息
TimerIdDef  TimerId;// Timer0 etc.
TimerChDef  TimerCh;//  timer channel(TimerCH0)
pIsrFunc    pTimerIsr;//用于超时检测的  Timer ISR (注意，该字段必须与 TIMx 相匹配)
uint32_t ulVal;//认为收到包尾的结束超时时间(具体延时单位参考stTimerInfoDef中的ulPresValue元素)
}stTimerDef;

typedef struct{
OS_MEM   *const pOsMem;//指向OS存储分区控制块的指针 (MemSmall64 etc)
uint8_t  *pucBuff;//指向从OS申请的内存起始地址的指针(由超时检测 Timer 将该指针加入消息队列)
uint16_t *pusLen;//指向存储接收到的数据长度的指针
uint16_t  usBuffLen;//申请到的内存长度
}stMemDef;

typedef const struct {
void * UARTx;
stGpioDef   *pstGpio;
IRQn_Type    enIrq;     //中断编号(UART0_IRQn etc.)
uint32_t     ulPwrEn;//Power enable(CLKPWR_PCONP_PCUART0 etc.)
pUartIsr     pIsr;      //指向Isr的指针
stMemDef    *pstMem;
//stFiFoDef   *pFiFo;           //缓存FIFO
OS_MUTEX    *pMutex;//互斥信号量为NULL表示不需要互斥保护
OS_Q        *pRecvQ;//数据接收消息队列
stTimerDef  *pTimer;//定义超时检测用timer相关信息(用于检测包尾)
}stUartInfoDef;
#pragma pack()

static void UARTx_ISR(void);
static void UART1_ISR(void);
static void TIMER0_ISR(void);

/*****************************************变量定义************************************************/
stGpioDef astGpio[] = {
{//uart0
    {UART0_TX_PORT,   UART0_TX_PIN,     UART0_TX_FUNC | IOCON_MODE_PULLUP | IOCON_HYS_ENABLE},   //TX
    {UART0_RX_PORT,   UART0_RX_PIN,     UART0_RX_FUNC | IOCON_MODE_PULLUP | IOCON_HYS_ENABLE}    //RX
},
{//uart1
    {UART1_TX_PORT,   UART1_TX_PIN,     UART1_TX_FUNC | IOCON_MODE_PULLUP | IOCON_HYS_ENABLE},   //TX
    {UART1_RX_PORT,   UART1_RX_PIN,     UART1_RX_FUNC | IOCON_MODE_PULLUP | IOCON_HYS_ENABLE}    //RX
},
{//uart2
    {UART2_TX_PORT,   UART2_TX_PIN,     UART2_TX_FUNC | IOCON_MODE_PULLUP | IOCON_HYS_ENABLE},   //TX
    {UART2_RX_PORT,   UART2_RX_PIN,     UART2_RX_FUNC | IOCON_MODE_PULLUP | IOCON_HYS_ENABLE}    //RX
},
{//uart3
    {UART3_TX_PORT,   UART3_TX_PIN,     UART3_TX_FUNC | IOCON_MODE_PULLUP | IOCON_HYS_ENABLE},   //TX
    {UART3_RX_PORT,   UART3_RX_PIN,     UART3_RX_FUNC | IOCON_MODE_PULLUP | IOCON_HYS_ENABLE}    //RX
},

};

/*定义超时检测用timer相关信息(用于检测包尾)*/
static stTimerDef  aTimer[] = {
{//Uart0
    UART_TIMERx, 
    UART0_TIMER,
    TIMER0_ISR,
    UART0_VAL,
},
{//Uart1
    UART_TIMERx, 
    UART1_TIMER,
    TIMER0_ISR,
    UART1_VAL,
},
{//Uart2
    UART_TIMERx, 
    UART2_TIMER,
    TIMER0_ISR,
    UART2_VAL,
},
{//Uart3
    UART_TIMERx, 
    UART3_TIMER,
    TIMER0_ISR,
    UART3_VAL,
}

};

static uint16_t ausLen[] = {
    0,// Uart0
    0,// Uart1
    0,// Uart2
    0,// Uart3
};

static stMemDef astMem[] = {
{//uart0
    &MemSmall64,
    NULL,
    &ausLen[0],
    MEM_SMALL64_SZ,
},
{//uart1
    &MemBig,
    NULL,
    &ausLen[1],
    MEM_BIG_SZ,
},
{//uart2
    &MemSmall64,
    NULL,
    &ausLen[2],
    MEM_SMALL64_SZ,
},
{//uart3
    &MemSmall,
    NULL,
    &ausLen[3],
    MEM_SMALL_SZ,
},
};

static stUartInfoDef astUartInfo[] = {
    {/*UART0*/ /*UART_COM_CH*/
        (void *)LPC_UART0,
        /* GPIO         中断编号       PWR ENR             ISR        OS_MEM          互斥信号量     接收队列*/   /*timer*/
        &astGpio[0],  UART0_IRQn, CLKPWR_PCONP_PCUART0, UARTx_ISR,    &astMem[0],    &COMSendM,     &COMRecvQ,   &aTimer[0]
    },
    {/*UART1*/  /*UART_GPRS_CH*/
        (void *)LPC_UART1,
        /*GPIO         中断编号       PWR ENR             ISR         OS_MEM         互斥信号量     接收队列*/   /*timer*/
        &astGpio[1], UART1_IRQn, CLKPWR_PCONP_PCUART1, UART1_ISR,    &astMem[1],     &GprsSendM,    &GprsRecvQ,   &aTimer[1]
    },
    {/*UART2*/  /*UART_485_CH*/
        (void *)LPC_UART2,
        /*GPIO         中断编号       PWR ENR             ISR         OS_MEM          互斥信号量      接收队列*/     /*timer*/
        &astGpio[2], UART2_IRQn, CLKPWR_PCONP_PCUART2, UARTx_ISR,    &astMem[2],      &SenSorSendM,   &SenSorQ,     &aTimer[2]
    },
    {/*UART3*/  /*UART_BD_CH*/
        (void *)LPC_UART3,
        /*GPIO          中断编号       CLK ENR             ISR        OS_MEM        互斥信号量    接收队列*/     /*timer*/
        &astGpio[3],    UART3_IRQn, CLKPWR_PCONP_PCUART3, UARTx_ISR,   &astMem[3],    &BDSendM,     &BDRecvQ,      &aTimer[3]
    },
};

static volatile uint8_t ucRecvFlg = 0;//已收到包头的标志(bit映射)(由 Uart ISR 置位，timer ISR 在确定收到完整包并将整包数据加入消息队列之后清零) 

/******************************************函数定义*****************************************************/
/****************************************static函数定义*********************************************/
#define SetRecvFlg(UartID)              do{ucRecvFlg |= (1 << UartID);}while(0)
#define ClrRecvFlg(UartID)              do{ucRecvFlg &= ~(1 << UartID);}while(0)
#define GetRecvFlg(UartID)              (ucRecvFlg & (1 << UartID))

/*
名称: GetActiveCh()
功能:
    1.获取当前ISR对应的Uart Channel
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static UART_ID_Type GetActiveCh(void)
{
    UART_ID_Type enUartId = UART_0;
    uint8_t i = 0;

    for (i = 0; i < 4; i++)
    {
        if (NVIC_GetActive((IRQn_Type)(UART0_IRQn + i)))
        {
            break;
        }
    }
    return (UART_ID_Type)(enUartId + i);
}

/*
名称: GetBuffer()
功能:
    1.判断是否刚收到包头,若收到包头则向OS申请内存,启动包尾检测Timer, 并将已收到的包长清零,并返回缓存指针
    2.若之前已收到包头，则该函数刷新包尾检测Timer并返回缓存指针
参数:   void
返回值: 
    指向数据缓存的指针
输入:   void
输出:   void
备注:   void
注意:   void
*/
static uint8_t * GetBuffer(UART_ID_Type UartID)
{// Recv Success
    stUartInfoDef *pstUartInfo = &astUartInfo[UartID];
    uint8_t     *pucBuff = NULL;//指向从OS申请的内存起始地址的指针(由超时检测 Timer 将该指针加入消息队列)
    stTimerDef  *pTimer = pstUartInfo->pTimer;//定义超时检测用timer相关信息(用于检测包尾)

    if (0 == GetRecvFlg(UartID))
    {//第一次收到包头,申请内存
        pucBuff = (uint8_t *)MemGet(pstUartInfo->pstMem->pOsMem);
        if ((NULL == pucBuff) || (NULL == pTimer))
        {//内存申请失败或未使用timer用于超时检测
            goto Return;
        }
        TimerStartCh(pTimer->TimerId, pTimer->TimerCh, pTimer->ulVal);
        SetRecvFlg(UartID);//只有申请成功且已启动 Timer 才设置已收到头的标志，避免因内存申请失败,未启动 Timer 而永久性丢包
        *(pstUartInfo->pstMem->pusLen)  = 0;//收到头，数据长度设为0
        pstUartInfo->pstMem->pucBuff = pucBuff;
    } 
    else if(pTimer)
    {
        TimerReFresh(pTimer->TimerId, pTimer->TimerCh, pTimer->ulVal);
        pucBuff = pstUartInfo->pstMem->pucBuff;
    }
Return:
    return pucBuff;
}

/*
名称: UARTx_ISR()
功能:
    1.UART0-UART3 ISR
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void UARTx_ISR(void)
{
    UART_ID_Type UartID = GetActiveCh();
    stUartInfoDef *pstUartInfo = &astUartInfo[UartID];
    LPC_UART_TypeDef * UARTx = pstUartInfo->UARTx;
    
    if (UARTx->IIR  & UART_IIR_INTID_RLS)
    {//Recv Error 注意:正常收数据也会进该case，具体原因LPC1778 user manual 未说明
        uint8_t ucTmp = UARTx->LSR;// Clear Interrupt

    #if 1
        if (UART_LSR_ERR & ucTmp)
        {
            printf("Uart%d Recv Err!%x\r\n", UartID, ucTmp);
            *pstUartInfo->pstMem->pusLen = 0;
            UARTx->FCR |= UART_FCR_RX_RS;
            goto Return;
        }
    #else
        (void)ucTmp;//防止UART_DBG为0时的编译器警告
    #endif
    }

    if (UARTx->IIR  & UART_IIR_INTID_RDA)
    {
        uint8_t *pucBuff = GetBuffer(UartID);//指向从OS申请的内存起始地址的指针(由超时检测 Timer 将该指针加入消息队列)

        if (NULL == pucBuff)
        {
            goto Return;
        }
        {
            uint16_t usLen = pstUartInfo->pstMem->usBuffLen;
            uint16_t usIndex = *pstUartInfo->pstMem->pusLen;

            while ((UARTx->LSR & UART_LSR_RDR) && (usIndex < (usLen - 1)))      
            {                
                usIndex = (*pstUartInfo->pstMem->pusLen)++;
                pucBuff[usIndex] = UARTx->RBR;
            }
            UARTx->FCR |= UART_FCR_RX_RS;
        }
    }
Return:
    return;
}

/*
名称: UART1_ISR()
功能:
    1.UART1 ISR
参数:   
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void UART1_ISR(void)
{
    stUartInfoDef *pstUartInfo = &astUartInfo[UART_1];

    if (LPC_UART1->IIR  & UART_IIR_INTID_RLS)
    {//Recv Error 
        uint8_t ucTmp = LPC_UART1->LSR;// Clear Interrupt
        
        if (UART_LSR_ERR & ucTmp)
        {
        #if 1//UART_DBG
            printf("Uart1 Recv Err!%x\r\n", ucTmp);
        #endif
            *pstUartInfo->pstMem->pusLen = 0;
            LPC_UART1->FCR |= UART_FCR_RX_RS;
            goto Return;
        }
    }
    if (LPC_UART1->IIR  & UART_IIR_INTID_RDA)
    {// Recv Success
        uint8_t *pucBuff = GetBuffer(UART_1);//指向从OS申请的内存起始地址的指针(由超时检测 Timer 将该指针加入消息队列)

        if (NULL == pucBuff)
        {
            goto Return;
        }
        {
            uint16_t usLen = pstUartInfo->pstMem->usBuffLen;
            uint16_t usIndex = *pstUartInfo->pstMem->pusLen;

            while ((LPC_UART1->LSR & UART_LSR_RDR) && (usIndex < (usLen - 1)))      
            {                
                usIndex = (*pstUartInfo->pstMem->pusLen)++;
                pucBuff[usIndex] = LPC_UART1->RBR;
            }
            LPC_UART1->FCR |= UART_FCR_RX_RS;
        }
    }
#if 1//UART_DBG
Return:
#endif
    return;
}

/*
名称: GetSendMutex()
功能:
    1.获取互斥信号量
参数:   
    1.UartID:       UART端口(UART_0 etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void GetMutex(UART_ID_Type UartID)
{
    OS_MUTEX * pMutex = astUartInfo[UartID].pMutex;
    CPU_TS ts = 0;
    OS_ERR     err = OS_ERR_NONE;
        
    if(NULL == pMutex)
    {//为NULL
        goto Return;
    }
    OSMutexPend(pMutex, 0, OS_OPT_PEND_BLOCKING, &ts, &err);//阻塞式无限期等待
Return:
    return;
}

/*
名称: PostMutex()
功能:
    1.释放互斥信号量
参数:   
    1.UartID:       UART端口(UART_0 etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void PostMutex(UART_ID_Type UartID)
{
    OS_MUTEX * pMutex = astUartInfo[UartID].pMutex;
    OS_ERR     err = OS_ERR_NONE;
        
    if(NULL == pMutex)
    {//为NULL
        goto Return;
    }
    OSMutexPost(pMutex, OS_OPT_POST_NONE, &err);
Return:
    return;
}

/*
名称: GetTimeOutCh()
功能:
    1.在Timer Isr中获取超时的timer对应的Uart Channel
参数:   
    1.TIMx:       Timer端口(LPC_TIM0 etc.)
返回值: 
    1.超时的Timer对应的Uart Channel
输入:   void
输出:   void
备注:   void
注意:   void
*/
static uint8_t GetTimeOutCh(TimerIdDef TimerId, UART_ID_Type *pUartID)
{
    TimerChDef TimerCh[TimerCHMax];//用于存储超时触发中断的 Timer Channel
    UART_ID_Type UartID = UART_0;
    uint8_t ucCnt = TimerGetCh(TimerId, &TimerCh[0]);//同时超时触发中断的 Timer Channel的个数
    uint8_t i = 0;

    for (i = 0; i < ucCnt; i++)
    {
        while(astUartInfo[UartID].pTimer->TimerCh != TimerCh[i])
        {//获取触发中断的timer对应的
            UartID = (UART_ID_Type)(UartID + 1);
        }
        pUartID[i] = UartID;
    }
    return i;
}

/*
名称: TIMER0_ISR()
功能:
    1.超时Timer Isr
参数:   
    1.UartID:       UART端口(UART_0 etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1. 由于同一timer的不同channel共用ISR，故同一timer只需定义一个ISR
*/
static void TIMER0_ISR(void)
{    
    UART_ID_Type    UartID[UART_MAX];
    stUartInfoDef   *pstUartInfo = NULL;
    stMemDef        *pstMem = NULL;
    OS_ERR          err =  OS_ERR_NONE;
    uint8_t         ucCnt = GetTimeOutCh(Timer0, &UartID[0]);//同时超时触发中断的 Timer Channel的个数
    uint8_t         i = 0;

    for (i = 0; i < ucCnt; i++)
    {//该循环用于处理多个timer channel同时触发中断的情况
        pstUartInfo = &astUartInfo[UartID[i]];
        pstMem = pstUartInfo->pstMem;
        ClrRecvFlg(UartID[i]);//清除收到包头的标志
        if (pstMem->pucBuff)
        {
            OSQPost(
                    pstUartInfo->pRecvQ, 
                    &(pstMem->pucBuff[0]), 
                    *pstMem->pusLen,
                    OS_OPT_POST_ALL | OS_OPT_POST_FIFO,
                    &err
                    );
        }
    }
    return;
}

/****************************************extern函数定义*********************************************/
/*
名称: fputc()
功能:
    1.重定义库函数中的fputc,用于将printf打印到串口
参数:   
    1.ch:       待输出数据
    2.f:        未用
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
int fputc(int ch, FILE *f)
{
/* 将Printf内容发往串口 */
    UART_PRINT_CH->THR = ch;
    while((UART_PRINT_CH->LSR & UART_LSR_THRE) == RESET){}//等待数据发送结束
    return (ch);
}

/*
名称: UartChangeBaudRate()
功能:
    1.更改串口波特率
参数:   
    1.UartID:       UART端口(UART_0 etc.)
    2.enBaudRate:   目标波特率(_9600Bps etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1. 由于底层驱动用查表方式确定指定波特率对应的寄存器值，故如需设置BandRateDef中指定波特率之外的值，需在astDivRegVal[]中添加给定波特率对应的各寄存器值
*/
void UartChangeBaudRate(UART_ID_Type UartID, BandRateDef enBaudRate)
{
    void *UARTx = astUartInfo[UartID].UARTx;
    UartSetBaudRate(UARTx, enBaudRate);
    return;
}

/*
名称: UartSend()
功能:
    1.通过串口发送指定长度的数据
参数:   
    1.UARTx: 发送端口(LPC_UART0 etc.)
    2.buff:  指向发送缓存的指针
    3.ucLen: 要发送的数据长度
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
1.由于为每个串口添加了互斥保护机制，故使用调用该函数时务必保证GetSendMutex()中的互斥信号量存在

*/
void UartSend(UART_ID_Type UartID, uint8_t const *buff, uint16_t usLen)
{
    unsigned int i;
    void *UARTx = astUartInfo[UartID].UARTx;
    __O  uint8_t  *THR = NULL;
    __I  uint8_t  *LSR  = NULL;

    if (UART_1 != UartID)
    {
        THR = &(((LPC_UART_TypeDef *)UARTx)->THR);
        LSR = &(((LPC_UART_TypeDef *)UARTx)->LSR);
    }
    else
    {
        THR = &(LPC_UART1->THR);
        LSR = &(LPC_UART1->LSR);
    }
    GetMutex(UartID);
    for(i = 0; i < usLen; i++)
    {
        *THR = buff[i]; 
        while((*LSR & UART_LSR_THRE) == RESET);//等待数据发送结束
    }        
    PostMutex(UartID);
    return;
}

/*
名称: UartBspInit()
功能:
    1. Uart Bsp初始化
参数:   
    1. UartID:   Uart Channel(UART0 etc.)
    2. ulBaudRate: Uart BaudRate(bps)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数默认配置如下:UART_Init
     * 8 data bit
     * 1 Stop bit
     * None parity
*/
void UartBspInit(UART_ID_Type UartID, BandRateDef enBaudRate)
{
    stUartInfoDef *pstUartInfo = &astUartInfo[UartID];
    
    {//UART GPIO Init
        LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;
        GpioPinCfg(pstUartInfo->pstGpio->RxPin);
        GpioPinCfg(pstUartInfo->pstGpio->TxPin);
    }
    LPC_SC->PCONP |= pstUartInfo->ulPwrEn;
    UartChangeBaudRate(UartID, enBaudRate);
    IsrRegester(pstUartInfo->enIrq,  pstUartInfo->pIsr);

    if (UART_1 != UartID)
    {//UART Cfg
        LPC_UART_TypeDef * UARTx = (LPC_UART_TypeDef * )pstUartInfo->UARTx;

        UARTx->FCR = (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV0);//每接收1Byte中断一次
        if (UART_485_CH == UartID)
        {//WYS-1型485通信水位传感器包含1bit偶校验
            UARTx->LCR = UART_LCR_WLEN8 | UART_LCR_PARITY_EN | UART_LCR_PARITY_EVEN;  // 8Bytes数据位, 1Byte停止位,无校验位,
        }
        else
        {
            UARTx->LCR = UART_LCR_WLEN8;  // 8Bytes数据位, 1Byte停止位,无校验位,
        }
        UARTx->TER |= UART_TER_TXEN;  // Enable UART Transmit
        UARTx->IER |= UART_IER_RBRINT_EN | UART_IER_RLSINT_EN;//Recv Interrupt
    }
    else
    {
        LPC_UART1->FCR = (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV0);//每接收1Byte中断一次
        LPC_UART1->LCR = UART_LCR_WLEN8;  // 8Bytes数据位, 1Byte停止位,无校验位,
        LPC_UART1->TER |= UART_TER_TXEN;  // Enable UART Transmit
        LPC_UART1->IER |= UART_IER_RBRINT_EN | UART_IER_RLSINT_EN;//Recv Interrupt
    }
    if (pstUartInfo->pTimer)
    {//定义了超时检测timer
        TimerIsrReg(pstUartInfo->pTimer->TimerId, pstUartInfo->pTimer->pTimerIsr);
    }
    return;
}

/*
名称: UartDisableCh()
功能:
    1. 失能某个UART Channel
参数:   
    1. UartID:   Uart Channel(UART0 etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void UartDisableCh(UART_ID_Type UartID)
{
    stUartInfoDef *pstUartInfo = &astUartInfo[UartID];

    
    LPC_SC->PCONP &= ~(pstUartInfo->ulPwrEn);    
    return;
}

/*
名称: UartEnableCh()
功能:
    1. 使能某个UART Channel
参数:   
    1. UartID:   Uart Channel(UART0 etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void UartEnableCh(UART_ID_Type UartID)
{
    stUartInfoDef *pstUartInfo = &astUartInfo[UartID];

    
    LPC_SC->PCONP |= (pstUartInfo->ulPwrEn);    
    return;
}


/*
名称: UartFreeBuf()
功能:
    1. Uart向OS归还内存
参数:   
    1. UartID:   Uart Channel(UART0 etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数默认配置如下:UART_Init
     * 8 data bit
     * 1 Stop bit
     * None parity
*/
void UartFreeBuf(UART_ID_Type UartID, uint8_t *pucBuf)
{
    stUartInfoDef *pstUartInfo = &astUartInfo[UartID];
    
    MemPut(pstUartInfo->pstMem->pOsMem, pucBuf);
#if OS_DBG
    if (OS_ERR_NONE != err)
    {
        printf("Mem Free Error!\r\n");
    }
#endif
    return;
}

#if BSP_UART_DBG
void UartTest(void)
{
    UartBspInit(UART_0, _9600Bps);
    UartBspInit(UART_1, _9600Bps);
    UartBspInit(UART_2, _9600Bps);
    UartBspInit(UART_3, _9600Bps);
    while(1)
    {
        uint8_t *pucData = NULL;
        uint16_t usLen = 0;
        CPU_TS   ts;
        OS_ERR   err;
        
        pucData = (uint8_t *)OSQPend(&SenSorQ, 0, OS_OPT_PEND_BLOCKING, &usLen, &ts, &err);
        UartPrintBuffer(pucData, usLen);
        UartFreeBuf(UART_2, pucData);
    }
    return;
}
#endif

