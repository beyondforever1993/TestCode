#include "shuiwei.c"
#include "qingxie.c"
#include "yewei.c"

/******************************************宏定义*************************************************/
#define _485DATASZ          4

/*******************************************声明**************************************************/
#pragma pack(1)
typedef struct{//485数据定义
uint8_t aucData[_485DATASZ];
}st485Def;
#pragma pack()
/*****************************************变量定义************************************************/
st485CfgDef  *const pst485Cfg = &(stRtuCfg.stSenCfg.st485Cfg); 
st485Def     st485Buf;//485缓存定义

/******************************************函数定义***********************************************/

/****************************************static函数定义*********************************************/
/*
名称: _485RestProc()
功能:
    1.485复位流程
参数:   void
返回值: void
输入:   void
输出:   void
备注:   
    1.当波特率变化时执行复位流程
注意:   void
*/
static void _485RestProc(void)
{
    static uint32_t ulBaud = 0;//波特率

    if (ulBaud != pst485Cfg->baud)
    {
        ulBaud = pst485Cfg->baud;
        _485ChangeBaud(ulBaud);
    }
    return;
}

/*
名称: __485Send()
功能:
    1.根据传感器类型发送指定协议的数据
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void __485Send(void)
{
    switch (pst485Cfg->enType)
    {
        case SWJ:
        {
            SwSendCmd();
            break;
        }
        default:
        {
            break;
        }
    }
}

/*
名称: _485Deal()
功能:
    1.485数据解析
参数:   
    1.pucTmp:   接收自传感器的原始数据
    2.ucLen:    接收到的数据长度
    2.pucData:  指向解析完成的传感器数据缓存的指针
返回值: void
输入:   void
输出:   void
备注:   
    1.当波特率变化时执行复位流程
注意:   void
*/
static void _485Deal(uint8_t *pucTmp, uint8_t ucLen, uint8_t *pucData)
{
    switch (pst485Cfg->enType)
    {
        case SWJ:
        {
            SwDeal(pucTmp, ucLen, pucData);
            break;
        }
        default:
        {
            break;
        }
    }
}


/*
名称: _485Collect()
功能:
    1.采集485数据
参数:   
    1.pucData: 指向数据缓存的指针
返回值: void
输入:   void
输出:   void
备注:   
    1.当波特率变化时执行复位流程
注意:   void
*/
static void _485Collect(uint8_t *pucData)
{
    static uint8_t aucTmp[100] = {0};
    uint8_t ucLen = 0;

    __485Send();
    ucLen = _485Recv(&aucTmp[0], sizeof(aucTmp));//获取接收自传感器的数据
    if (ucLen)
    {//接收到传感器数据
        _485Deal(aucTmp, ucLen, pucData);
    }
    return;
}

/*
名称: __485Init()
功能:
    1.执行485相关硬件初始化
    2.初始化485数据缓存指针
参数:   
    1.ppucData: 指向数据缓存的指针
返回值: void
输入:   void
输出:   void
备注:   
    1.当波特率变化时执行复位流程
注意:   void
*/
static void __485Init(st485Def **ppstData)
{
    _485Init();//485硬件初始化
    ppstData = &st485Buf;
    return;
}

/****************************************extern函数定义*********************************************/

/*
名称: _485Proc()
功能:
    1.485传感器数据采集流程
参数:   
    1.pucData: 指向数据缓存的指针
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void _485Proc(void *pvData)
{
    _485RestProc();
    _485Collect((uint8_t *)pvData);
    return;
}

/*
名称: _485Default()
功能:
    1.485传感器参数 default
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void _485Default(void)
{
    pst485Cfg->baud   = 9600;
    pst485Cfg->enType = SWJ;
    pst485Cfg->para0  = 0;
    pst485Cfg->para1  = 0;
    return;
}

