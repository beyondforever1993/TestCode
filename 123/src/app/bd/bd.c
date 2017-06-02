/*
文件名称: BD.c
功能:
    1.包含北斗模块驱动相关的函数
作者: 杜在连
修改记录:
    2017-4-16 文件初创
备注:   void
注意:   void
*/
#include "include.h"

/******************************************宏定义*************************************************/
#define BD_PACKET_HEAD          '$'
#define BD_CMD_SZ               4

#define BD_LEN_H_POS            5//BD 4.0协议中的包长所在位置(高字节)
#define BD_LEN_L_POS            6//BD 4.0协议中的包长所在位置(低字节)

#define BD_BANDRATE             (_9600Bps)

#define BD_DATA_LEN_MAX         210

/*******************************************声明**************************************************/
typedef struct {
uint32_t ulIcNum;//IC 卡号
uint32_t ulDestIcNum;//IC 卡号
}stBdInfoDef;

/*****************************************变量定义************************************************/
stBdInfoDef stBdInfo = {0};

/******************************************函数定义***********************************************/
#define GetIcNum()                      (stBdInfo.ulIcNum)
#define GetDestIcNum()                  (stBdInfo.ulDestIcNum)
/****************************************static函数定义*********************************************/
/*
名称: SendToBd()
功能:
    1.向BD模块发送数据
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void SendToBd(uint8_t const *pucCmd, uint8_t const *pucData, uint16_t usLen)
{
    uint8_t *pucTmp = NULL;
    int i = 0;;
    
    pucTmp = (uint8_t *)MemGet(&MemSmall);
    if (NULL == pucTmp)
    {
        return;
    }
    pucTmp[i++] = BD_PACKET_HEAD;//$
    memcpy(&pucTmp[i], pucCmd, BD_CMD_SZ);//命令
    i += BD_CMD_SZ;

    i += 2;//暂时跳过包长存储位置

    pucTmp[i++] = (GetIcNum() >> 16) & 0xff;//IC卡号
    pucTmp[i++] = (GetIcNum() >> 8) & 0xff;
    pucTmp[i++] = (GetIcNum() >> 0) & 0xff;
    
    memcpy(&pucTmp[i], pucData, usLen);//数据包
    i += usLen;
    
    pucTmp[i++] = GetChckSum(pucTmp, i);    //校验
    
    pucTmp[BD_LEN_H_POS] = (i >> 8) & 0xff;//包长
    pucTmp[BD_LEN_L_POS] = (i >> 0) & 0xff;

    UartSend(UART_BD_CH, pucTmp,i);
    MemPut(&MemSmall, pucTmp);
    return;
}

/****************************************extern函数定义*********************************************/

/*
名称: BdInit()
功能:
    1.北斗模块 初始化
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void  BdInit(void)
{
    UartBspInit(UART_BD_CH, BD_BANDRATE);
    return;
}

/*
名称: BdRecvDeal()
功能:
    1.处理收到的BD模块数据
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void BdRecvDeal(uint8_t *pucRecv, uint16_t usLen)
{
    
    return;
}

/*
名称: BdChckSta()
功能:
    1.检查BD在线状态
参数:   void
返回值:
    
输入:   void
输出:   void
备注:   void
注意:   void
*/
enBdStaDef BdChckSta(void)
{
    enBdStaDef stSta = BdOnLine;

    if (0 == stBdInfo.ulIcNum)
    {
        stSta = BdOffLine;
    }
    return stSta;
}

/*
名称: BdAskIC()
功能:
    1.北斗模块 初始化
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void  BdAskIC(void)
{
    uint8_t ucTmp = 0;
    
    SendToBd(Str(ICJC), &ucTmp, 1);//发送请求IC指令
    return;
}

/*
名称: BdSend()
功能:
    1.通过BD模块发送数据
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数一次最多发送BD_DATA_LEN_MAX个字节
*/
void BdSend(uint8_t const *pucSend, uint16_t usSendLen)
{
    uint8_t *pucTmp = NULL;
    int i = 0;;

    if ((usSendLen > BD_DATA_LEN_MAX) || (BdOffLine == BdChckSta()))
    {//离线或者长度超范围，直接返回
        goto Return;
    }
    pucTmp = (uint8_t *)MemGet(&MemSmall);
    if (NULL == pucTmp)
    {
        goto Return;
    }
    pucTmp[i++] = 0x46; //通信信息类型 
    
    pucTmp[i++] = GetDestIcNum() >> 16; //目标用户地址
    pucTmp[i++] = GetDestIcNum() >> 8; //目标用户地址
    pucTmp[i++] = GetDestIcNum() >> 0; //目标用户地址

    pucTmp[i++] = ((usSendLen * 8) >> 8) & 0xff;//包长(高字节)
    pucTmp[i++] = ((usSendLen * 8) >> 0) & 0xff;//包长(低字节)

    pucTmp[i++] = 0;//是否需要应答
    memcpy(&pucTmp[i], pucSend, usSendLen);
    i += usSendLen;
    
    SendToBd((const uint8_t *)Str(TXSQ), pucTmp, i);//发送请求IC指令
    MemPut(&MemSmall, pucTmp);
Return:
    return;
}

/*
名称: BbAskTime()
功能:
    1.通过BD模块发送请求时间数据的指令
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void BbAskTime(void)
{
    uint8_t aucTmp[2] = {0};//该数组设置BD模块time 数据输出间隔,(以秒为单位，高字节在前)。此处为0表示单次获取，

    SendToBd(Str(SJSC), &aucTmp[0], 2);
    return;
}

#if BD_DBG
void BdTest(void)
{
    BbAskTime();
    BdSend("duzailian123456", strlen("duzailian123456"));
    BdAskIC();
    return;
}
#endif
