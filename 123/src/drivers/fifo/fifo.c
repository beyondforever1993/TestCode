/*
文件名称: fifo.c
功能:
    1.fifo 相关的操作函数
作者: 杜在连
修改记录:
    2017-3-30 文件初创
备注:   void
注意:
    1.当前文件中的FIFO函数目前只支持整包数据的读(写)操作(单包数据不大于0xff字节)
    2.FIFO中每包存储均以包体长度开头
*/

#include "include.h"

/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/

/*****************************************变量定义************************************************/

/******************************************函数定义***********************************************/

/****************************************static函数定义*********************************************/
#define IncPoint(piont, sz)         do{piont = (++piont) % sz;}while(0)//环形指针+1
#define DecPoint(piont, sz)         if(0 == piont)\
                                    {\
                                        piont = sz - 1;\
                                    }\
                                    else\
                                    {\
                                        piont--;\
                                    }
#if 0

                                                                        /*
名称: FifoSearch()
功能:
    1.判断FIFO中是否为空
参数:   
    1.pstFIFO: 指向待写入FIFO的指针
返回值: 
    true:   FIFO为空
    false:  FIFO非空
输入:   void
输出:   void
备注:   void
注意:   void
*/
bool FifoIsE(const stFiFoDef *const pstFIFO)
{
    return (pstFIFO->usWpoint == pstFIFO->usRpoint);
}

/*
名称: SearchByte()
功能:
    1.在FIFO缓存中搜索指定Byte数据
    2.若搜索到则将pstFIFO中的读指针指向搜索到的字符
参数:   
    1.pstFIFO:  指向待写入FIFO的指针
    2.ucData:   待搜索的字符
    3.pucPoint: 用于存储搜索到的字符在FIFO中的位置
返回值: 
    0:      OK
    Others: Error Step
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数未操作FIFO读指针
*/
static uint8_t SearchByte(const stFiFoDef *const pstFIFO, uint8_t ucData, uint16_t *pusPoint)
{
    uint8_t ucRes = 0;
    uint16_t usRpoint = pstFIFO->usRpoint;  //读指针，存储可读数据在pucBuf中的起始位置(读取操作在串口数据读取函数中进行)
    uint16_t usBufSz  = pstFIFO->usBufSz;  //缓存 size
    uint8_t  const *const pucBuff = pstFIFO->pucBuf;//指向buffer的指针

Start:
    if (FifoIsE(pstFIFO))
    {//FIFO为空
        ucRes = 0x01;
        goto Return;
    }
    if (ucData == pucBuff[usRpoint])
    {
        *pusPoint = usRpoint;
        ucRes = 0x00;
        goto Return;
    }
    IncPoint(usRpoint, usBufSz);
    goto Start;
Return:
    return ucRes;
}

/*
名称: SearchHead()
功能:
    1.在FIFO中搜索指定数据
参数:   
    1.pstFIFO: 指向待写入FIFO的指针
    2.pucData: 读取数据用的缓存
返回值: 
    0:      OK
    Others: Error Step
输入:   void
输出:   void
备注:   void
注意:   void
*/
static uint8_t SearchData(stFiFoDef *const pstFIFO, uint8_t *pcData, uint16_t *pusPoint)
{
    uint8_t ucRes = 0;
    uint8_t i = 0;

    for(i = 0; i < 2; i++)
    {//搜索包头
        if(SearchByte(pstFIFO, pcData[i], pusPoint))
        {
            ucRes = 0x01;
            DecPoint(*pusPoint, pstFIFO->usBufSz);//回退到FIFO中搜索到的数据位置(如包头的第一个Byte)
            goto Return;
        }
        if (0 == pcData[i + 1])
        {//包头/包尾只有1Byte
            goto Return;
        }
    }
Return:
    return ucRes;
}

/*
名称: GetData()
功能:
    1.从FIFO中获取整包数据
参数:   
    1.pstFIFO:  指向待写入FIFO的指针
    2.pucData:  数据缓存指针
    3.pucLen:   指向存储包长的变量的指针
    4.pucTail:  指向包尾的指针
返回值: 
    0:      OK
    Others: Error Step
输入:   void
输出:   void
备注:   void
注意:   void
*/
static uint8_t GetPack(uint16_t uspHead, uint16_t uspTail, uint8_t *const pucData, uint16_t *pusLen, uint8_t  *pucBuff)
{
    uint16_t usRpoint = pstFIFO->usRpoint;  //读指针，存储可读数据在pucBuf中的起始位置(读取操作在串口数据读取函数中进行)
    uint8_t  *const pucBuff = pstFIFO->pucBuf;//指向buffer的指针

    while(pucBuff[usRpoint] != pucTail[1])
    {
        
    }
    return;
}
#endif

/****************************************extern函数定义*********************************************/


/*
名称: FifoWrite()
功能:
    1.向FIFO中写入usLen个字节的数据
参数:   
    1.pstFIFO: 指向待写入FIFO的指针
    2.ucData:  待写入数据
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void FifoWrite(stFiFoDef *const pstFIFO, const uint8_t ucData)
{
#if FIFO_DBG
    uint16_t usRpoint = pstFIFO->usRpoint;  //读指针，存储可读数据在pucBuf中的起始位置(读取操作在串口数据读取函数中进行)
#endif
    uint16_t usWpoint = pstFIFO->usWpoint;  //写指针,存储写入的数据在pucBuf中的结束位置(写入操作在中断中执行)
    uint16_t usBufSz  = pstFIFO->usBufSz;  //缓存 size
    uint8_t  *const pucBuff = pstFIFO->pucBuf;//指向buffer的指针

    pucBuff[usWpoint++] =  ucData;
#if FIFO_DBG
    if(usRpoint == usWpoint)
    {
        printf("FiFo Full!!\r\n");
    }
#endif
    pstFIFO->usWpoint = usWpoint % usBufSz;
    return;
}

#if 0
/*
名称: FifoRead()
功能:
    1.从FIFO中获取整包数据
参数:   
    1.pstFIFO:  指向待写入FIFO的指针
    2.pucData:  读取数据用的缓存
    3.pucLen:   指向存储包长的变量的指针
    4.pstPack:  指向待搜索的包头/包尾的指针
返回值: 
    0:      OK
    Others: Error Step
输入:   void
输出:   void
备注:   void
注意:   void
*/
uint8_t FifoRead(stFiFoDef *const pstFIFO, uint8_t *const pucData, uint16_t *pusLen, stPackDef *pstPack)
{
    uint8_t  ucRes = 0;
    uint16_t uspHead = 0;
    uint16_t uspTail = 0;
    uint8_t  *const pucBuff = pstFIFO->pucBuf;
    uint16_t usNew = 0;//读操作完成后的读指针(未处理)
    uint16_t usRpoint = pstFIFO->usRpoint;  //读指针，存储可读数据在pucBuf中的起始位置(读取操作在串口数据读取函数中进行)
    uint16_t usBufSz  = pstFIFO->usBufSz;  //缓存 size

    if (SearchData(pstFIFO, pstPack->pcHead, &uspHead))
    {//未找到头 
        ucRes = 0x01;
        goto Return;
    }
    if (SearchData(pstFIFO, pstPack->pcTail, &uspTail))
    {//未找到尾
        ucRes = 0x02;
        goto Return;
    }
    GetPack(uspHead, uspTail, pucData, pusLen, pucBuff);
    pstFIFO->usRpoint = IncPoint(uspTail);//将读指针设为读到的包尾 + 1
Return:
    return ucRes;
}
#endif
