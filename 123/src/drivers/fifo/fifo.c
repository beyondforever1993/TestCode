/*
�ļ�����: fifo.c
����:
    1.fifo ��صĲ�������
����: ������
�޸ļ�¼:
    2017-3-30 �ļ�����
��ע:   void
ע��:
    1.��ǰ�ļ��е�FIFO����Ŀǰֻ֧���������ݵĶ�(д)����(�������ݲ�����0xff�ֽ�)
    2.FIFO��ÿ���洢���԰��峤�ȿ�ͷ
*/

#include "include.h"

/******************************************�궨��*************************************************/

/*******************************************����**************************************************/

/*****************************************��������************************************************/

/******************************************��������***********************************************/

/****************************************static��������*********************************************/
#define IncPoint(piont, sz)         do{piont = (++piont) % sz;}while(0)//����ָ��+1
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
����: FifoSearch()
����:
    1.�ж�FIFO���Ƿ�Ϊ��
����:   
    1.pstFIFO: ָ���д��FIFO��ָ��
����ֵ: 
    true:   FIFOΪ��
    false:  FIFO�ǿ�
����:   void
���:   void
��ע:   void
ע��:   void
*/
bool FifoIsE(const stFiFoDef *const pstFIFO)
{
    return (pstFIFO->usWpoint == pstFIFO->usRpoint);
}

/*
����: SearchByte()
����:
    1.��FIFO����������ָ��Byte����
    2.����������pstFIFO�еĶ�ָ��ָ�����������ַ�
����:   
    1.pstFIFO:  ָ���д��FIFO��ָ��
    2.ucData:   ���������ַ�
    3.pucPoint: ���ڴ洢���������ַ���FIFO�е�λ��
����ֵ: 
    0:      OK
    Others: Error Step
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú���δ����FIFO��ָ��
*/
static uint8_t SearchByte(const stFiFoDef *const pstFIFO, uint8_t ucData, uint16_t *pusPoint)
{
    uint8_t ucRes = 0;
    uint16_t usRpoint = pstFIFO->usRpoint;  //��ָ�룬�洢�ɶ�������pucBuf�е���ʼλ��(��ȡ�����ڴ������ݶ�ȡ�����н���)
    uint16_t usBufSz  = pstFIFO->usBufSz;  //���� size
    uint8_t  const *const pucBuff = pstFIFO->pucBuf;//ָ��buffer��ָ��

Start:
    if (FifoIsE(pstFIFO))
    {//FIFOΪ��
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
����: SearchHead()
����:
    1.��FIFO������ָ������
����:   
    1.pstFIFO: ָ���д��FIFO��ָ��
    2.pucData: ��ȡ�����õĻ���
����ֵ: 
    0:      OK
    Others: Error Step
����:   void
���:   void
��ע:   void
ע��:   void
*/
static uint8_t SearchData(stFiFoDef *const pstFIFO, uint8_t *pcData, uint16_t *pusPoint)
{
    uint8_t ucRes = 0;
    uint8_t i = 0;

    for(i = 0; i < 2; i++)
    {//������ͷ
        if(SearchByte(pstFIFO, pcData[i], pusPoint))
        {
            ucRes = 0x01;
            DecPoint(*pusPoint, pstFIFO->usBufSz);//���˵�FIFO��������������λ��(���ͷ�ĵ�һ��Byte)
            goto Return;
        }
        if (0 == pcData[i + 1])
        {//��ͷ/��βֻ��1Byte
            goto Return;
        }
    }
Return:
    return ucRes;
}

/*
����: GetData()
����:
    1.��FIFO�л�ȡ��������
����:   
    1.pstFIFO:  ָ���д��FIFO��ָ��
    2.pucData:  ���ݻ���ָ��
    3.pucLen:   ָ��洢�����ı�����ָ��
    4.pucTail:  ָ���β��ָ��
����ֵ: 
    0:      OK
    Others: Error Step
����:   void
���:   void
��ע:   void
ע��:   void
*/
static uint8_t GetPack(uint16_t uspHead, uint16_t uspTail, uint8_t *const pucData, uint16_t *pusLen, uint8_t  *pucBuff)
{
    uint16_t usRpoint = pstFIFO->usRpoint;  //��ָ�룬�洢�ɶ�������pucBuf�е���ʼλ��(��ȡ�����ڴ������ݶ�ȡ�����н���)
    uint8_t  *const pucBuff = pstFIFO->pucBuf;//ָ��buffer��ָ��

    while(pucBuff[usRpoint] != pucTail[1])
    {
        
    }
    return;
}
#endif

/****************************************extern��������*********************************************/


/*
����: FifoWrite()
����:
    1.��FIFO��д��usLen���ֽڵ�����
����:   
    1.pstFIFO: ָ���д��FIFO��ָ��
    2.ucData:  ��д������
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void FifoWrite(stFiFoDef *const pstFIFO, const uint8_t ucData)
{
#if FIFO_DBG
    uint16_t usRpoint = pstFIFO->usRpoint;  //��ָ�룬�洢�ɶ�������pucBuf�е���ʼλ��(��ȡ�����ڴ������ݶ�ȡ�����н���)
#endif
    uint16_t usWpoint = pstFIFO->usWpoint;  //дָ��,�洢д���������pucBuf�еĽ���λ��(д��������ж���ִ��)
    uint16_t usBufSz  = pstFIFO->usBufSz;  //���� size
    uint8_t  *const pucBuff = pstFIFO->pucBuf;//ָ��buffer��ָ��

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
����: FifoRead()
����:
    1.��FIFO�л�ȡ��������
����:   
    1.pstFIFO:  ָ���д��FIFO��ָ��
    2.pucData:  ��ȡ�����õĻ���
    3.pucLen:   ָ��洢�����ı�����ָ��
    4.pstPack:  ָ��������İ�ͷ/��β��ָ��
����ֵ: 
    0:      OK
    Others: Error Step
����:   void
���:   void
��ע:   void
ע��:   void
*/
uint8_t FifoRead(stFiFoDef *const pstFIFO, uint8_t *const pucData, uint16_t *pusLen, stPackDef *pstPack)
{
    uint8_t  ucRes = 0;
    uint16_t uspHead = 0;
    uint16_t uspTail = 0;
    uint8_t  *const pucBuff = pstFIFO->pucBuf;
    uint16_t usNew = 0;//��������ɺ�Ķ�ָ��(δ����)
    uint16_t usRpoint = pstFIFO->usRpoint;  //��ָ�룬�洢�ɶ�������pucBuf�е���ʼλ��(��ȡ�����ڴ������ݶ�ȡ�����н���)
    uint16_t usBufSz  = pstFIFO->usBufSz;  //���� size

    if (SearchData(pstFIFO, pstPack->pcHead, &uspHead))
    {//δ�ҵ�ͷ 
        ucRes = 0x01;
        goto Return;
    }
    if (SearchData(pstFIFO, pstPack->pcTail, &uspTail))
    {//δ�ҵ�β
        ucRes = 0x02;
        goto Return;
    }
    GetPack(uspHead, uspTail, pucData, pusLen, pucBuff);
    pstFIFO->usRpoint = IncPoint(uspTail);//����ָ����Ϊ�����İ�β + 1
Return:
    return ucRes;
}
#endif
