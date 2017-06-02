/*
文件名称: osTmr.c
功能:
    1.包含uCos Timer相关的函数
作者: 杜在连
修改记录:
    2017-5-11 文件初创
备注:   
    1. 使用该文件中的函数时，只需在astTmrInfo[]中添加元素，在必要时候启动timer即可
注意:   
    1. 为避免陷入先注册Timer参数后Create Timer的纠结，本文件中的参数未使用Register函数，如有需要，直接在astTmrInfo[]中填充参数即可
 */

/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/
#pragma pack(1)

typedef const struct {
OS_TMR     *p_tmr;
CPU_CHAR   *p_name;
OS_TICK     dly;//初始延时值
OS_TICK     period;//(1 / OS_CFG_TMR_TASK_RATE_HZ 为单位)
OS_OPT      opt;
OS_TMR_CALLBACK_PTR  p_callback;
void       *p_callback_arg;
}stTmrInfoDef;

#pragma pack()

/*****************************************变量定义************************************************/
//static OS_TMR_CALLBACK_PTR  apTmrFunc[MaxTmr] = {NULL};
static OS_TMR BdLedTmr;
static OS_TMR GprsLedTmr;
static OS_TMR OnlineLedTmr;
static OS_TMR SenSorLedTmr;

stTmrInfoDef astTmrInfo[] = {
{
    &BdLedTmr,//p_tmr
    "Bd LED Tmr",//p_name
    30,//delay
    20,//period (1 / OS_CFG_TMR_TASK_RATE_HZ 为单位) 100ms
    OS_OPT_TMR_PERIODIC,//opt
    LedProc,
    (void *)BdLed,// p_callback_arg
},
{
    &GprsLedTmr,//p_tmr
    "Gprs LED Tmr",//p_name
    50,//delay
    20,//period (1 / OS_CFG_TMR_TASK_RATE_HZ 为单位) 100ms
    OS_OPT_TMR_PERIODIC,//opt
    LedProc,
    (void *)GprsLed,// p_callback_arg
},
{
    &OnlineLedTmr,//p_tmr
    "Online LED Tmr",//p_name
    80,//delay
    20,//period (1 / OS_CFG_TMR_TASK_RATE_HZ 为单位) 100ms
    OS_OPT_TMR_PERIODIC,//opt
    LedProc,
    (void *)OnlineLed,// p_callback_arg
},
{
    &SenSorLedTmr,//p_tmr
    "SenSor LED Tmr",//p_name
    0,//delay
    20,//period (1 / OS_CFG_TMR_TASK_RATE_HZ 为单位) 100ms
    OS_OPT_TMR_PERIODIC,//opt
    LedProc,
    (void *)SenSorLed,// p_callback_arg
},


};

/******************************************函数定义***********************************************/
/****************************************static函数定义*********************************************/


/*
名称: CreateTmr()
功能:
    1.创建OS Timer
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void CreateTmr(void)
{
    OS_ERR err = OS_ERR_NONE;
    uint8_t i = 0;

    for (i = 0; i < SizeOfArray(astTmrInfo); i++)
    {
        stTmrInfoDef *pstTmrInfo = &astTmrInfo[i];
        
        OSTmrCreate(
                    pstTmrInfo->p_tmr,
                    pstTmrInfo->p_name,
                    pstTmrInfo->dly,//初始延时值
                    pstTmrInfo->period,
                    pstTmrInfo->opt,
                    pstTmrInfo->p_callback,
                    pstTmrInfo->p_callback_arg,
                    &err
                    );
        if (OS_ERR_NONE != err)
        {
            goto ErrReturn;
        }
    }

    return;
ErrReturn:
    printf("Timer memory err!\r\n");
    return;
}
/****************************************extern函数定义*********************************************/

/*
名称: TmrStart()
功能:
    1.注册timer call back function
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void TmrStart(enOsTmrDef enTmr)
{
    OS_ERR  err = OS_ERR_NONE;
    
    OSTmrStart(astTmrInfo[enTmr].p_tmr, &err);
    if (OS_ERR_NONE != err)
    {
        printf("Timer Start Error%d!\r\n", err);
    }
    return;
}

