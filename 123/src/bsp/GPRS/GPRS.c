/*
文件名称: GPRS.c
功能:
    1.包含GPRS驱动相关的函数
作者: 杜在连
修改记录:
    2017-4-20 文件初创
备注:   void
注意:   void
*/
#include "include.h"

/******************************************宏定义*************************************************/
#define UL865           1//
#define GPRS_DBG        1
#define RETRY_CNT       20//重试次数定义，若同一AT指令重发次数等于该值，则执行硬件复位

#define HardRst         (1 << 0)
#define SoftRst         (1 << 1)

#define TIME_OUT        20//超时未收到数据，执行软件reset的时间间隔(单位为秒)

/*******************************************声明**************************************************/ 
typedef enum{
OK,//success
Error,//Error
Packet,//收到分包数据分包
}enChckRes;
typedef void (*const pAtFunc)(uint8_t ucState);//用于发送AT指令的函数指针
typedef enChckRes (*const pChckFunc)(uint8_t *pucData, uint16_t usLen, uint8_t *pucState);//用于检测模块对AT指令的应答数据的函数指针
typedef const char * const pAtCmd;//AT指令字符串指针
typedef void    (*const pResetFunc)(uint8_t *pucState);//用于软复位操作的函数指针
typedef void (*const pOffLineFunc)(uint8_t *pucData, uint16_t usLen, uint8_t *pucState);//用于检测离线状态的函数指针

static void HardReset(void);
	
typedef const struct{
pAtCmd      *paAtCmd;
pAtFunc     *paAtFunc;
pChckFunc   *paChckFunc;
pResetFunc  pResetFunc;
pOffLineFunc  pOffLineFunc;
}stAtInfoDef;

static void SendAT(uint8_t ucIndex);
static enChckRes ChckOk(uint8_t *pucData, uint16_t usLen, uint8_t *pucState);
static enChckRes ChckReady(uint8_t *pucData, uint16_t usLen, uint8_t *pucState);
static enChckRes ChckConnect(uint8_t *pucData, uint16_t usLen, uint8_t *pucState);
static void SendData(uint8_t *pucData, uint16_t usLen);

#include "ul865\ul865.h"

/*****************************************变量定义************************************************/

static uint8_t ucGprsState  = 0;//GPRS模块当前状态
static uint8_t ucAtIndex    = 0;//用于存储AT指令相关的操作在stAtInfo[]中的位置，执行完GprsInit()函数，确定了模块型号该值即确定，暂未实现
static bool    bOnLineFlg   = 0;//表示在线状态
static uint8_t ucResetFlg   = 0;//表示请求 Hard Reset
static const stNetParDef  *const pstNetPar = &(stRtuCfg.stNetPar);//联网相关参数
    
/*
特别注意:
1.务必保证该数组中每个元素中的各个结构体成员一一对应,
*/
static stAtInfoDef stAtInfo[] = {
#if UL865
{
    &T_ATCmd[0],    //存放AT指令集的数组
    &UL865AtFunc[0],//AT指令发送函数
    &UL865ChckFunc[0],//AT指令应答检测函数
    UL865Rest,
    UL865OffLine,
},
#endif
};

/******************************************函数定义***********************************************/

/****************************************static函数定义*********************************************/
#define SetOnLineFLg()              do{bOnLineFlg = true;}while(0)
#define ClrOnLineFLg()              do{bOnLineFlg = false;}while(0)
#define GetOnLineFlg()              (bOnLineFlg)

#define SetResetFLg(flg)              do{ucResetFlg |= (flg);}while(0)
#define ClrResetFLg(flg)              do{ucResetFlg &= ~(flg);}while(0)
#define GetResetFlg(flg)              (ucResetFlg & (flg))

#include "UL865\UL865.c"

