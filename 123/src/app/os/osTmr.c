/*
�ļ�����: osTmr.c
����:
    1.����uCos Timer��صĺ���
����: ������
�޸ļ�¼:
    2017-5-11 �ļ�����
��ע:   
    1. ʹ�ø��ļ��еĺ���ʱ��ֻ����astTmrInfo[]�����Ԫ�أ��ڱ�Ҫʱ������timer����
ע��:   
    1. Ϊ����������ע��Timer������Create Timer�ľ��ᣬ���ļ��еĲ���δʹ��Register������������Ҫ��ֱ����astTmrInfo[]������������
 */

/******************************************�궨��*************************************************/

/*******************************************����**************************************************/
#pragma pack(1)

typedef const struct {
OS_TMR     *p_tmr;
CPU_CHAR   *p_name;
OS_TICK     dly;//��ʼ��ʱֵ
OS_TICK     period;//(1 / OS_CFG_TMR_TASK_RATE_HZ Ϊ��λ)
OS_OPT      opt;
OS_TMR_CALLBACK_PTR  p_callback;
void       *p_callback_arg;
}stTmrInfoDef;

#pragma pack()

/*****************************************��������************************************************/
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
    20,//period (1 / OS_CFG_TMR_TASK_RATE_HZ Ϊ��λ) 100ms
    OS_OPT_TMR_PERIODIC,//opt
    LedProc,
    (void *)BdLed,// p_callback_arg
},
{
    &GprsLedTmr,//p_tmr
    "Gprs LED Tmr",//p_name
    50,//delay
    20,//period (1 / OS_CFG_TMR_TASK_RATE_HZ Ϊ��λ) 100ms
    OS_OPT_TMR_PERIODIC,//opt
    LedProc,
    (void *)GprsLed,// p_callback_arg
},
{
    &OnlineLedTmr,//p_tmr
    "Online LED Tmr",//p_name
    80,//delay
    20,//period (1 / OS_CFG_TMR_TASK_RATE_HZ Ϊ��λ) 100ms
    OS_OPT_TMR_PERIODIC,//opt
    LedProc,
    (void *)OnlineLed,// p_callback_arg
},
{
    &SenSorLedTmr,//p_tmr
    "SenSor LED Tmr",//p_name
    0,//delay
    20,//period (1 / OS_CFG_TMR_TASK_RATE_HZ Ϊ��λ) 100ms
    OS_OPT_TMR_PERIODIC,//opt
    LedProc,
    (void *)SenSorLed,// p_callback_arg
},


};

/******************************************��������***********************************************/
/****************************************static��������*********************************************/


/*
����: CreateTmr()
����:
    1.����OS Timer
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
                    pstTmrInfo->dly,//��ʼ��ʱֵ
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
/****************************************extern��������*********************************************/

/*
����: TmrStart()
����:
    1.ע��timer call back function
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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

