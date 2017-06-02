/*
文件名称: SL_T.c
功能:
    1.包含水文协议相关的函数
作者: 杜在连
修改记录:
    2017-5-19 文件初创
备注:   void
注意:   void
 */
#include "SL_T.h"



/******************************************宏定义*************************************************/
#define SL_INVL         40//心跳包发送间隔       

/*******************************************声明**************************************************/
#pragma pack(1)
typedef struct{//时间定义
uint8_t ucYear;//年
uint8_t ucMonth;//月
uint8_t ucDay;//日
uint8_t ucHour;//时
uint8_t ucMin;//分
uint8_t ucSec;//秒
}stTDef;
#pragma pack()

/*******************************************变量定义************************************************/
static uint32_t SL_Num = 0;

/******************************************函数定义*************************************************/
/****************************************static函数定义*********************************************/
#define SL_NUM_INC()      do{SL_Num++;}while(0)
#define GetBZF(len0, len1)  (((len0) << 3) | (len1))//len0:为扣除小数点后包含符号位的数据长度,len1:小数点后位数 

/*
名称: SL_GetTime()
功能:
    1.获取时间
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void  SL_GetTime(stTDef  *pstTime)
{
    static stTimeDef stTime = {0};

    Ds1339GetTime(&stTime);
    pstTime->ucYear = stTime.ucYear;
    pstTime->ucMonth = stTime.ucMonth;
    pstTime->ucDay = stTime.ucDate;
    pstTime->ucHour = stTime.ucHour;
    pstTime->ucMin = stTime.ucMin;
    pstTime->ucSec = stTime.ucSec;
    return;
}

/*
名称: SL_PackHead()
功能:
    1.打包水文协议正文头(包含流水号和实时时间)
参数:   
    1. usNum:   流水号
    2. pucData: 指向数据缓存的指针
返回值: 
    打包后的数据长度
输入:   void
输出:   void
备注:   void
注意:   void
*/
static uint8_t SL_PackHead(uint16_t usNum, uint8_t *pucData)
{
    uint8_t i = 0;
    
    pucData[i++] = usNum >> 8;
    pucData[i++] = usNum & 0xff;
    SL_GetTime((stTDef  *)&pucData[i]);
    i += sizeof(stTDef);
    
    return i;
}

/*
名称: SL_PackData()
功能:
    1.按水文协议打包数据
参数:   
    1. pucBuf:      指向打包后的数据缓存的指针
    2. ucFunc:      功能码
    3. pucData:     打包前的数据缓存指针
    4. usDataLen:   打包前的数据长度
返回值: 
    打包后的数据长度
输入:   void
输出:   void
备注:   void
注意:   void
*/
static uint16_t SL_PackData(uint8_t *pucBuf, uint8_t ucFunc, uint8_t *pucData, uint16_t usDataLen)
{
    uint16_t i = 0;
    
    if ((NULL == pucBuf) || (NULL == pucData) || (0 == usDataLen))
    {//缓存为NULL,直接返回
        goto  Return;
    }
    /*帧起始*/
    pucBuf[i++] = SOH >> 8;
    pucBuf[i++] = SOH & 0xff;
    
    {//复制中心站地址+遥测站地址+密码
        uint8_t ucTmp = sizeof(pstNetPar->CenAddr) + sizeof(pstNetPar->RTU_ID) + sizeof(pstNetPar->PassWd);
        memcpy(&pucBuf[i], &(pstNetPar->CenAddr), ucTmp);
        i += ucTmp;
    }
    
    pucBuf[i++] = ucFunc;//功能码
    pucBuf[i++] = 0x80 | ((usDataLen >> 8) & 0x0f);//上行标志 + 正文长度高4 bits
    pucBuf[i++] = usDataLen & 0xff;//正文长度低 8bits

    pucBuf[i++] = STX;//正文起始符
    memcpy(&pucBuf[i], pucData, usDataLen);//报文正文
    i += usDataLen;
    pucBuf[i++] = ETX;//正文结束符

    {//计算CRC
        uint16_t ulCrc = 0;
        
        LPC_CRC->SEED = 0xFFFF;
        ulCrc = CRC_CalcBlockChecksum(pucBuf, i, CRC_WR_8BIT);
        pucBuf[i++] = ulCrc >> 8;//CRC H
        pucBuf[i++] = ulCrc & 0xff;//CRC L
    }
Return:
    return i;
}

/*
名称: SL_Heart()
功能:
    1.按水文协议 发送心跳报文
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void SL_Heart(void)
{
    uint16_t usLen = 0;//组包后的数据长度
    static uint32_t ulLstSec = 0;//记录上次发送心跳报文的时间戳
    uint8_t *pucData = NULL;
    
    if ((ulBootSec - ulLstSec) < SL_INVL)
    {//心跳发送间隔未到
        goto Return;
    }
    ulLstSec = ulBootSec;
    pucData = MemGet(&MemSmall);
    if (NULL == pucData)
    {
        goto Return;
    }
    usLen = SL_PackHead(SL_Num, &pucData[200]);//添加流水号及时间
    usLen = SL_PackData(&pucData[0], LLWC, &pucData[200], usLen);//打包数据
    MemPut(&MemSmall, pucData);
Return:
    return;
}

/*
名称: SL_Deal()
功能:
    1.按水文协议处理接收到的数据
参数:   
    1. pucData:     打包前的数据缓存指针
    2. usDataLen:   打包前的数据长度
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void SL_Deal(uint8_t *pucData, uint16_t usDataLen)
{
    //uint8_t 
    return;
}

/*
名称: SL_GetRainNow()
功能:
    1.根据水文协议，获取当前降水量
参数:   
    1. pucData:     打包前的数据缓存指针
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void SL_GetRainNow(uint8_t *pucData)
{
    uint32_t uLTmp = 0;
    
    uLTmp = RainGetNow();
    
    return;
}

/****************************************extern函数定义*********************************************/
/*
名称: SL_PackTest()
功能:
    1.发送水文协议测试报
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void SL_PackTest(void)
{
    uint8_t i = 0;
    uint8_t *pucData = NULL;

    pucData = MemGet(&MemSmall);
    if (NULL == pucData)
    {
        goto Return;
    }
    SL_Num++;
    i += SL_PackHead(SL_Num, &pucData[150]);//存放流水号和发报时间
    memcpy(&pucData[150 + i], &(pstNetPar->RTU_ID[0]), sizeof(pstNetPar->RTU_ID));//遥测站地址
    i += sizeof(pstNetPar->RTU_ID);
    pucData[150 + i++] = pstNetPar->TypeCode;//遥测站分类码
    
    pucData[150 + i++] = TT;//观测时间标识符引导符
    pucData[150 + i++] = GetBZF(5, 0);//观测时间标识符低半字节
    SL_GetTime((stTDef  *)&pucData[150 + i]);
    i += 5;

    pucData[150 + i++] = PJ;//当前降水量标识符引导符
    pucData[150 + i++] = GetBZF(5, 1);//观测时间标识符低半字节
    SL_GetRainNow();

    MemPut(&MemSmall, pucData);
Return:
    return;
}