/*
名称: ResetVar()
功能:
    1.  将本文件中定义的全局变量全部清零(配置参数除外)
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void ResetVar(void)
{
    ucGprsState = 0;
#if 0//GPRS_DBG
    strcpy(stGprsCfg.IP, "61.172.254.184");
    stGprsCfg.usPort = 5601;
    strcpy(stGprsCfg.APN_Num, "CMNET");
    stGprsCfg.ucType = 0;
#endif
    return;
}

/*
名称: SendData()
功能:
    1.  向GPRS模块发送数据
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void SendData(uint8_t *pucData, uint16_t usLen)
{
#if GPRS_DBG
    printf("<<");
    UartSend(UART_COM_CH, pucData, usLen);
    printf("<<");
#endif
    UartSend(UART_GPRS_CH, pucData, usLen);
    return;
}

/*
名称: SendAT()
功能:
    1.处理接收自GPRS模块的数据
参数:   
    ucState:    存储当前的GPRS状态的变量(ucGprsState)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void SendAT(uint8_t ucState)
{
    stAtInfoDef *pstAtInfo = &stAtInfo[ucAtIndex];
    
    SendData((uint8_t *)pstAtInfo->paAtCmd[ucState], strlen(pstAtInfo->paAtCmd[ucState]));
    return;
}

/*
名称: ChckOffLine()
功能:
    1.GPRS离线检测,若检测到离线则执行SoftRest
参数:   
    pucData:    指向来自模块的数据的指针
    usLen:      来自模块的数据长度
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void ChckOffLine(uint8_t *pucData, uint16_t usLen)
{
    stAtInfoDef *pstAtInfo = &stAtInfo[ucAtIndex];

    pstAtInfo->pOffLineFunc(pucData, usLen, &ucGprsState);
    return;
}

/*
名称: ChckOk()
功能:
    1.搜索接收自GPRS模块的数据,判断是否包含"OK"字符串
参数:   
    pucData:    指向来自模块的数据的指针
    usLen:      来自模块的数据长度
    pucState:   指向存储当前的GPRS状态的变量(ucGprsState)的指针
返回值: 
    0:      OK
    others: error
输入:   void
输出:   void
备注:   void
注意:   void
*/
static enChckRes ChckOk(uint8_t *pucData, uint16_t usLen, uint8_t *pucState)
{
    enChckRes enRes = Error;

    if (strstr((char *)pucData, "OK"))
    {
        enRes = OK;
    }
    else if (strstr((char *)pucData, "ERROR"))
    {
        enRes = Error;
    }
    else
    {
        enRes = Packet;
    }
    return enRes;
}

/*
名称: ChckConnect()
功能:
    1.搜索接收自GPRS模块的数据,判断是否包含"CONNECT"字符串
    2.若检测到"CONNECT"字符串，则设置在线状态
参数:   
    pucData:    指向来自模块的数据的指针
    usLen:      来自模块的数据长度
    pucState:   指向存储当前的GPRS状态的变量(ucGprsState)的指针
返回值: 
    0:      OK
    others: error
输入:   void
输出:   void
备注:   void
注意:   void
*/
static enChckRes ChckConnect(uint8_t *pucData, uint16_t usLen, uint8_t *pucState)
{
    enChckRes enRes = Packet;

    if (strstr((char *)pucData, "CONNECT"))
    {
        enRes = OK;
        SetOnLineFLg();//
    }
    else if (strstr((char *)pucData, "ERROR"))
    {
        enRes = Error;
    }
    else
    {
        enRes = Packet;
    }
    return enRes;
}


/*
名称: ChckReady()
功能:
    1.搜索接收自GPRS模块的数据,判断是否包含"Ready"字符串
参数:   
    pucData:    指向来自模块的数据的指针
    usLen:      来自模块的数据长度
    pucState:   指向存储当前的GPRS状态的变量(ucGprsState)的指针
返回值: 
    0:      OK
    others: error
输入:   void
输出:   void
备注:   void
注意:   void
*/
static enChckRes ChckReady(uint8_t *pucData, uint16_t usLen, uint8_t *pucState)
{
    enChckRes enRes = Packet;

    if (strstr((char *)pucData, "READY"))
    {
        enRes = OK;
    }
    else 
    {
        enRes = Packet;
    }
    return enRes;
}

