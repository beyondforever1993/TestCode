/*
�ļ�����: GPRS.c
����:
    1.����GPRS������صĺ���
����: ������
�޸ļ�¼:
    2017-4-20 �ļ�����
��ע:   void
ע��:   void
*/
#include "include.h"

/******************************************�궨��*************************************************/
#define UL865           1//
#define GPRS_DBG        1
#define RETRY_CNT       20//���Դ������壬��ͬһATָ���ط��������ڸ�ֵ����ִ��Ӳ����λ

#define HardRst         (1 << 0)
#define SoftRst         (1 << 1)

#define TIME_OUT        20//��ʱδ�յ����ݣ�ִ�����reset��ʱ����(��λΪ��)

/*******************************************����**************************************************/ 
typedef enum{
OK,//success
Error,//Error
Packet,//�յ��ְ����ݷְ�
}enChckRes;
typedef void (*const pAtFunc)(uint8_t ucState);//���ڷ���ATָ��ĺ���ָ��
typedef enChckRes (*const pChckFunc)(uint8_t *pucData, uint16_t usLen, uint8_t *pucState);//���ڼ��ģ���ATָ���Ӧ�����ݵĺ���ָ��
typedef const char * const pAtCmd;//ATָ���ַ���ָ��
typedef void    (*const pResetFunc)(uint8_t *pucState);//������λ�����ĺ���ָ��
typedef void (*const pOffLineFunc)(uint8_t *pucData, uint16_t usLen, uint8_t *pucState);//���ڼ������״̬�ĺ���ָ��

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

/*****************************************��������************************************************/

static uint8_t ucGprsState  = 0;//GPRSģ�鵱ǰ״̬
static uint8_t ucAtIndex    = 0;//���ڴ洢ATָ����صĲ�����stAtInfo[]�е�λ�ã�ִ����GprsInit()������ȷ����ģ���ͺŸ�ֵ��ȷ������δʵ��
static bool    bOnLineFlg   = 0;//��ʾ����״̬
static uint8_t ucResetFlg   = 0;//��ʾ���� Hard Reset
static const stNetParDef  *const pstNetPar = &(stRtuCfg.stNetPar);//������ز���
    
/*
�ر�ע��:
1.��ر�֤��������ÿ��Ԫ���еĸ����ṹ���Աһһ��Ӧ,
*/
static stAtInfoDef stAtInfo[] = {
#if UL865
{
    &T_ATCmd[0],    //���ATָ�������
    &UL865AtFunc[0],//ATָ��ͺ���
    &UL865ChckFunc[0],//ATָ��Ӧ���⺯��
    UL865Rest,
    UL865OffLine,
},
#endif
};

/******************************************��������***********************************************/

/****************************************static��������*********************************************/
#define SetOnLineFLg()              do{bOnLineFlg = true;}while(0)
#define ClrOnLineFLg()              do{bOnLineFlg = false;}while(0)
#define GetOnLineFlg()              (bOnLineFlg)

#define SetResetFLg(flg)              do{ucResetFlg |= (flg);}while(0)
#define ClrResetFLg(flg)              do{ucResetFlg &= ~(flg);}while(0)
#define GetResetFlg(flg)              (ucResetFlg & (flg))

#include "UL865\UL865.c"

