/*
文件名称: ds1339.c
功能:
    1.包含ds1339驱动相关的函数
作者: 杜在连
修改记录:
    2017-4-15 文件初创
备注:   void
注意:   void
*/
#include "include.h"
#include "ds1339reg.h"

/******************************************宏定义*************************************************/
#define DS1339ADDR      0x68//I2c device addr

/*******************************************声明**************************************************/

/*****************************************变量定义************************************************/
static stI2cDef I2Cx = {DS1339CH, DS1339DEV_ID};
#define pI2c        &I2Cx

/******************************************函数定义***********************************************/
/****************************************static函数定义*********************************************/
/*
名称: Ds1339Write()
功能:
    1.向Ds1339的寄存器写入数据
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
名称: Ds1339Read()
功能:
    1.从Ds1339的寄存器读取数据
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void Ds1339Read(uint8_t ucAddr, uint8_t *pucData, uint8_t ucLen)
{
    uint8_t ucTmp = 0;

    WriteReg(ucAddr, &ucTmp, 0);//设置寄存器地址
    I2cRecv(pI2c, &pucData[0], ucLen);
    return;
}

/****************************************extern函数定义*********************************************/
/*
名称: Ds1339SetTime()
功能:
    1.将时间数据写入DS1339
参数:   
    1.stTime:   指向时间结构体的指针
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void Ds1339SetTime(stTimeDef *stTime)
{
    WriteReg(SecReg, (uint8_t *)stTime, sizeof(stTime));
    return;
}

/*
名称: Ds1339GetTime()
功能:
    1.将时间数据写入DS1339
参数:   
    1.stTime:   指向时间结构体的指针
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void Ds1339GetTime(stTimeDef *stTime)
{
    Ds1339Read(SecReg, (uint8_t *)stTime, sizeof(stTime));
    return;
}

/*
名称: BSP_Init()
功能:
    1.BSP Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数包括E2PROM_Default()函数，将比较代码中的参数及E2PROM中的参数，不一致将覆盖掉E2PROM中的参数
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