/*
名称: DealAT()
功能:
    1.处理接收到的AT指令应答数据
参数:   
    1. pucData: 指向接收自模块的数据的指针
    2. usLen:   接收自模块的数据长度
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void DealAT(uint8_t *pucData, uint16_t usLen)
{
    stAtInfoDef     *pstAtInfo    = &stAtInfo[ucAtIndex];
    pAtFunc         *paAtFunc     = pstAtInfo->paAtFunc;//指向存储AT指令发送函数的数组的指针
    pChckFunc       *paChckFunc   = pstAtInfo->paChckFunc;//指向存储AT指令应答检测函数的数组的指针
    enChckRes       enChckRes     = Error;
    static uint8_t  ucRetryCnt    = RETRY_CNT;
    
#if GPRS_DBG
    printf(">>:%s>>\r\n", pucData);
#endif
    if (paChckFunc[ucGprsState])
    {
        enChckRes = paChckFunc[ucGprsState](pucData, usLen, &ucGprsState);    
    }
    /*根据收到的模块应答数据确定当前模块状态*/
    if (OK == enChckRes)
    {
        ucRetryCnt = RETRY_CNT;
        ucGprsState++;
    }
    else
    {
        ucRetryCnt--;
    }
    if (0 == ucRetryCnt)
    {//硬件复位
        SetResetFLg(HardRst);
        ucRetryCnt    = RETRY_CNT;
        goto Return;
    }
    if ((Packet != enChckRes) && (false == GetOnLineFlg()))
    {//接收到的数据未分包,且尚未结束联网流程
        paAtFunc[ucGprsState](ucGprsState);
    }
    
Return:
    return;
}

/*
名称: SoftReset()
功能:
    1.Gprs Soft Reset
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void SoftReset(void)
{
#if GPRS_DBG
    printf("GPRS Soft Reset!\r\n");
#endif
    stAtInfo[ucAtIndex].pResetFunc(&ucGprsState);
    ClrResetFLg(SoftRst);
    return;
}

/*
名称: HardReset()
功能:
    1.Gprs Hard Reset
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void HardReset(void)
{
    OS_ERR err = OS_ERR_NONE;

    UartDisableCh(UART_GPRS_CH);//reset 期间串口数据总是异常，此处关闭串口
    GpioClrBit(BRD_POWER_SW_GPRS_PORT, BRD_POWER_SW_GPRS_PIN);
    OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_DLY, &err);//等待2s
    //Turn_on_module_power
    GpioSetBit(BRD_POWER_SW_GPRS_PORT, BRD_POWER_SW_GPRS_PIN);
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_DLY, &err);//等待1s

    //Set_Module_ONOFF_LOW
    GpioClrBit(BRD_SW_GPRS_ON_PORT, BRD_SW_GPRS_ON_PIN);
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_DLY, &err);//等待5s //edit 2012.08.16

    //Set_Module_ONOFF_HIGH
    GpioSetBit(BRD_SW_GPRS_ON_PORT, BRD_SW_GPRS_ON_PIN);
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_DLY, &err);//等待500ms

    //Set_Module_RST_LOW
    GpioClrBit(BRD_SW_GPRS_RST_PORT, BRD_SW_GPRS_RST_PIN);
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_DLY, &err);//等待200ms

    //Set_Module_RST_HIGH
    GpioSetBit(BRD_SW_GPRS_RST_PORT, BRD_SW_GPRS_RST_PIN);
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_DLY, &err);//等待200ms
    UartEnableCh(UART_GPRS_CH);//reset 期间串口数据总是异常，此处关闭串口中断
#if GPRS_DBG
    printf("GPRS Hard Reset!\r\n");
#endif
    ClrOnLineFLg();
    ClrResetFLg(HardRst | SoftRst);
    ucGprsState = 0;
    return;
}

/*
名称: Reset()
功能:
    1.Gprs GPIO Reset
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void GpioInit(void)
{
    LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;
    GpioSetDir(BRD_POWER_SW_GPRS_PORT, BRD_POWER_SW_GPRS_PIN, 1);
    GpioSetDir(BRD_SW_GPRS_ON_PORT, BRD_SW_GPRS_ON_PIN, 1);
    GpioSetDir(BRD_SW_GPRS_RST_PORT, BRD_SW_GPRS_RST_PIN, 1);
    return;
}


/*
名称: ReadPara()
功能:
    1.从E2PROM中读取GPRS联网参数
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void ReadPara(void)
{
    
    return;
}

/*
名称: GprsSend()
功能:
    1.通过GPRS模块发送网络数据
参数:   
    pucData:    指向来自模块的数据的指针
    usLen:      来自模块的数据长度
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void GprsSend(uint8_t *pucData, uint16_t usLen)
{
    if (GetOnLineFlg())
    {
        SendData(pucData, usLen);
    }
#if GPRS_DBG
    else
    {
        printf("GPRS OffLine!\r\n");
    }
#endif
    return;
}

/*
名称: ResetProc(enGprsRestDef)
功能:
    1.执行复位操作
参数:   void
返回值: 
    0: 执行Reset
    1: 未执行Reset
输入:   void
输出:   void
备注:   void
注意:   void
*/
static uint8_t ResetProc(void)
{        
    uint8_t ucRes = 0xff;
    
    if (GetResetFlg(HardRst))
    {
        ucRes = 0;
        HardReset();
        SendAT(ucGprsState);
    }
    if (GetResetFlg(SoftRst))
    {
        ucRes = 0;
        SoftReset();
    }
    return ucRes;
}

