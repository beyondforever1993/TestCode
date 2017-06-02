#include "include.h"


/*
名称: GetChckSum()
功能:
    1.根据缓存中的数据及其长度获取单字节的ChackSum
参数:   
    pucData:    指向数据的指针
    usLen:      数据长度
返回值: ChackSum
输入:   void
输出:   void
备注:   void
注意:   void
*/
uint8_t GetChckSum(uint8_t *pucData, uint16_t usLen)
{
    uint8_t ucChckSum = 0;
    uint8_t i = 0;
    
    for(i = 0; i < usLen; i++) 
    {
        ucChckSum ^= pucData[i];
    }
    return ucChckSum;
}

/*
名称: Hex2BCD()
功能:
    1.根据缓存中的数据及其长度获取单字节的ChackSum
参数:   
    pucBuf:     指向数据缓存的指针
    ulData:     待转换的数据
    usLen:      转换后的数据位数
返回值: ChackSum
输入:   void
输出:   void
备注:   
    1.根据水文协议要求，该函数将转换后的数据按大端存储，并在位数不足时补零
注意:   void
*/
void Hex2BCD(uint8_t *pucBuf, uint32_t ulData, const uint8_t ucLen)
{
    uint8_t     i = 0;
    uint8_t     ucCnt = (ucLen / 2) - 1;//用于索引存储位置

    if (0 == ucLen)
    {
        goto Return;
    }
    memset(pucBuf, 0, ucLen / 2);
    for (i = 0; i < (ucLen / 2); i++)
    {
        pucBuf[ucCnt - i] = (((ulData % 100) / 10) << 4) | (ulData % 10);
        ulData /= 100;
    }
    
Return:
    return;
}
