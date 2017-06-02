#include "include.h"

int main(void)                       /* Main Program */
{
    OS_ERR   os_err;
    BSP_Init();
    OSInit(&os_err);                                            /* Init uC/OS-III.                                      */
    IntEn();// enable interrupt
    OSTaskCreate((OS_TCB      *)&TaskSensorTCB,            /* Create the start task                                */
                 (CPU_CHAR    *)"Sensor",
                 (OS_TASK_PTR  )TaskSensor, 
                 (void        *)0,
                 (OS_PRIO      )TASK_CFG_SENSOR_PRIO,
                 (CPU_STK     *)TaskSensorStkPtr,
                 (CPU_STK_SIZE )TASK_CFG_SENSOR_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE )TASK_CFG_SENSOR_STK_SIZE,
                 (OS_MSG_QTY   )0u,
                 (OS_TICK      )0u,
                 (void        *)0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR      *)&os_err);
    
    OSStart(&os_err);                                           /* Start multitasking.                                    */
    
    (void)&os_err;

    return (0);
}


/***************************************************************************/
 

