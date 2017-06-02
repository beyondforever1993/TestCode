/*
文件名称: sensor.c
功能:
    1.包含sensor驱动相关的函数
作者: 杜在连
修改记录:
    2017-4-20 文件初创
备注:   void 
注意:   void
*/
#include "include.h"

#include "temp\temp.c"
#include "485\485.c"
#include "rain\rain.c"

/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/
#pragma pack(1)
typedef struct{//时间定义
uint8_t ucYear;//年
uint8_t ucMonth;//月
uint8_t ucDay;//日
uint8_t ucHour;//时
uint8_t ucMin;//分
}stTDef;
#pragma pack()

typedef void (* const pFunc)(void *);//传感器数据采集函数
typedef struct {//该结构体中的数据缓存必须与aCollFunc[]中的采集函数一一对应,且数据存储长度必须统一为 DATA_SZ
stTDef  stTime;//YYMMDDHHmm
st485Def *pst485Def;//485采集的水位数据
stRainDef *pstRainDef;//存储当前降水量
}stDataDef;
/*****************************************变量定义************************************************/
static pFunc aCollFunc[] = {//传感器数据采集函数定义 该数组中的元素必须与stDataDef中的数据缓存一一对应,
_485Proc,//485数据采集函数
}; 
static stDataDef stData = {0};
static stSenCfgDef  *const pstSenCfg = &(stRtuCfg.stSenCfg);

/******************************************函数定义***********************************************/

/****************************************static函数定义*********************************************/

/*
名称: GetTime()
功能:
    1.获取时间
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
名称: ChckIntvl()
功能:
    1.判断是否到传感器数据采集时间
参数:   void
返回值: 
    true:   时间到，该执行数据采集
    false:  时间未到，不能执行数据采集
输入:   void
输出:   void
备注:   void
注意:   void
*/
static bool  ChckIntvl(void)
{
    bool bRes = false;
    static uint32_t ulLstSec = 0;//上次执行数据采集的时间戳

    if ((ulBootSec - ulLstSec) < pstSenCfg->ulInvl)
    {//采集时间未到
        goto Return;
    }
    bRes = true;
    ulLstSec = ulBootSec;
Return:
    return bRes;
}

/*
名称: CollData()
功能:
    1.采集传感器数据
参数:   
    1.pstData: 指向存储传感器数据的结构体的指针
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void  CollData(void)
{
    void *pvTmp = (void *)sizeof(stTDef) + (void *)&stData;//指向第一个存储空间的指针
    uint8_t i = 0;
    
    GetTime(&(stData.stTime));//获取采集时间
    for (i = 0; i < SizeOfArray(aCollFunc); i++)
    {
        aCollFunc[i](pvTmp);
        pvTmp += (void *)sizeof(void *);
    }

    return;
}

/*
名称: PwrInit()
功能:
    1.电源控制引脚初始化操作
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void  PwrInit(void)
{
    LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;

    GpioSetDir(BRD_SEN_PWR_PORT, BRD_SEN_PWR_PIN, GPIO_DIRECTION_OUTPUT);
    return;
}

/*
名称: PwrProc()
功能:
    1.采集传感器电源控制操作
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
/****************************************extern函数定义*********************************************/

/*
名称: SenSorInit()
功能:
    1.SenSor Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
名称: SenSorPro()
功能:
    1.传感器数据采集
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void  SenSorPro(void)
{
    //TempPro();
    PwrProc();//设置传感器电压开关
    if (false == ChckIntvl())
    {//未到数据采集时间，不执行数据采集
        goto Return;
    }
    CollData();

Return:
    return;
}


/*
名称: SenSorChck()
功能:
    1.检测传感器状态
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void  SenSorChck(void)
{
    
    return;
}

/*
名称: SenSorGetDir()
功能:
    1.直接执行传感器数据采集
参数:   
    1.ucType:   采集的传感器类型编号
    2.pucData:  指向数据缓存的指针
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
名称: GetSenData()
功能:
    1. 获取传感器采集到的数据(包含采集时间)
参数:   
    1.pucData:  指向数据缓存的指针
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void GetSenData(uint8_t *pucData)
{
    memcpy(pucData, &stData, sizeof(stData));
    return;
}

/*
名称: SenDefault()
功能:
    1. 初始化传感器参数
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void SenDefault(void)
{
    _485Default();
    return;
}
