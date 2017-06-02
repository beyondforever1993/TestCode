/*
�ļ�����: bsp_uart.c
����:
    1.����Uart������صĺ���
����: ������
�޸ļ�¼:
    2017-3-23 �ļ�����
��ע:
    1.������������˿�������ֻ����astUartFifo[], astUartInfo[]���������Ӧλ�������������ò�������,
    2.�������ж�ʹ��,����astIRQnInfo[]�����������ò���
ע��:
    1.���ļ��У�ֻ�����˽����жϡ�������Ҫ���Ը���UartBspInit()������astIRQnInfo[]����
    2.�ļ�����ʱ��������:
        ��ʼλ��1  
        ����λ��8  
        ֹͣλ��1  
        У�飺  �� 
     ������Ҫ�ɸ�����Ҫ�޸�UartBspInit()����
    3.�ļ�����ʱ��ֻ������UART0-3��Ӧ�ã�������Ҫ�������LPC1778 User Manual��Ӷ�UART4�ļ���
*/


#include "include.h"

/******************************************�궨��***********************************************/

/*******************************************����************************************************/
#pragma pack(1)

typedef const struct {
stPinCfgDef TxPin;
stPinCfgDef RxPin;
}stGpioDef;

typedef const struct {//���峬ʱ�����timer�����Ϣ
TimerIdDef  TimerId;// Timer0 etc.
TimerChDef  TimerCh;//  timer channel(TimerCH0)
pIsrFunc    pTimerIsr;//���ڳ�ʱ����  Timer ISR (ע�⣬���ֶα����� TIMx ��ƥ��)
uint32_t ulVal;//��Ϊ�յ���β�Ľ�����ʱʱ��(������ʱ��λ�ο�stTimerInfoDef�е�ulPresValueԪ��)
}stTimerDef;

typedef struct{
OS_MEM   *const pOsMem;//ָ��OS�洢�������ƿ��ָ�� (MemSmall64 etc)
uint8_t  *pucBuff;//ָ���OS������ڴ���ʼ��ַ��ָ��(�ɳ�ʱ��� Timer ����ָ�������Ϣ����)
uint16_t *pusLen;//ָ��洢���յ������ݳ��ȵ�ָ��
uint16_t  usBuffLen;//���뵽���ڴ泤��
}stMemDef;

typedef const struct {
void * UARTx;
stGpioDef   *pstGpio;
IRQn_Type    enIrq;     //�жϱ��(UART0_IRQn etc.)
uint32_t     ulPwrEn;//Power enable(CLKPWR_PCONP_PCUART0 etc.)
pUartIsr     pIsr;      //ָ��Isr��ָ��
stMemDef    *pstMem;
//stFiFoDef   *pFiFo;           //����FIFO
OS_MUTEX    *pMutex;//�����ź���ΪNULL��ʾ����Ҫ���Ᵽ��
OS_Q        *pRecvQ;//���ݽ�����Ϣ����
stTimerDef  *pTimer;//���峬ʱ�����timer�����Ϣ(���ڼ���β)
}stUartInfoDef;
#pragma pack()

static void UARTx_ISR(void);
static void UART1_ISR(void);
static void TIMER0_ISR(void);

/*****************************************��������************************************************/
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

/*���峬ʱ�����timer�����Ϣ(���ڼ���β)*/
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
        /* GPIO         �жϱ��       PWR ENR             ISR        OS_MEM          �����ź���     ���ն���*/   /*timer*/
        &astGpio[0],  UART0_IRQn, CLKPWR_PCONP_PCUART0, UARTx_ISR,    &astMem[0],    &COMSendM,     &COMRecvQ,   &aTimer[0]
    },
    {/*UART1*/  /*UART_GPRS_CH*/
        (void *)LPC_UART1,
        /*GPIO         �жϱ��       PWR ENR             ISR         OS_MEM         �����ź���     ���ն���*/   /*timer*/
        &astGpio[1], UART1_IRQn, CLKPWR_PCONP_PCUART1, UART1_ISR,    &astMem[1],     &GprsSendM,    &GprsRecvQ,   &aTimer[1]
    },
    {/*UART2*/  /*UART_485_CH*/
        (void *)LPC_UART2,
        /*GPIO         �жϱ��       PWR ENR             ISR         OS_MEM          �����ź���      ���ն���*/     /*timer*/
        &astGpio[2], UART2_IRQn, CLKPWR_PCONP_PCUART2, UARTx_ISR,    &astMem[2],      &SenSorSendM,   &SenSorQ,     &aTimer[2]
    },
    {/*UART3*/  /*UART_BD_CH*/
        (void *)LPC_UART3,
        /*GPIO          �жϱ��       CLK ENR             ISR        OS_MEM        �����ź���    ���ն���*/     /*timer*/
        &astGpio[3],    UART3_IRQn, CLKPWR_PCONP_PCUART3, UARTx_ISR,   &astMem[3],    &BDSendM,     &BDRecvQ,      &aTimer[3]
    },
};

