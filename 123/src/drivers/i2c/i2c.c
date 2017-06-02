/*
�ļ�����: i2c.c
����:
    1.����i2c������صĺ���
����: ������
�޸ļ�¼:
    2017-4-14 �ļ�����
��ע:
    1.������������˿�������ֻ����astI2cInfo[]�����Ӧλ�������������ò�������,
ע��:
    1.�ļ�����ʱ��ֻ������I2c1��ʹ�ã�����ʹ�������˿ڣ�������LPC1778��User Manual�Ƿ���I2c1���������ϵĲ���
    2.���ļ��е�������ʹ��LPC1778 I2C��Master mode,��������Ҫ����ģʽ�����Ը�����Ҫ���
*/
#include "include.h"



/******************************************�궨��*************************************************/
#define I2C_DEV_MAX         I2cDevMax//����channel֧�ֵ����device����

#define RETRY_CNT           3//���Դ���

#define ClrSi()             do{I2Cx->CONCLR = I2C_I2CONCLR_SIC;}while(0)
/*******************************************����**************************************************/
typedef const struct{
stPinCfgDef  Scl;
stPinCfgDef  Sda;
}stI2cGpioDef;

typedef enum{
Write,
Read,
}enRwDef;

typedef const struct{
LPC_I2C_TypeDef *I2Cx; //ָ��I2C�Ĵ�����ָ��(LPC_I2C0 etc.)
uint32_t ulPwrEn;   //PCONP �Ĵ����е�ʹ��λ(CLKPWR_PCONP_PCI2C1 etc.)
uint32_t ulFreq;    //SCLƵ��(HZ)
stI2cGpioDef stGpio;//scl/sda ���ŵ�������Ϣ
}stI2cInfoDef;

/*****************************************��������************************************************/
stI2cInfoDef astI2cInfo[] = {
{0},//I2C 0
{/*I2C 1*/
    LPC_I2C1,
    /*power ctrl*/              
    CLKPWR_PCONP_PCI2C1,  
    /*clk freqence*/
    350000,//350kHZ
    /*Pin Cfg*/
    {   /*SCL*/   
        {I2C1_SCL_PORT, I2C1_SCL_PIN, (3 | IOCON_MODE_PLAIN | IOCON_OPENDRAIN_MODE | IOCON_SLEW_ENABLE)},     
        /*SDA*/   
        {I2C1_SDA_PORT, I2C1_SDA_PIN, (3 | IOCON_MODE_PLAIN | IOCON_OPENDRAIN_MODE | IOCON_SLEW_ENABLE)}
    },       
},
};

static uint8_t aucAddr[I2C_CH_MAX][I2C_DEV_MAX] = {0};//�洢Salve Addr

/******************************************��������***********************************************/
/*
����: SetClock()
����:
    1.����ʱ��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void SetClock (LPC_I2C_TypeDef *I2Cx, uint32_t target_clock)
{
    uint32_t temp;

    temp = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER) / target_clock / 2;

    /* Set the I2C clock value to register */
    I2Cx->SCLH = temp;

    I2Cx->SCLL = temp;
}

/*
����: I2cInit()
����:
    1.I2c Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void I2cInit(stI2cDef *I2cId, uint8_t const ucSlaveAddr)
{
    stI2cInfoDef *pstI2cInfo = &astI2cInfo[I2cId->I2cCh];
        
    en_I2C_unitId I2cCh  = I2cId->I2cCh;
    enI2cDevDef   I2cDev = I2cId->I2cDev;
    
    {/*GPIO Init*/
        LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;
        GpioPinCfg(pstI2cInfo->stGpio.Scl);
        GpioPinCfg(pstI2cInfo->stGpio.Sda);
    }
    {/*I2c Init*/
        LPC_I2C_TypeDef *I2Cx = pstI2cInfo->I2Cx;
        
        LPC_SC->PCONP |= pstI2cInfo->ulPwrEn;//I2c power on
        I2Cx->CONCLR = (I2C_I2CONCLR_AAC | I2C_I2CONCLR_STAC | I2C_I2CONCLR_I2ENC | I2C_I2CONCLR_SIC);//����ƼĴ���
        I2Cx->CONSET = I2C_I2CONSET_I2EN;
        SetClock(I2Cx, pstI2cInfo->ulFreq);
    }
    aucAddr[I2cCh][I2cDev] = ucSlaveAddr;
    return;
}

/*
����: GetSta()
����:
    1.�ȴ�SI��λ
    2.����STAT�Ĵ���״̬
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static uint8_t GetSta(LPC_I2C_TypeDef *I2Cx)
{
    while(0 == (I2C_I2CONSET_SI & I2Cx->CONSET));//�ȴ�SI��λ

    return (I2Cx->STAT) & I2C_STAT_CODE_BITMASK;
}

/*
����: I2cStart()
����:
    1.����enRw����I2c��ʼ�źż���ʼ�ֽ�
����:   
    1. I2Cx: ָ��I2c�Ĵ�����ָ��
    2. enRw: read/write
����ֵ: 
    0:      OK
    ohers   error
����:   void
���:   void
��ע:   void I2C_I2STAT_M_TX_START
ע��:   void
*/
uint8_t I2cStart(LPC_I2C_TypeDef *I2Cx, uint8_t ucAddr, enRwDef enRw)
{
    uint8_t ucCnt = RETRY_CNT;
    uint8_t ucRes = 0;
    uint8_t ucTmp = 0;
    
    while(ucCnt)
    {
        I2Cx->CONSET |= I2C_I2CONSET_STA;//������ʼ����
        GetSta(I2Cx);//�ȴ���ʼ�����������
        I2Cx->CONCLR = I2C_I2CONSET_STA;//ֹͣ���� I2c��ʼ����
        I2Cx->DAT = (ucAddr << 1) | enRw;
        ClrSi();
        ucTmp = GetSta(I2Cx);
        if ((I2C_I2STAT_M_RX_SLAR_ACK == ucTmp) || (I2C_I2STAT_M_TX_SLAW_ACK == ucTmp))
        {//���յ�Ӧ��
            ucRes = 0;
            goto Return;
        }
        ucCnt--;
    }
    ucRes = ucTmp;
Return:    
    if (Read == enRw)
    {
        I2Cx->CONSET |= I2C_I2CONSET_AA;//����ACK
    }
    else
    {
        I2Cx->CONCLR = I2C_I2CONSET_AA;//���跢��ACK
    }
    return ucRes;
}