/*
����: ResetVar()
����:
    1.  �����ļ��ж����ȫ�ֱ���ȫ������(���ò�������)
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: SendData()
����:
    1.  ��GPRSģ�鷢������
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: SendAT()
����:
    1.���������GPRSģ�������
����:   
    ucState:    �洢��ǰ��GPRS״̬�ı���(ucGprsState)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void SendAT(uint8_t ucState)
{
    stAtInfoDef *pstAtInfo = &stAtInfo[ucAtIndex];
    
    SendData((uint8_t *)pstAtInfo->paAtCmd[ucState], strlen(pstAtInfo->paAtCmd[ucState]));
    return;
}

/*
����: ChckOffLine()
����:
    1.GPRS���߼��,����⵽������ִ��SoftRest
����:   
    pucData:    ָ������ģ������ݵ�ָ��
    usLen:      ����ģ������ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void ChckOffLine(uint8_t *pucData, uint16_t usLen)
{
    stAtInfoDef *pstAtInfo = &stAtInfo[ucAtIndex];

    pstAtInfo->pOffLineFunc(pucData, usLen, &ucGprsState);
    return;
}

/*
����: ChckOk()
����:
    1.����������GPRSģ�������,�ж��Ƿ����"OK"�ַ���
����:   
    pucData:    ָ������ģ������ݵ�ָ��
    usLen:      ����ģ������ݳ���
    pucState:   ָ��洢��ǰ��GPRS״̬�ı���(ucGprsState)��ָ��
����ֵ: 
    0:      OK
    others: error
����:   void
���:   void
��ע:   void
ע��:   void
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
����: ChckConnect()
����:
    1.����������GPRSģ�������,�ж��Ƿ����"CONNECT"�ַ���
    2.����⵽"CONNECT"�ַ���������������״̬
����:   
    pucData:    ָ������ģ������ݵ�ָ��
    usLen:      ����ģ������ݳ���
    pucState:   ָ��洢��ǰ��GPRS״̬�ı���(ucGprsState)��ָ��
����ֵ: 
    0:      OK
    others: error
����:   void
���:   void
��ע:   void
ע��:   void
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
����: ChckReady()
����:
    1.����������GPRSģ�������,�ж��Ƿ����"Ready"�ַ���
����:   
    pucData:    ָ������ģ������ݵ�ָ��
    usLen:      ����ģ������ݳ���
    pucState:   ָ��洢��ǰ��GPRS״̬�ı���(ucGprsState)��ָ��
����ֵ: 
    0:      OK
    others: error
����:   void
���:   void
��ע:   void
ע��:   void
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
����: DealAT()
����:
    1.������յ���ATָ��Ӧ������
����:   
    1. pucData: ָ�������ģ������ݵ�ָ��
    2. usLen:   ������ģ������ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void DealAT(uint8_t *pucData, uint16_t usLen)
{
    stAtInfoDef     *pstAtInfo    = &stAtInfo[ucAtIndex];
    pAtFunc         *paAtFunc     = pstAtInfo->paAtFunc;//ָ��洢ATָ��ͺ����������ָ��
    pChckFunc       *paChckFunc   = pstAtInfo->paChckFunc;//ָ��洢ATָ��Ӧ���⺯���������ָ��
    enChckRes       enChckRes     = Error;
    static uint8_t  ucRetryCnt    = RETRY_CNT;
    
#if GPRS_DBG
    printf(">>:%s>>\r\n", pucData);
#endif
    if (paChckFunc[ucGprsState])
    {
        enChckRes = paChckFunc[ucGprsState](pucData, usLen, &ucGprsState);    
    }
    /*�����յ���ģ��Ӧ������ȷ����ǰģ��״̬*/
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
    {//Ӳ����λ
        SetResetFLg(HardRst);
        ucRetryCnt    = RETRY_CNT;
        goto Return;
    }
    if ((Packet != enChckRes) && (false == GetOnLineFlg()))
    {//���յ�������δ�ְ�,����δ������������
        paAtFunc[ucGprsState](ucGprsState);
    }
    
Return:
    return;
}

