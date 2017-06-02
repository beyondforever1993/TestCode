#ifndef __TASK_H
#define __TASK_H

extern OS_TCB TaskSensorTCB;
extern CPU_STK TaskSensorStkPtr[TASK_CFG_SENSOR_STK_SIZE] ;

extern void  TaskSensor (void *p_arg);

#endif
