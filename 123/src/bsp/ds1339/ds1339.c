/*
�ļ�����: ds1339.c
����:
    1.����ds1339������صĺ���
����: ������
�޸ļ�¼:
    2017-4-15 �ļ�����
��ע:   void
ע��:   void
*/
#include "include.h"
#include "ds1339reg.h"

/******************************************�궨��*************************************************/
#define DS1339ADDR      0x68//I2c device addr

/*******************************************����**************************************************/

/*****************************************��������************************************************/
static stI2cDef I2Cx = {DS1339CH, DS1339DEV_ID};
#define pI2c        &I2Cx

/******************************************��������***********************************************/
/****************************************static��������*********************************************/
/*
����: Ds1339Write()
����:
    1.��Ds1339�ļĴ���д������
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void WriteReg(uint8_t ucAddr, uint8_t *pucData, uint8_t ucLen)
{
    uint8_t aucTmp[20] = {0};
    uint8_t i = 0;

    aucTmp[i++] = ucAddr;
    memcpy(&aucTmp[i], pucData, ucLen);
    I2cSend(pI2c, &aucTmp[0], ucLen + 1);
    return;
}

/*
����: Ds1339Read()
����:
    1.��Ds1339�ļĴ�����ȡ����
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void Ds1339Read(uint8_t ucAddr, uint8_t *pucData, uint8_t ucLen)
{
    uint8_t ucTmp = 0;

    WriteReg(ucAddr, &ucTmp, 0);//���üĴ�����ַ
    I2cRecv(pI2c, &pucData[0], ucLen);
    return;
}

/****************************************extern��������*********************************************/
/*
����: Ds1339SetTime()
����:
    1.��ʱ������д��DS1339
����:   
    1.stTime:   ָ��ʱ��ṹ���ָ��
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void Ds1339SetTime(stTimeDef *stTime)
{
    WriteReg(SecReg, (uint8_t *)stTime, sizeof(stTime));
    return;
}

/*
����: Ds1339GetTime()
����:
    1.��ʱ������д��DS1339
����:   
    1.stTime:   ָ��ʱ��ṹ���ָ��
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void Ds1339GetTime(stTimeDef *stTime)
{
    Ds1339Read(SecReg, (uint8_t *)stTime, sizeof(stTime));
    return;
}

/*
����: BSP_Init()
����:
    1.BSP Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú�������E2PROM_Default()���������Ƚϴ����еĲ�����E2PROM�еĲ�������һ�½����ǵ�E2PROM�еĲ���
*/
void Ds1339Init(void)
{
    uint8_t ucTmp = 0;
    
    I2cInit(pI2c, DS1339ADDR);
    DelayMs(100);
    WriteReg(CtrlReg, &ucTmp, 1);
    return;
}
#if I2C_DBG
static stTimeDef stTime = {0};

void Ds1339Dbg(void)
{
    OS_ERR err = OS_ERR_NONE;

#if 1
    stTime.ucDay    = 0x01;
    stTime.ucSec    = 0x00;
    stTime.ucMin    = 0x47;
    stTime.ucHour   = 0x16;
    stTime.ucDate   = 0x17;
    stTime.ucMonth  = 0x04;
    stTime.ucYear   = 0x16;
#endif
    Ds1339Init();
    printf("Cpu Reset\r\n");
    //Ds1339SetTime(&stTime);
    while(1)
    {
        Ds1339GetTime(&stTime);
        printf("NowTime:20%x-%x-%x Date:%x %x:%x:%x\r\n", stTime.ucYear, stTime.ucMonth, 
               stTime.ucDate, stTime.ucDay, stTime.ucHour, stTime.ucMin, stTime.ucSec);
        OSTimeDlyHMSM(0u, 0u, 0u, 1000u,                         /* Delay for 100ms.                                     */
                              OS_OPT_TIME_DLY | OS_OPT_TIME_HMSM_STRICT, 
                              &err);
    }
    return;
}
#endif