/*
����: SoftReset()
����:
    1.Gprs Soft Reset
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: HardReset()
����:
    1.Gprs Hard Reset
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void HardReset(void)
{
    OS_ERR err = OS_ERR_NONE;

    UartDisableCh(UART_GPRS_CH);//reset �ڼ䴮�����������쳣���˴��رմ���
    GpioClrBit(BRD_POWER_SW_GPRS_PORT, BRD_POWER_SW_GPRS_PIN);
    OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_DLY, &err);//�ȴ�2s
    //Turn_on_module_power
    GpioSetBit(BRD_POWER_SW_GPRS_PORT, BRD_POWER_SW_GPRS_PIN);
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_DLY, &err);//�ȴ�1s

    //Set_Module_ONOFF_LOW
    GpioClrBit(BRD_SW_GPRS_ON_PORT, BRD_SW_GPRS_ON_PIN);
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_DLY, &err);//�ȴ�5s //edit 2012.08.16

    //Set_Module_ONOFF_HIGH
    GpioSetBit(BRD_SW_GPRS_ON_PORT, BRD_SW_GPRS_ON_PIN);
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_DLY, &err);//�ȴ�500ms

    //Set_Module_RST_LOW
    GpioClrBit(BRD_SW_GPRS_RST_PORT, BRD_SW_GPRS_RST_PIN);
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_DLY, &err);//�ȴ�200ms

    //Set_Module_RST_HIGH
    GpioSetBit(BRD_SW_GPRS_RST_PORT, BRD_SW_GPRS_RST_PIN);
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_DLY, &err);//�ȴ�200ms
    UartEnableCh(UART_GPRS_CH);//reset �ڼ䴮�����������쳣���˴��رմ����ж�
#if GPRS_DBG
    printf("GPRS Hard Reset!\r\n");
#endif
    ClrOnLineFLg();
    ClrResetFLg(HardRst | SoftRst);
    ucGprsState = 0;
    return;
}

/*
����: Reset()
����:
    1.Gprs GPIO Reset
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: ReadPara()
����:
    1.��E2PROM�ж�ȡGPRS��������
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void ReadPara(void)
{
    
    return;
}

/*
����: GprsSend()
����:
    1.ͨ��GPRSģ�鷢����������
����:   
    pucData:    ָ������ģ������ݵ�ָ��
    usLen:      ����ģ������ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: ResetProc(enGprsRestDef)
����:
    1.ִ�и�λ����
����:   void
����ֵ: 
    0: ִ��Reset
    1: δִ��Reset
����:   void
���:   void
��ע:   void
ע��:   void
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
����: ResetChck(enGprsRestDef)
����:
    1.����Ƿ���Ҫִ�и�λ����
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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

/****************************************extern��������*********************************************/

/*
����: GprsInit()
����:
    1.  ִ��GPRS Hard Reset
    2.  ����״̬��ѯָ��
    3.  ���㱾�������õ���ȫ�ֱ���
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void GprsInit(void)
{
    GpioInit();
    HardReset();
    ResetVar();
    ReadPara();
    UartBspInit(UART_GPRS_CH, UART1_BPS);//
    SendAT(ucGprsState);
    //������Ӽ���ʱ��ͨ���ڴ˴���Ӳ�ѯģ������ָ�������� // UL865ģ��Ķ�Ӧָ��ΪCGMI/CGMM
    return;
}

/*
����: TaskGprsS
����:
    1.GPRS Data Send Task
����:   
    p_arg: ����������β�
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
        OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_DLY, &err);//�ȴ�2s
        GprsSend((uint8_t *)"duzailian\r\n", strlen("duzailian\r\n"));
    #endif
    }
    return;
}

/*
����: TaskGprsR
����:
    1.GPRS Data Recv Task
����:   
    p_arg: ����������β�
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void  TaskGprsR(void *p_arg)
{
    uint8_t     *pucData = NULL;
    uint16_t    usLen = 0;
    uint8_t     ucCnt = 0;//���ڳ�ʱ���
    uint8_t     ucReset = 0;//Ϊ0��ʾ��ִ��Reset,�����ʾδִ��Reset
    CPU_TS      ts;
    OS_ERR      err;
    
    GprsInit();
    
    while(1)
    {
        pucData = (uint8_t *)OSQPend(&GprsRecvQ, OS_CFG_TICK_RATE_HZ, OS_OPT_PEND_BLOCKING, &usLen, &ts, &err);//���˴���ʱʱ��ı䣬��ͬ������ResetProc()�еı���ֵ

        ucReset = ResetProc();
        if (0 == ucReset)
        {//��ִ��Reset
            ucCnt = 0;
        }
        if (OS_ERR_NONE == err)
        {//�յ�������δִ��Reset
            ChckOffLine(pucData, usLen);
            if (GetOnLineFlg())
            {//����
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
        {//��ʱδ�յ���������
            ucCnt++;
            ResetChck(ucCnt);
        }
    }
    return;
}

