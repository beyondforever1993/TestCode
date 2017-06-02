#include "include.h"


/*
����: GetChckSum()
����:
    1.���ݻ����е����ݼ��䳤�Ȼ�ȡ���ֽڵ�ChackSum
����:   
    pucData:    ָ�����ݵ�ָ��
    usLen:      ���ݳ���
����ֵ: ChackSum
����:   void
���:   void
��ע:   void
ע��:   void
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
����: Hex2BCD()
����:
    1.���ݻ����е����ݼ��䳤�Ȼ�ȡ���ֽڵ�ChackSum
����:   
    pucBuf:     ָ�����ݻ����ָ��
    ulData:     ��ת��������
    usLen:      ת���������λ��
����ֵ: ChackSum
����:   void
���:   void
��ע:   
    1.����ˮ��Э��Ҫ�󣬸ú�����ת��������ݰ���˴洢������λ������ʱ����
ע��:   void
*/
void Hex2BCD(uint8_t *pucBuf, uint32_t ulData, const uint8_t ucLen)
{
    uint8_t     i = 0;
    uint8_t     ucCnt = (ucLen / 2) - 1;//���������洢λ��

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
