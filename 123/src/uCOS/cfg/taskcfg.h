#ifndef __TASK_CFG_H
#define __TASK_CFG_H

/*Sensor Task*/
#define TASK_CFG_SENSOR_PRIO                10
#define TASK_CFG_SENSOR_STK_SIZE            128
#define TASK_CFG_SENSOR_STK_SIZE_PCT_FULL   90
#define TASK_CFG_SENSOR_STK_SIZE_LIMIT      ((TASK_CFG_SENSOR_STK_SIZE * (100u - TASK_CFG_SENSOR_STK_SIZE_PCT_FULL))   / 100u)

/*GPRS Send Task*/
#define TASK_CFG_GPRS_S_PRIO                5
#define TASK_CFG_GPRS_S_STK_SIZE            128
#define TASK_CFG_GPRS_S_STK_SIZE_PCT_FULL   90
#define TASK_CFG_GPRS_S_STK_SIZE_LIMIT      ((TASK_CFG_SENSOR_STK_SIZE * (100u - TASK_CFG_SENSOR_STK_SIZE_PCT_FULL))   / 100u)

/*GPRS Recv Task*/
#define TASK_CFG_GPRS_R_PRIO                3
#define TASK_CFG_GPRS_R_STK_SIZE            128
#define TASK_CFG_GPRS_R_STK_SIZE_PCT_FULL   90
#define TASK_CFG_GPRS_R_STK_SIZE_LIMIT      ((TASK_CFG_SENSOR_STK_SIZE * (100u - TASK_CFG_SENSOR_STK_SIZE_PCT_FULL))   / 100u)

/*BD Send Task*/
#define TASK_CFG_BD_S_PRIO                  5
#define TASK_CFG_BD_S_STK_SIZE              128
#define TASK_CFG_BD_S_STK_SIZE_PCT_FULL     90
#define TASK_CFG_BD_S_STK_SIZE_LIMIT        ((TASK_CFG_SENSOR_STK_SIZE * (100u - TASK_CFG_SENSOR_STK_SIZE_PCT_FULL))   / 100u)

/*BD Recv Task*/
#define TASK_CFG_BD_R_PRIO                  3
#define TASK_CFG_BD_R_STK_SIZE              128
#define TASK_CFG_BD_R_STK_SIZE_PCT_FULL     90
#define TASK_CFG_BD_R_STK_SIZE_LIMIT        ((TASK_CFG_SENSOR_STK_SIZE * (100u - TASK_CFG_SENSOR_STK_SIZE_PCT_FULL))   / 100u)

/*Protocol Task*/
#define TASK_CFG_PRO_PRIO                   3
#define TASK_CFG_PRO_STK_SIZE               128
#define TASK_CFG_PRO_STK_SIZE_PCT_FULL      90
#define TASK_CFG_PRO_STK_SIZE_LIMIT         ((TASK_CFG_SENSOR_STK_SIZE * (100u - TASK_CFG_SENSOR_STK_SIZE_PCT_FULL))   / 100u)

#endif

