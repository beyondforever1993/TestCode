/*
�ļ�����: SL_T.c
����:
    1.����ˮ��Э����صĺ���
����: ������
�޸ļ�¼:
    2017-5-19 �ļ�����
��ע:   void
ע��:   void
 */
#include "SL_T.h"



/******************************************�궨��*************************************************/
#define SL_INVL         40//���������ͼ��       

/*******************************************����**************************************************/
#pragma pack(1)
typedef struct{//ʱ�䶨��
uint8_t ucYear;//��
uint8_t ucMonth;//��
uint8_t ucDay;//��
uint8_t ucHour;//ʱ
uint8_t ucMin;//��
uint8_t ucSec;//��
}stTDef;
#pragma pack()

/*******************************************��������************************************************/
static uint32_t SL_Num = 0;

/******************************************��������*************************************************/
/****************************************static��������*********************************************/
#define SL_NUM_INC()      do{SL_Num++;}while(0)
#define GetBZF(len0, len1)  (((len0) << 3) | (len1))//len0:Ϊ�۳�С������������λ�����ݳ���,len1:С�����λ�� 

/*
����: SL_GetTime()
����:
    1.��ȡʱ��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: SL_PackHead()
����:
    1.���ˮ��Э������ͷ(������ˮ�ź�ʵʱʱ��)
����:   
    1. usNum:   ��ˮ��
    2. pucData: ָ�����ݻ����ָ��
����ֵ: 
    ���������ݳ���
����:   void
���:   void
��ע:   void
ע��:   void
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
����: SL_PackData()
����:
    1.��ˮ��Э��������
����:   
    1. pucBuf:      ָ����������ݻ����ָ��
    2. ucFunc:      ������
    3. pucData:     ���ǰ�����ݻ���ָ��
    4. usDataLen:   ���ǰ�����ݳ���
����ֵ: 
    ���������ݳ���
����:   void
���:   void
��ע:   void
ע��:   void
*/
static uint16_t SL_PackData(uint8_t *pucBuf, uint8_t ucFunc, uint8_t *pucData, uint16_t usDataLen)
{
    uint16_t i = 0;
    
    if ((NULL == pucBuf) || (NULL == pucData) || (0 == usDataLen))
    {//����ΪNULL,ֱ�ӷ���
        goto  Return;
    }
    /*֡��ʼ*/
    pucBuf[i++] = SOH >> 8;
    pucBuf[i++] = SOH & 0xff;
    
    {//��������վ��ַ+ң��վ��ַ+����
        uint8_t ucTmp = sizeof(pstNetPar->CenAddr) + sizeof(pstNetPar->RTU_ID) + sizeof(pstNetPar->PassWd);
        memcpy(&pucBuf[i], &(pstNetPar->CenAddr), ucTmp);
        i += ucTmp;
    }
    
    pucBuf[i++] = ucFunc;//������
    pucBuf[i++] = 0x80 | ((usDataLen >> 8) & 0x0f);//���б�־ + ���ĳ��ȸ�4 bits
    pucBuf[i++] = usDataLen & 0xff;//���ĳ��ȵ� 8bits

    pucBuf[i++] = STX;//������ʼ��
    memcpy(&pucBuf[i], pucData, usDataLen);//��������
    i += usDataLen;
    pucBuf[i++] = ETX;//���Ľ�����

    {//����CRC
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
����: SL_Heart()
����:
    1.��ˮ��Э�� ������������
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void SL_Heart(void)
{
    uint16_t usLen = 0;//���������ݳ���
    static uint32_t ulLstSec = 0;//��¼�ϴη����������ĵ�ʱ���
    uint8_t *pucData = NULL;
    
    if ((ulBootSec - ulLstSec) < SL_INVL)
    {//�������ͼ��δ��
        goto Return;
    }
    ulLstSec = ulBootSec;
    pucData = MemGet(&MemSmall);
    if (NULL == pucData)
    {
        goto Return;
    }
    usLen = SL_PackHead(SL_Num, &pucData[200]);//�����ˮ�ż�ʱ��
    usLen = SL_PackData(&pucData[0], LLWC, &pucData[200], usLen);//�������
    MemPut(&MemSmall, pucData);
Return:
    return;
}

/*
����: SL_Deal()
����:
    1.��ˮ��Э�鴦����յ�������
����:   
    1. pucData:     ���ǰ�����ݻ���ָ��
    2. usDataLen:   ���ǰ�����ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void SL_Deal(uint8_t *pucData, uint16_t usDataLen)
{
    //uint8_t 
    return;
}

/*
����: SL_GetRainNow()
����:
    1.����ˮ��Э�飬��ȡ��ǰ��ˮ��
����:   
    1. pucData:     ���ǰ�����ݻ���ָ��
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void SL_GetRainNow(uint8_t *pucData)
{
    uint32_t uLTmp = 0;
    
    uLTmp = RainGetNow();
    
    return;
}

/****************************************extern��������*********************************************/
/*
����: SL_PackTest()
����:
    1.����ˮ��Э����Ա�
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
    i += SL_PackHead(SL_Num, &pucData[150]);//�����ˮ�źͷ���ʱ��
    memcpy(&pucData[150 + i], &(pstNetPar->RTU_ID[0]), sizeof(pstNetPar->RTU_ID));//ң��վ��ַ
    i += sizeof(pstNetPar->RTU_ID);
    pucData[150 + i++] = pstNetPar->TypeCode;//ң��վ������
    
    pucData[150 + i++] = TT;//�۲�ʱ���ʶ��������
    pucData[150 + i++] = GetBZF(5, 0);//�۲�ʱ���ʶ���Ͱ��ֽ�
    SL_GetTime((stTDef  *)&pucData[150 + i]);
    i += 5;

    pucData[150 + i++] = PJ;//��ǰ��ˮ����ʶ��������
    pucData[150 + i++] = GetBZF(5, 1);//�۲�ʱ���ʶ���Ͱ��ֽ�
    SL_GetRainNow();

    MemPut(&MemSmall, pucData);
Return:
    return;
}

