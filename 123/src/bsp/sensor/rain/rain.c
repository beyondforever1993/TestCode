/*
文件名称: rain.c
功能:
    1.包含雨量传感器相关驱动相关的函数
作者: 杜在连
修改记录:
    2017-5-22 文件初创
备注:   void
注意:   void
*/


/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/
typedef struct{//雨量数据定义
uint32_t ulNow;//当前雨量值 是指最近日起始时间开始统计至当前时刻的降水总量。 
uint32_t ulToTal;//累计雨量值 是指从某个时间起点开始（一般为1月1日的日起始时间）至统计结束时间（一般为报文编制相对应
                 //的观测时间）的降水量总值。 
}stRainDef;
/*****************************************变量定义************************************************/

/******************************************函数定义***********************************************/
static stRainDef stRainBuffer = {0};
/****************************************static函数定义*********************************************/

/*
名称: RainInit()
功能:
    1.雨量传感器 Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void RainInit(stRainDef **ppstData)
{
    *ppstData = &stRainBuffer;
    LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;
    GpioSetDir(BRD_RAIN_PORT, BRD_RAIN_PIN, 0);
    return;
}

/****************************************extern函数定义*********************************************/
/*
名称: RainGetNow()
功能:
    1.获取当前降水量
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
uint32_t RainGetNow(void)
{
    return stRainBuffer.ulNow;
}

/*
名称: RainGetToTal()
功能:
    1.获取累计降水量
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
uint32_t void RainGetToTal(void)
{
    return  stRainBuffer.ulToTal;
}