/*
����: I2cSendByte()
����:
    1.I2cSendByte
����:   
    1.I2Cx:     ָ��i2c�Ĵ�����ָ��
    2.ucData:   ��������
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void I2cSendByte(LPC_I2C_TypeDef *I2Cx, uint8_t ucData)
{
    I2Cx->DAT = ucData;
    ClrSi();
#if I2C_DBG
    if (GetSta(I2Cx) != I2C_I2STAT_M_TX_DAT_ACK)
    {//Ӧ�����
    #if I2C_DBG
        printf("I2c(Ch:%d) ACK Error!\r\n", I2Cx - LPC_I2C0);
    #endif
    }
#else
    GetSta(I2Cx);//�ȴ�Ӧ�����
#endif
    return;
}

/*
����: I2cRecvByte()
����:
    1.I2cRecvByte
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static uint8_t I2cRecvByte(LPC_I2C_TypeDef *I2Cx)
{
    uint8_t ucData = 0;

    ClrSi();
#if I2C_DBG
    if ((GetSta(I2Cx) != I2C_I2STAT_M_RX_DAT_ACK) && (GetSta(I2Cx) != I2C_I2STAT_M_RX_DAT_NACK))
    {//Ӧ�����
#if I2C_DBG
        printf("I2c(Ch:%d) ACK Error!\r\n", I2Cx - LPC_I2C0);
#endif
    }
#else
    GetSta(I2Cx);//�ȴ�Ӧ�����
#endif
    ucData = I2Cx->DAT;
    return ucData;
}


/*
����: I2cSend()
����:
    1.I2cSend
����:   
    1.I2cId:    ָ�����I2c������Ϣ(channel��ż�device���)�Ľṹ��ָ��
    2.pucSend:  ָ����������ݵ�ָ��
    3.uslen:    �������ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void I2cSend(stI2cDef *I2cId, uint8_t const *pucSend, const uint16_t uslen)
{
    LPC_I2C_TypeDef *I2Cx = astI2cInfo[I2cId->I2cCh].I2Cx;
    uint8_t ucAddr = aucAddr[I2cId->I2cCh][I2cId->I2cDev];//device addr
    uint16_t i = 0;
    uint8_t ucRes = 0;

    ucRes = I2cStart(I2Cx, ucAddr, Write);    
    if (ucRes)
    {   
    #if I2C_DBG
        printf("I2c(Ch:%d, Dev:%d) Start Error:%d!\r\n", I2cId->I2cCh, I2cId->I2cDev, ucRes);
    #endif
        goto Return;
    }
    for (i = 0; i < uslen; i++)
    {
        I2cSendByte(I2Cx, pucSend[i]);
    }
    ClrSi();
    I2Cx->CONSET |= I2C_I2CONSET_STO;//������ֹ����
    ClrSi();
Return:
    return;
}

/*
����: I2cRecv()
����:
    1.I2c Recveive Data
����:   
    1.I2cId:    ����I2c Channel�� DevId(��������ͬһ�����ϵĲ�ͬ�豸)��Ϣ�Ľṹ��
    2.pucRecv:  ָ����ջ����ָ��
    3.uslen:    �����յ����ݳ���
����ֵ: 
    1.��ȡ�������ݳ���
����:   void
���:   void
��ע:   void
ע��:   void
*/
uint16_t I2cRecv(stI2cDef *I2cId, uint8_t *const pucRecv, uint16_t const uslen)
{
    LPC_I2C_TypeDef *I2Cx = astI2cInfo[I2cId->I2cCh].I2Cx;
    uint8_t ucAddr = aucAddr[I2cId->I2cCh][I2cId->I2cDev];//device addr
    uint16_t i = 0;
    uint8_t ucRes = 0;

    ucRes = I2cStart(I2Cx, ucAddr, Read);
    if (ucRes)
    {   
    #if I2C_DBG
        printf("I2c(Ch:%d, Dev:%d) Start Error%d!\r\n", I2cId->I2cCh, I2cId->I2cDev, ucRes);
    #endif
        goto Return;
    }
    for (i = 0; i < uslen - 1; i++)
    {
        pucRecv[i] = I2cRecvByte(I2Cx);
    }
    I2Cx->CONCLR = I2C_I2CONCLR_AAC;//NACK
    pucRecv[i++] = I2cRecvByte(I2Cx);
    ClrSi();
    I2Cx->CONSET |= I2C_I2CONSET_STO;//������ֹ����
    ClrSi();
Return:
    return i;
}