/*
名称: ResetChck(enGprsRestDef)
功能:
    1.检查是否需要执行复位操作
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void ResetChck(uint8_t ucTimeCnt)
{    
    static uint8_t  ucRetryCnt = RETRY_CNT;
    
    if (ucTimeCnt >= TIME_OUT)
    {
        SetResetFLg(SoftRst);
        if (ucRetryCnt)
        {
            ucRetryCnt--;
        }
    }
    else
    {
        ucRetryCnt = RETRY_CNT;
    }
    if (0 == ucRetryCnt)
    {
        SetResetFLg(HardRst);
    }
    return;
}

/****************************************extern函数定义*********************************************/

/*
名称: GprsInit()
功能:
    1.  执行GPRS Hard Reset
    2.  发送状态查询指令
    3.  清零本函数中用到的全局变量
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void GprsInit(void)
{
    GpioInit();
    HardReset();
    ResetVar();
    ReadPara();
    UartBspInit(UART_GPRS_CH, UART1_BPS);//
    SendAT(ucGprsState);
    //后续添加兼容时可通过在此处添加查询模块类型指令做区分 // UL865模块的对应指令为CGMI/CGMM
    return;
}

/*
名称: TaskGprsS
功能:
    1.GPRS Data Send Task
参数:   
    p_arg: 传入任务的形参
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void  TaskGprsS(void *p_arg)
{
#if !GPRS_DBG
    uint8_t     *pucData = NULL;
    uint16_t    usLen = 0;
    CPU_TS   ts;
#endif
    OS_ERR   err;

    while(1)
    {
    #if !GPRS_DBG
        pucData = (uint8_t *)OSQPend(&GprsSendQ, OS_CFG_TICK_RATE_HZ, OS_OPT_PEND_BLOCKING, &usLen, &ts, &err);
        if (OS_ERR_NONE == err)
        {
            GprsSend(pucData, usLen);
        }
    #else
        OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_DLY, &err);//等待2s
        GprsSend((uint8_t *)"duzailian\r\n", strlen("duzailian\r\n"));
    #endif
    }
    return;
}

/*
名称: TaskGprsR
功能:
    1.GPRS Data Recv Task
参数:   
    p_arg: 传入任务的形参
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void  TaskGprsR(void *p_arg)
{
    uint8_t     *pucData = NULL;
    uint16_t    usLen = 0;
    uint8_t     ucCnt = 0;//用于超时检测
    uint8_t     ucReset = 0;//为0表示已执行Reset,非零表示未执行Reset
    CPU_TS      ts;
    OS_ERR      err;
    
    GprsInit();
    
    while(1)
    {
        pucData = (uint8_t *)OSQPend(&GprsRecvQ, OS_CFG_TICK_RATE_HZ, OS_OPT_PEND_BLOCKING, &usLen, &ts, &err);//若此处超时时间改变，需同步更改ResetProc()中的变量值

        ucReset = ResetProc();
        if (0 == ucReset)
        {//已执行Reset
            ucCnt = 0;
        }
        if (OS_ERR_NONE == err)
        {//收到数据且未执行Reset
            ChckOffLine(pucData, usLen);
            if (GetOnLineFlg())
            {//在线
            #if !GPRS_DBG
                ProDeal(pucData, usLen);
            #else
                printf(">>%s>>", (char *)pucData);
            #endif
                ucCnt = 0;
            }
            else
            {
                DealAT(pucData, usLen);
            }
            UartFreeBuf(UART_GPRS_CH, pucData);
        }
        else
        {//超时未收到心跳数据
            ucCnt++;
            ResetChck(ucCnt);
        }
    }
    return;
}

