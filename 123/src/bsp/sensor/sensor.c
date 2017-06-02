/*
�ļ�����: sensor.c
����:
    1.����sensor������صĺ���
����: ������
�޸ļ�¼:
    2017-4-20 �ļ�����
��ע:   void 
ע��:   void
*/
#include "include.h"

#include "temp\temp.c"
#include "485\485.c"
#include "rain\rain.c"

/******************************************�궨��*************************************************/

/*******************************************����**************************************************/
#pragma pack(1)
typedef struct{//ʱ�䶨��
uint8_t ucYear;//��
uint8_t ucMonth;//��
uint8_t ucDay;//��
uint8_t ucHour;//ʱ
uint8_t ucMin;//��
}stTDef;
#pragma pack()

typedef void (* const pFunc)(void *);//���������ݲɼ�����
typedef struct {//�ýṹ���е����ݻ��������aCollFunc[]�еĲɼ�����һһ��Ӧ,�����ݴ洢���ȱ���ͳһΪ DATA_SZ
stTDef  stTime;//YYMMDDHHmm
st485Def *pst485Def;//485�ɼ���ˮλ����
stRainDef *pstRainDef;//�洢��ǰ��ˮ��
}stDataDef;
/*****************************************��������************************************************/
static pFunc aCollFunc[] = {//���������ݲɼ��������� �������е�Ԫ�ر�����stDataDef�е����ݻ���һһ��Ӧ,
_485Proc,//485���ݲɼ�����
}; 
static stDataDef stData = {0};
static stSenCfgDef  *const pstSenCfg = &(stRtuCfg.stSenCfg);

/******************************************��������***********************************************/

/****************************************static��������*********************************************/

/*
����: GetTime()
����:
    1.��ȡʱ��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void  GetTime(stTDef  *pstTime)
{
    static stTimeDef stTime = {0};

    Ds1339GetTime(&stTime);
    pstTime->ucYear = stTime.ucYear;
    pstTime->ucMonth = stTime.ucMonth;
    pstTime->ucDay = stTime.ucDate;
    pstTime->ucHour = stTime.ucHour;
    pstTime->ucMin = stTime.ucMin;
    return;
}

/*
����: ChckIntvl()
����:
    1.�ж��Ƿ񵽴��������ݲɼ�ʱ��
����:   void
����ֵ: 
    true:   ʱ�䵽����ִ�����ݲɼ�
    false:  ʱ��δ��������ִ�����ݲɼ�
����:   void
���:   void
��ע:   void
ע��:   void
*/
static bool  ChckIntvl(void)
{
    bool bRes = false;
    static uint32_t ulLstSec = 0;//�ϴ�ִ�����ݲɼ���ʱ���

    if ((ulBootSec - ulLstSec) < pstSenCfg->ulInvl)
    {//�ɼ�ʱ��δ��
        goto Return;
    }
    bRes = true;
    ulLstSec = ulBootSec;
Return:
    return bRes;
}

/*
����: CollData()
����:
    1.�ɼ�����������
����:   
    1.pstData: ָ��洢���������ݵĽṹ���ָ��
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void  CollData(void)
{
    void *pvTmp = (void *)sizeof(stTDef) + (void *)&stData;//ָ���һ���洢�ռ��ָ��
    uint8_t i = 0;
    
    GetTime(&(stData.stTime));//��ȡ�ɼ�ʱ��
    for (i = 0; i < SizeOfArray(aCollFunc); i++)
    {
        aCollFunc[i](pvTmp);
        pvTmp += (void *)sizeof(void *);
    }

    return;
}

/*
����: PwrInit()
����:
    1.��Դ�������ų�ʼ������
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void  PwrInit(void)
{
    LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;

    GpioSetDir(BRD_SEN_PWR_PORT, BRD_SEN_PWR_PIN, GPIO_DIRECTION_OUTPUT);
    return;
}

/*
����: PwrProc()
����:
    1.�ɼ���������Դ���Ʋ���
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void  PwrProc(void)
{
    static  enPwrModedef  enMode = ModeNull; //0:wake 1:sleep
    
    if (enMode != pstSenCfg->PwrMode)
    {
        enMode = pstSenCfg->PwrMode;
        switch (enMode)
        {
            case Wake:
            {
                GpioSetBit(BRD_SEN_PWR_PORT, BRD_SEN_PWR_PIN);
                break;
            }
            case Sleep:
            {
                GpioClrBit(BRD_SEN_PWR_PORT, BRD_SEN_PWR_PIN);
                break;
            }
            default :
            {
                break;
            }
        }
    }
    return;
}
/****************************************extern��������*********************************************/

/*
����: SenSorInit()
����:
    1.SenSor Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void  SenSorInit(void)
{
    TempInit();
    PwmInit();
    __485Init(&(stData.pst485Def));
    RainInit(&(stData.pstRainDef));
    PwrInit();
    return;
}

/*
����: SenSorPro()
����:
    1.���������ݲɼ�
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void  SenSorPro(void)
{
    //TempPro();
    PwrProc();//���ô�������ѹ����
    if (false == ChckIntvl())
    {//δ�����ݲɼ�ʱ�䣬��ִ�����ݲɼ�
        goto Return;
    }
    CollData();

Return:
    return;
}


/*
����: SenSorChck()
����:
    1.��⴫����״̬
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void  SenSorChck(void)
{
    
    return;
}

/*
����: SenSorGetDir()
����:
    1.ֱ��ִ�д��������ݲɼ�
����:   
    1.ucType:   �ɼ��Ĵ��������ͱ��
    2.pucData:  ָ�����ݻ����ָ��
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void SenSorGetDir(uint8_t ucType, uint8_t *pucData)
{
    if (ucType < SizeOfArray(aCollFunc))
    {
        aCollFunc[ucType](pucData);
    }
    return;
}

/*
����: GetSenData()
����:
    1. ��ȡ�������ɼ���������(�����ɼ�ʱ��)
����:   
    1.pucData:  ָ�����ݻ����ָ��
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void GetSenData(uint8_t *pucData)
{
    memcpy(pucData, &stData, sizeof(stData));
    return;
}

/*
����: SenDefault()
����:
    1. ��ʼ������������
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void SenDefault(void)
{
    _485Default();
    return;
}