static volatile uint8_t ucRecvFlg = 0;//���յ���ͷ�ı�־(bitӳ��)(�� Uart ISR ��λ��timer ISR ��ȷ���յ������������������ݼ�����Ϣ����֮������) 

/******************************************��������*****************************************************/
/****************************************static��������*********************************************/
#define SetRecvFlg(UartID)              do{ucRecvFlg |= (1 << UartID);}while(0)
#define ClrRecvFlg(UartID)              do{ucRecvFlg &= ~(1 << UartID);}while(0)
#define GetRecvFlg(UartID)              (ucRecvFlg & (1 << UartID))

/*
����: GetActiveCh()
����:
    1.��ȡ��ǰISR��Ӧ��Uart Channel
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: GetBuffer()
����:
    1.�ж��Ƿ���յ���ͷ,���յ���ͷ����OS�����ڴ�,������β���Timer, �������յ��İ�������,�����ػ���ָ��
    2.��֮ǰ���յ���ͷ����ú���ˢ�°�β���Timer�����ػ���ָ��
����:   void
����ֵ: 
    ָ�����ݻ����ָ��
����:   void
���:   void
��ע:   void
ע��:   void
*/
static uint8_t * GetBuffer(UART_ID_Type UartID)
{// Recv Success
    stUartInfoDef *pstUartInfo = &astUartInfo[UartID];
    uint8_t     *pucBuff = NULL;//ָ���OS������ڴ���ʼ��ַ��ָ��(�ɳ�ʱ��� Timer ����ָ�������Ϣ����)
    stTimerDef  *pTimer = pstUartInfo->pTimer;//���峬ʱ�����timer�����Ϣ(���ڼ���β)

    if (0 == GetRecvFlg(UartID))
    {//��һ���յ���ͷ,�����ڴ�
        pucBuff = (uint8_t *)MemGet(pstUartInfo->pstMem->pOsMem);
        if ((NULL == pucBuff) || (NULL == pTimer))
        {//�ڴ�����ʧ�ܻ�δʹ��timer���ڳ�ʱ���
            goto Return;
        }
        TimerStartCh(pTimer->TimerId, pTimer->TimerCh, pTimer->ulVal);
        SetRecvFlg(UartID);//ֻ������ɹ��������� Timer ���������յ�ͷ�ı�־���������ڴ�����ʧ��,δ���� Timer �������Զ���
        *(pstUartInfo->pstMem->pusLen)  = 0;//�յ�ͷ�����ݳ�����Ϊ0
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
����: UARTx_ISR()
����:
    1.UART0-UART3 ISR
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void UARTx_ISR(void)
{
    UART_ID_Type UartID = GetActiveCh();
    stUartInfoDef *pstUartInfo = &astUartInfo[UartID];
    LPC_UART_TypeDef * UARTx = pstUartInfo->UARTx;
    
    if (UARTx->IIR  & UART_IIR_INTID_RLS)
    {//Recv Error ע��:����������Ҳ�����case������ԭ��LPC1778 user manual δ˵��
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
        (void)ucTmp;//��ֹUART_DBGΪ0ʱ�ı���������
    #endif
    }

    if (UARTx->IIR  & UART_IIR_INTID_RDA)
    {
        uint8_t *pucBuff = GetBuffer(UartID);//ָ���OS������ڴ���ʼ��ַ��ָ��(�ɳ�ʱ��� Timer ����ָ�������Ϣ����)

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
����: UART1_ISR()
����:
    1.UART1 ISR
����:   
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
        uint8_t *pucBuff = GetBuffer(UART_1);//ָ���OS������ڴ���ʼ��ַ��ָ��(�ɳ�ʱ��� Timer ����ָ�������Ϣ����)

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
����: GetSendMutex()
����:
    1.��ȡ�����ź���
����:   
    1.UartID:       UART�˿�(UART_0 etc.)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void GetMutex(UART_ID_Type UartID)
{
    OS_MUTEX * pMutex = astUartInfo[UartID].pMutex;
    CPU_TS ts = 0;
    OS_ERR     err = OS_ERR_NONE;
        
    if(NULL == pMutex)
    {//ΪNULL
        goto Return;
    }
    OSMutexPend(pMutex, 0, OS_OPT_PEND_BLOCKING, &ts, &err);//����ʽ�����ڵȴ�
Return:
    return;
}

/*
����: PostMutex()
����:
    1.�ͷŻ����ź���
����:   
    1.UartID:       UART�˿�(UART_0 etc.)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void PostMutex(UART_ID_Type UartID)
{
    OS_MUTEX * pMutex = astUartInfo[UartID].pMutex;
    OS_ERR     err = OS_ERR_NONE;
        
    if(NULL == pMutex)
    {//ΪNULL
        goto Return;
    }
    OSMutexPost(pMutex, OS_OPT_POST_NONE, &err);
Return:
    return;
}

/*
����: GetTimeOutCh()
����:
    1.��Timer Isr�л�ȡ��ʱ��timer��Ӧ��Uart Channel
����:   
    1.TIMx:       Timer�˿�(LPC_TIM0 etc.)
����ֵ: 
    1.��ʱ��Timer��Ӧ��Uart Channel
����:   void
���:   void
��ע:   void
ע��:   void
*/
static uint8_t GetTimeOutCh(TimerIdDef TimerId, UART_ID_Type *pUartID)
{
    TimerChDef TimerCh[TimerCHMax];//���ڴ洢��ʱ�����жϵ� Timer Channel
    UART_ID_Type UartID = UART_0;
    uint8_t ucCnt = TimerGetCh(TimerId, &TimerCh[0]);//ͬʱ��ʱ�����жϵ� Timer Channel�ĸ���
    uint8_t i = 0;

    for (i = 0; i < ucCnt; i++)
    {
        while(astUartInfo[UartID].pTimer->TimerCh != TimerCh[i])
        {//��ȡ�����жϵ�timer��Ӧ��
            UartID = (UART_ID_Type)(UartID + 1);
        }
        pUartID[i] = UartID;
    }
    return i;
}

/*
����: TIMER0_ISR()
����:
    1.��ʱTimer Isr
����:   
    1.UartID:       UART�˿�(UART_0 etc.)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1. ����ͬһtimer�Ĳ�ͬchannel����ISR����ͬһtimerֻ�趨��һ��ISR
*/
static void TIMER0_ISR(void)
{    
    UART_ID_Type    UartID[UART_MAX];
    stUartInfoDef   *pstUartInfo = NULL;
    stMemDef        *pstMem = NULL;
    OS_ERR          err =  OS_ERR_NONE;
    uint8_t         ucCnt = GetTimeOutCh(Timer0, &UartID[0]);//ͬʱ��ʱ�����жϵ� Timer Channel�ĸ���
    uint8_t         i = 0;

    for (i = 0; i < ucCnt; i++)
    {//��ѭ�����ڴ�����timer channelͬʱ�����жϵ����
        pstUartInfo = &astUartInfo[UartID[i]];
        pstMem = pstUartInfo->pstMem;
        ClrRecvFlg(UartID[i]);//����յ���ͷ�ı�־
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

/****************************************extern��������*********************************************/
/*
����: fputc()
����:
    1.�ض���⺯���е�fputc,���ڽ�printf��ӡ������
����:   
    1.ch:       ���������
    2.f:        δ��
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
int fputc(int ch, FILE *f)
{
/* ��Printf���ݷ������� */
    UART_PRINT_CH->THR = ch;
    while((UART_PRINT_CH->LSR & UART_LSR_THRE) == RESET){}//�ȴ����ݷ��ͽ���
    return (ch);
}

/*
����: UartChangeBaudRate()
����:
    1.���Ĵ��ڲ�����
����:   
    1.UartID:       UART�˿�(UART_0 etc.)
    2.enBaudRate:   Ŀ�겨����(_9600Bps etc.)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1. ���ڵײ������ò��ʽȷ��ָ�������ʶ�Ӧ�ļĴ���ֵ������������BandRateDef��ָ��������֮���ֵ������astDivRegVal[]����Ӹ��������ʶ�Ӧ�ĸ��Ĵ���ֵ
*/
void UartChangeBaudRate(UART_ID_Type UartID, BandRateDef enBaudRate)
{
    void *UARTx = astUartInfo[UartID].UARTx;
    UartSetBaudRate(UARTx, enBaudRate);
    return;
}

/*
����: UartSend()
����:
    1.ͨ�����ڷ���ָ�����ȵ�����
����:   
    1.UARTx: ���Ͷ˿�(LPC_UART0 etc.)
    2.buff:  ָ���ͻ����ָ��
    3.ucLen: Ҫ���͵����ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
1.����Ϊÿ����������˻��Ᵽ�����ƣ���ʹ�õ��øú���ʱ��ر�֤GetSendMutex()�еĻ����ź�������

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
        while((*LSR & UART_LSR_THRE) == RESET);//�ȴ����ݷ��ͽ���
    }        
    PostMutex(UartID);
    return;
}

/*
����: UartBspInit()
����:
    1. Uart Bsp��ʼ��
����:   
    1. UartID:   Uart Channel(UART0 etc.)
    2. ulBaudRate: Uart BaudRate(bps)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú���Ĭ����������:UART_Init
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

        UARTx->FCR = (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV0);//ÿ����1Byte�ж�һ��
        if (UART_485_CH == UartID)
        {//WYS-1��485ͨ��ˮλ����������1bitżУ��
            UARTx->LCR = UART_LCR_WLEN8 | UART_LCR_PARITY_EN | UART_LCR_PARITY_EVEN;  // 8Bytes����λ, 1Byteֹͣλ,��У��λ,
        }
        else
        {
            UARTx->LCR = UART_LCR_WLEN8;  // 8Bytes����λ, 1Byteֹͣλ,��У��λ,
        }
        UARTx->TER |= UART_TER_TXEN;  // Enable UART Transmit
        UARTx->IER |= UART_IER_RBRINT_EN | UART_IER_RLSINT_EN;//Recv Interrupt
    }
    else
    {
        LPC_UART1->FCR = (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV0);//ÿ����1Byte�ж�һ��
        LPC_UART1->LCR = UART_LCR_WLEN8;  // 8Bytes����λ, 1Byteֹͣλ,��У��λ,
        LPC_UART1->TER |= UART_TER_TXEN;  // Enable UART Transmit
        LPC_UART1->IER |= UART_IER_RBRINT_EN | UART_IER_RLSINT_EN;//Recv Interrupt
    }
    if (pstUartInfo->pTimer)
    {//�����˳�ʱ���timer
        TimerIsrReg(pstUartInfo->pTimer->TimerId, pstUartInfo->pTimer->pTimerIsr);
    }
    return;
}

/*
����: UartDisableCh()
����:
    1. ʧ��ĳ��UART Channel
����:   
    1. UartID:   Uart Channel(UART0 etc.)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void UartDisableCh(UART_ID_Type UartID)
{
    stUartInfoDef *pstUartInfo = &astUartInfo[UartID];

    
    LPC_SC->PCONP &= ~(pstUartInfo->ulPwrEn);    
    return;
}

/*
����: UartEnableCh()
����:
    1. ʹ��ĳ��UART Channel
����:   
    1. UartID:   Uart Channel(UART0 etc.)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void UartEnableCh(UART_ID_Type UartID)
{
    stUartInfoDef *pstUartInfo = &astUartInfo[UartID];

    
    LPC_SC->PCONP |= (pstUartInfo->ulPwrEn);    
    return;
}


/*
����: UartFreeBuf()
����:
    1. Uart��OS�黹�ڴ�
����:   
    1. UartID:   Uart Channel(UART0 etc.)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú���Ĭ����������:UART_Init
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

