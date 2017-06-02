/*
文件名称: i2c.c
功能:
    1.包含i2c驱动相关的函数
作者: 杜在连
修改记录:
    2017-4-14 文件初创
备注:
    1.如需添加其他端口驱动，只需在astI2cInfo[]数组对应位置中添加相关配置参数即可,
注意:
    1.文件初创时，只考虑了I2c1的使用，如需使用其他端口，请留意LPC1778的User Manual是否与I2c1存在配置上的差异
    2.本文件中的驱动仅使用LPC1778 I2C的Master mode,后续若需要其他模式，可以根据需要添加
*/
#include "include.h"



/******************************************宏定义*************************************************/
#define I2C_DEV_MAX         I2cDevMax//单个channel支持的最多device个数

#define RETRY_CNT           3//重试次数

#define ClrSi()             do{I2Cx->CONCLR = I2C_I2CONCLR_SIC;}while(0)
/*******************************************声明**************************************************/
typedef const struct{
stPinCfgDef  Scl;
stPinCfgDef  Sda;
}stI2cGpioDef;

typedef enum{
Write,
Read,
}enRwDef;

typedef const struct{
LPC_I2C_TypeDef *I2Cx; //指向I2C寄存器的指针(LPC_I2C0 etc.)
uint32_t ulPwrEn;   //PCONP 寄存器中的使能位(CLKPWR_PCONP_PCI2C1 etc.)
uint32_t ulFreq;    //SCL频率(HZ)
stI2cGpioDef stGpio;//scl/sda 引脚的配置信息
}stI2cInfoDef;

/*****************************************变量定义************************************************/
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

static uint8_t aucAddr[I2C_CH_MAX][I2C_DEV_MAX] = {0};//存储Salve Addr

/******************************************函数定义***********************************************/
/*
名称: SetClock()
功能:
    1.设置时钟
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
名称: I2cInit()
功能:
    1.I2c Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
        I2Cx->CONCLR = (I2C_I2CONCLR_AAC | I2C_I2CONCLR_STAC | I2C_I2CONCLR_I2ENC | I2C_I2CONCLR_SIC);//清控制寄存器
        I2Cx->CONSET = I2C_I2CONSET_I2EN;
        SetClock(I2Cx, pstI2cInfo->ulFreq);
    }
    aucAddr[I2cCh][I2cDev] = ucSlaveAddr;
    return;
}

/*
名称: GetSta()
功能:
    1.等待SI置位
    2.返回STAT寄存器状态
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static uint8_t GetSta(LPC_I2C_TypeDef *I2Cx)
{
    while(0 == (I2C_I2CONSET_SI & I2Cx->CONSET));//等待SI置位

    return (I2Cx->STAT) & I2C_STAT_CODE_BITMASK;
}

/*
名称: I2cStart()
功能:
    1.根据enRw发送I2c起始信号及起始字节
参数:   
    1. I2Cx: 指向I2c寄存器的指针
    2. enRw: read/write
返回值: 
    0:      OK
    ohers   error
输入:   void
输出:   void
备注:   void I2C_I2STAT_M_TX_START
注意:   void
*/
uint8_t I2cStart(LPC_I2C_TypeDef *I2Cx, uint8_t ucAddr, enRwDef enRw)
{
    uint8_t ucCnt = RETRY_CNT;
    uint8_t ucRes = 0;
    uint8_t ucTmp = 0;
    
    while(ucCnt)
    {
        I2Cx->CONSET |= I2C_I2CONSET_STA;//发送起始条件
        GetSta(I2Cx);//等待起始条件发送完成
        I2Cx->CONCLR = I2C_I2CONSET_STA;//停止发送 I2c起始条件
        I2Cx->DAT = (ucAddr << 1) | enRw;
        ClrSi();
        ucTmp = GetSta(I2Cx);
        if ((I2C_I2STAT_M_RX_SLAR_ACK == ucTmp) || (I2C_I2STAT_M_TX_SLAW_ACK == ucTmp))
        {//接收到应答
            ucRes = 0;
            goto Return;
        }
        ucCnt--;
    }
    ucRes = ucTmp;
Return:    
    if (Read == enRw)
    {
        I2Cx->CONSET |= I2C_I2CONSET_AA;//发送ACK
    }
    else
    {
        I2Cx->CONCLR = I2C_I2CONSET_AA;//无需发送ACK
    }
    return ucRes;
}

/*
名称: I2cSendByte()
功能:
    1.I2cSendByte
参数:   
    1.I2Cx:     指向i2c寄存器的指针
    2.ucData:   待发数据
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void I2cSendByte(LPC_I2C_TypeDef *I2Cx, uint8_t ucData)
{
    I2Cx->DAT = ucData;
    ClrSi();
#if I2C_DBG
    if (GetSta(I2Cx) != I2C_I2STAT_M_TX_DAT_ACK)
    {//应答出错
    #if I2C_DBG
        printf("I2c(Ch:%d) ACK Error!\r\n", I2Cx - LPC_I2C0);
    #endif
    }
#else
    GetSta(I2Cx);//等待应答完成
#endif
    return;
}

/*
名称: I2cRecvByte()
功能:
    1.I2cRecvByte
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static uint8_t I2cRecvByte(LPC_I2C_TypeDef *I2Cx)
{
    uint8_t ucData = 0;

    ClrSi();
#if I2C_DBG
    if ((GetSta(I2Cx) != I2C_I2STAT_M_RX_DAT_ACK) && (GetSta(I2Cx) != I2C_I2STAT_M_RX_DAT_NACK))
    {//应答出错
#if I2C_DBG
        printf("I2c(Ch:%d) ACK Error!\r\n", I2Cx - LPC_I2C0);
#endif
    }
#else
    GetSta(I2Cx);//等待应答完成
#endif
    ucData = I2Cx->DAT;
    return ucData;
}


/*
名称: I2cSend()
功能:
    1.I2cSend
参数:   
    1.I2cId:    指向包含I2c器件信息(channel编号及device编号)的结构体指针
    2.pucSend:  指向待发送数据的指针
    3.uslen:    待发数据长度
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
    I2Cx->CONSET |= I2C_I2CONSET_STO;//发送终止条件
    ClrSi();
Return:
    return;
}

/*
名称: I2cRecv()
功能:
    1.I2c Recveive Data
参数:   
    1.I2cId:    包含I2c Channel及 DevId(用于区分同一总线上的不同设备)信息的结构体
    2.pucRecv:  指向接收缓存的指针
    3.uslen:    待接收的数据长度
返回值: 
    1.读取到的数据长度
输入:   void
输出:   void
备注:   void
注意:   void
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
    I2Cx->CONSET |= I2C_I2CONSET_STO;//发送终止条件
    ClrSi();
Return:
    return i;
}
