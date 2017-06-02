/*
�ļ�����: BD.c
����:
    1.��������ģ��������صĺ���
����: ������
�޸ļ�¼:
    2017-4-16 �ļ�����
��ע:   void
ע��:   void
*/
#include "include.h"

/******************************************�궨��*************************************************/
#define BD_PACKET_HEAD          '$'
#define BD_CMD_SZ               4

#define BD_LEN_H_POS            5//BD 4.0Э���еİ�������λ��(���ֽ�)
#define BD_LEN_L_POS            6//BD 4.0Э���еİ�������λ��(���ֽ�)

#define BD_BANDRATE             (_9600Bps)

#define BD_DATA_LEN_MAX         210

/*******************************************����**************************************************/
typedef struct {
uint32_t ulIcNum;//IC ����
uint32_t ulDestIcNum;//IC ����
}stBdInfoDef;

/*****************************************��������************************************************/
stBdInfoDef stBdInfo = {0};

/******************************************��������***********************************************/
#define GetIcNum()                      (stBdInfo.ulIcNum)
#define GetDestIcNum()                  (stBdInfo.ulDestIcNum)
/****************************************static��������*********************************************/
/*
����: SendToBd()
����:
    1.��BDģ�鷢������
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
    memcpy(&pucTmp[i], pucCmd, BD_CMD_SZ);//����
    i += BD_CMD_SZ;

    i += 2;//��ʱ���������洢λ��

    pucTmp[i++] = (GetIcNum() >> 16) & 0xff;//IC����
    pucTmp[i++] = (GetIcNum() >> 8) & 0xff;
    pucTmp[i++] = (GetIcNum() >> 0) & 0xff;
    
    memcpy(&pucTmp[i], pucData, usLen);//���ݰ�
    i += usLen;
    
    pucTmp[i++] = GetChckSum(pucTmp, i);    //У��
    
    pucTmp[BD_LEN_H_POS] = (i >> 8) & 0xff;//����
    pucTmp[BD_LEN_L_POS] = (i >> 0) & 0xff;

    UartSend(UART_BD_CH, pucTmp,i);
    MemPut(&MemSmall, pucTmp);
    return;
}

/****************************************extern��������*********************************************/

/*
����: BdInit()
����:
    1.����ģ�� ��ʼ��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void  BdInit(void)
{
    UartBspInit(UART_BD_CH, BD_BANDRATE);
    return;
}

/*
����: BdRecvDeal()
����:
    1.�����յ���BDģ������
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void BdRecvDeal(uint8_t *pucRecv, uint16_t usLen)
{
    
    return;
}

/*
����: BdChckSta()
����:
    1.���BD����״̬
����:   void
����ֵ:
    
����:   void
���:   void
��ע:   void
ע��:   void
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
����: BdAskIC()
����:
    1.����ģ�� ��ʼ��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void  BdAskIC(void)
{
    uint8_t ucTmp = 0;
    
    SendToBd(Str(ICJC), &ucTmp, 1);//��������ICָ��
    return;
}

/*
����: BdSend()
����:
    1.ͨ��BDģ�鷢������
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú���һ����෢��BD_DATA_LEN_MAX���ֽ�
*/
void BdSend(uint8_t const *pucSend, uint16_t usSendLen)
{
    uint8_t *pucTmp = NULL;
    int i = 0;;

    if ((usSendLen > BD_DATA_LEN_MAX) || (BdOffLine == BdChckSta()))
    {//���߻��߳��ȳ���Χ��ֱ�ӷ���
        goto Return;
    }
    pucTmp = (uint8_t *)MemGet(&MemSmall);
    if (NULL == pucTmp)
    {
        goto Return;
    }
    pucTmp[i++] = 0x46; //ͨ����Ϣ���� 
    
    pucTmp[i++] = GetDestIcNum() >> 16; //Ŀ���û���ַ
    pucTmp[i++] = GetDestIcNum() >> 8; //Ŀ���û���ַ
    pucTmp[i++] = GetDestIcNum() >> 0; //Ŀ���û���ַ

    pucTmp[i++] = ((usSendLen * 8) >> 8) & 0xff;//����(���ֽ�)
    pucTmp[i++] = ((usSendLen * 8) >> 0) & 0xff;//����(���ֽ�)

    pucTmp[i++] = 0;//�Ƿ���ҪӦ��
    memcpy(&pucTmp[i], pucSend, usSendLen);
    i += usSendLen;
    
    SendToBd((const uint8_t *)Str(TXSQ), pucTmp, i);//��������ICָ��
    MemPut(&MemSmall, pucTmp);
Return:
    return;
}

/*
����: BbAskTime()
����:
    1.ͨ��BDģ�鷢������ʱ�����ݵ�ָ��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void BbAskTime(void)
{
    uint8_t aucTmp[2] = {0};//����������BDģ��time ����������,(����Ϊ��λ�����ֽ���ǰ)���˴�Ϊ0��ʾ���λ�ȡ��

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
