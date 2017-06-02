#include "include.h"


/******************************************宏定义*************************************************/
/*******************************************声明**************************************************/
typedef const struct {
OS_TCB        *p_tcb;
CPU_CHAR      *p_name;
OS_TASK_PTR    p_task;
OS_PRIO        prio;
CPU_STK       *p_stk_base;
CPU_STK_SIZE   stk_limit;
CPU_STK_SIZE   stk_size;
}stTaskInfoDef;

static  void  CreateTask (void);

static void  TaskBdS(void *p_arg);
static void  TaskBdR(void *p_arg);
static void  TaskPro(void *p_arg);

/*****************************************变量定义************************************************/
OS_TCB TaskSensorTCB = {0};
OS_TCB TaskGprs_S_TCB = {0};
OS_TCB TaskGprs_R_TCB = {0};
OS_TCB TaskBD_S_TCB = {0};
OS_TCB TaskBD_R_TCB = {0};
OS_TCB TaskPRO_TCB = {0};


CPU_STK TaskSensorStkPtr[TASK_CFG_SENSOR_STK_SIZE] = {0};

CPU_STK TaskGprsS_StkPtr[TASK_CFG_GPRS_S_STK_SIZE] = {0};
CPU_STK TaskGprsR_StkPtr[TASK_CFG_GPRS_R_STK_SIZE] = {0};
CPU_STK TaskBdS_StkPtr[TASK_CFG_BD_S_STK_SIZE]     = {0};
CPU_STK TaskBdR_StkPtr[TASK_CFG_BD_R_STK_SIZE]     = {0};
CPU_STK TaskProStkPtr[TASK_CFG_PRO_STK_SIZE]       = {0};

stTaskInfoDef astTaskInfo[] = {
{
    &TaskGprs_S_TCB,            /* Create the start task                                */
    "GPRS Send",
    TaskGprsS, 
    TASK_CFG_GPRS_S_PRIO,
    TaskGprsS_StkPtr,
    TASK_CFG_GPRS_S_STK_SIZE_LIMIT,
    TASK_CFG_GPRS_S_STK_SIZE,
},
{
    &TaskGprs_R_TCB,            /* Create the start task                                */
    "GPRS Recv",
    TaskGprsR, 
    TASK_CFG_GPRS_R_PRIO,
    TaskGprsR_StkPtr,
    TASK_CFG_GPRS_R_STK_SIZE_LIMIT,
    TASK_CFG_GPRS_R_STK_SIZE,
},
#if 0
{
    &TaskBD_S_TCB,            /* Create the start task                                */
    "BD Send",
    TaskBdS, 
    TASK_CFG_BD_S_PRIO,
    TaskBdS_StkPtr,
    TASK_CFG_BD_S_STK_SIZE_LIMIT,
    TASK_CFG_BD_S_STK_SIZE,
},
{
    &TaskBD_R_TCB,            /* Create the start task                                */
    "BD Recv",
    TaskBdR, 
    TASK_CFG_BD_R_PRIO,
    TaskBdR_StkPtr,
    TASK_CFG_BD_R_STK_SIZE_LIMIT,
    TASK_CFG_BD_R_STK_SIZE,
},
#endif
{
    &TaskPRO_TCB,            /* Create the start task                                */
    "Protocol",
    TaskPro, 
    TASK_CFG_PRO_PRIO,
    TaskProStkPtr,
    TASK_CFG_PRO_STK_SIZE_LIMIT,
    TASK_CFG_PRO_STK_SIZE,
},
};


/*******************************************函数定义************************************************/
/****************************************static函数定义*********************************************/
/*
名称: CreateTask()
功能:
    1.创建Task
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static  void  CreateTask (void)
{
    OS_ERR  err;
    uint8_t i = 0;
    stTaskInfoDef *pstTaskInfo = NULL;

    OSSchedLock(&err);
    for(i = 0; i < SizeOfArray(astTaskInfo); i++)
    {
        pstTaskInfo = &astTaskInfo[i];
        OSTaskCreate((OS_TCB      *)pstTaskInfo->p_tcb,            /* Create the start task                                */
                     (CPU_CHAR    *)pstTaskInfo->p_name,
                     (OS_TASK_PTR  )pstTaskInfo->p_task, 
                     (void        *)0,
                     (OS_PRIO      )pstTaskInfo->prio,
                     (CPU_STK     *)pstTaskInfo->p_stk_base,
                     (CPU_STK_SIZE )pstTaskInfo->stk_limit,
                     (CPU_STK_SIZE )pstTaskInfo->stk_size,
                     (OS_MSG_QTY   )0u,
                     (OS_TICK      )0u,
                     (void        *)0,
                     (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                     (OS_ERR      *)&err);
    }
    OSSchedUnlock(&err);

    return;
}


/*
名称: TaskBdS
功能:
    1.BD Data Send Task
参数:   
    p_arg: 传入任务的形参
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void  TaskBdS(void *p_arg)
{
    BdInit();
    while(1)
    {
        
    }
    return;
}

/*
名称: TaskBdR
功能:
    1.BD Data Recv Task
参数:   
    p_arg: 传入任务的形参
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void  TaskBdR(void *p_arg)
{
    uint8_t *pucData = NULL;
    uint16_t usLen = 0;
    CPU_TS   ts;
    OS_ERR   err;

    while(1)
    {
        pucData = (uint8_t *)OSQPend(&BDRecvQ, 0, OS_OPT_PEND_BLOCKING, &usLen, &ts, &err);
        if (OS_ERR_NONE == err)
        {
            UartFreeBuf(UART_BD_CH, pucData);
        }
    }
    return;
}

/*
名称: TaskPro
功能:
    1.Protocol Task
参数:   
    p_arg: 传入任务的形参
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void  TaskPro(void *p_arg)
{
    uint8_t *pucData = NULL;
    uint16_t usLen = 0;
    CPU_TS   ts;
    OS_ERR   err;
    
    while(1)
    {
        pucData = (uint8_t *)OSQPend(&COMRecvQ, 0, OS_OPT_PEND_BLOCKING, &usLen, &ts, &err);
        if (OS_ERR_NONE == err)
        {
            UartDeal(pucData, usLen);//处理串口数据
            UartFreeBuf(UART_COM_CH, pucData);
        }
        
    }
    return;
}

/****************************************extern函数定义*********************************************/

/*
名称: TaskSensor()
功能:
    1.CPU reset后创建的第一个任务,用于BSP初始化、任务及内核对象创建等功能
参数:   
    p_arg: 传入任务的形参
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void  TaskSensor (void *p_arg)
{
    OS_ERR  err;

    (void)p_arg;
    OS_CSP_TickInit();                                          /* Initialize the Tick interrupt.                       */
    CreateObj();                                            /* Create Applicaiton kernel objects                    */
    AppInit();
    CreateTask();                                           /* Create Application tasks                             */
    SenSorInit();
    while (DEF_TRUE) 
    {                                          /* Task body, always written as an infinite loop.       */
        SenSorPro();
        OSTimeDlyHMSM(0u, 0u, 0u, 500u,                         /* Delay for 100ms.                                     */
                      OS_OPT_TIME_DLY | OS_OPT_TIME_HMSM_STRICT, 
                      &err);
    }
    return;
}




