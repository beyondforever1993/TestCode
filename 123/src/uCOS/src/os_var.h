#ifndef   OS_VAR_H
#define   OS_VAR_H

#ifdef OS_EXT
#undef OS_EXT
#endif

#ifdef   OS_GLOBALS
#define  OS_EXT
#else
#define  OS_EXT  extern
#endif


#if OS_CFG_APP_HOOKS_EN > 0u
OS_EXT           OS_APP_HOOK_TCB         OS_AppTaskCreateHookPtr;     /* Application hooks                            */
OS_EXT           OS_APP_HOOK_TCB         OS_AppTaskDelHookPtr;
OS_EXT           OS_APP_HOOK_TCB         OS_AppTaskReturnHookPtr;

OS_EXT           OS_APP_HOOK_VOID        OS_AppIdleTaskHookPtr;
OS_EXT           OS_APP_HOOK_VOID        OS_AppStatTaskHookPtr;
OS_EXT           OS_APP_HOOK_VOID        OS_AppTaskSwHookPtr;
OS_EXT           OS_APP_HOOK_VOID        OS_AppTimeTickHookPtr;
#endif

                                                                      /* IDLE TASK ---------------------------------- */
OS_EXT            OS_IDLE_CTR            OSIdleTaskCtr;
OS_EXT            OS_TCB                 OSIdleTaskTCB;

                                                                      /* MISCELLANEOUS ------------------------------ */
OS_EXT            OS_NESTING_CTR         OSIntNestingCtr;             /* Interrupt nesting level                      */
#ifdef CPU_CFG_INT_DIS_MEAS_EN
OS_EXT            CPU_TS                 OSIntDisTimeMax;             /* Overall interrupt disable time               */
#endif

OS_EXT            OS_STATE               OSRunning;                   /* Flag indicating that kernel is running       */


                                                                      /* ISR HANDLER TASK --------------------------- */
#if OS_CFG_ISR_POST_DEFERRED_EN > 0u
OS_EXT            OS_INT_Q              *OSIntQInPtr;
OS_EXT            OS_INT_Q              *OSIntQOutPtr;
OS_EXT            OS_OBJ_QTY             OSIntQNbrEntries;
OS_EXT            OS_OBJ_QTY             OSIntQMaxNbrEntries;
OS_EXT            OS_OBJ_QTY             OSIntQOvfCtr;
OS_EXT            OS_TCB                 OSIntQTaskTCB;
OS_EXT            CPU_TS                 OSIntQTaskTimeMax;
#endif

                                                                      /* FLAGS -------------------------------------- */
#if OS_CFG_FLAG_EN > 0u
#if OS_CFG_DBG_EN  > 0u
OS_EXT            OS_FLAG_GRP           *OSFlagDbgListPtr;
#endif
OS_EXT            OS_OBJ_QTY             OSFlagQty;
#endif

                                                                      /* MEMORY MANAGEMENT -------------------------- */
#if OS_CFG_MEM_EN > 0u
#if OS_CFG_DBG_EN > 0u
OS_EXT            OS_MEM                *OSMemDbgListPtr;
#endif
OS_EXT            OS_OBJ_QTY             OSMemQty;                    /* Number of memory partitions created          */
#endif

                                                                      /* OS_MSG POOL -------------------------------- */
OS_EXT            OS_MSG_POOL            OSMsgPool;                   /* Pool of OS_MSG                               */

                                                                      /* MUTEX MANAGEMENT --------------------------- */
#if OS_CFG_MUTEX_EN > 0u
#if OS_CFG_DBG_EN   > 0u
OS_EXT            OS_MUTEX              *OSMutexDbgListPtr;
#endif
OS_EXT            OS_OBJ_QTY             OSMutexQty;                  /* Number of mutexes created                    */
#endif

                                                                      /* PRIORITIES --------------------------------- */
OS_EXT            OS_PRIO                OSPrioCur;                   /* Priority of current task                     */
OS_EXT            OS_PRIO                OSPrioHighRdy;               /* Priority of highest priority task            */
OS_EXT            OS_PRIO                OSPrioSaved;                 /* Saved priority level when Post Deferred      */
extern            CPU_DATA               OSPrioTbl[OS_PRIO_TBL_SIZE];

                                                                      /* QUEUES ------------------------------------- */
#if OS_CFG_Q_EN   > 0u
#if OS_CFG_DBG_EN > 0u
OS_EXT            OS_Q                  *OSQDbgListPtr;
#endif
OS_EXT            OS_OBJ_QTY             OSQQty;                      /* Number of message queues created             */
#endif



                                                                      /* READY LIST --------------------------------- */
OS_EXT            OS_RDY_LIST            OSRdyList[OS_CFG_PRIO_MAX];  /* Table of tasks ready to run                  */


#ifdef OS_SAFETY_CRITICAL_IEC61508
OS_EXT            CPU_BOOLEAN            OSSafetyCriticalStartFlag;   /* Flag indicating that all init. done          */
#endif
                                                                      /* SCHEDULER ---------------------------------- */
#if OS_CFG_SCHED_LOCK_TIME_MEAS_EN > 0u
OS_EXT            CPU_TS_TMR             OSSchedLockTimeBegin;        /* Scheduler lock time measurement              */
OS_EXT            CPU_TS_TMR             OSSchedLockTimeMax;
OS_EXT            CPU_TS_TMR             OSSchedLockTimeMaxCur;
#endif

OS_EXT            OS_NESTING_CTR         OSSchedLockNestingCtr;       /* Lock nesting level                           */
#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
OS_EXT            OS_TICK                OSSchedRoundRobinDfltTimeQuanta;
OS_EXT            CPU_BOOLEAN            OSSchedRoundRobinEn;         /* Enable/Disable round-robin scheduling        */
#endif
                                                                      /* SEMAPHORES --------------------------------- */
#if OS_CFG_SEM_EN > 0u
#if OS_CFG_DBG_EN > 0u
OS_EXT            OS_SEM                *OSSemDbgListPtr;
#endif
OS_EXT            OS_OBJ_QTY             OSSemQty;                    /* Number of semaphores created                 */
#endif

                                                                      /* STATISTICS --------------------------------- */
#if OS_CFG_STAT_TASK_EN > 0u
OS_EXT            OS_CPU_USAGE           OSStatTaskCPUUsage;          /* CPU Usage in %                               */
OS_EXT            OS_TICK                OSStatTaskCtr;
OS_EXT            OS_TICK                OSStatTaskCtrMax;
OS_EXT            OS_TICK                OSStatTaskCtrRun;
OS_EXT            CPU_BOOLEAN            OSStatTaskRdy;
OS_EXT            OS_TCB                 OSStatTaskTCB;
OS_EXT            CPU_TS                 OSStatTaskTimeMax;
OS_EXT            CPU_BOOLEAN            OSStatResetFlag;             /* Force the reset of the computed statistics   */
#endif

                                                                      /* TASKS -------------------------------------- */
OS_EXT            OS_CTX_SW_CTR          OSTaskCtxSwCtr;              /* Number of context switches                   */
#if OS_CFG_DBG_EN > 0u
OS_EXT            OS_TCB                *OSTaskDbgListPtr;
#endif
OS_EXT            OS_OBJ_QTY             OSTaskQty;                   /* Number of tasks created                      */


                                                                      /* TICK TASK ---------------------------------- */
OS_EXT            OS_TICK                OSTickCtr;                   /* Counts the #ticks since startup or last set  */
OS_EXT            OS_TCB                 OSTickTaskTCB;
OS_EXT            CPU_TS                 OSTickTaskTimeMax;


#if OS_CFG_TMR_EN > 0u                                                /* TIMERS ------------------------------------- */
#if OS_CFG_DBG_EN > 0u
OS_EXT            OS_TMR                *OSTmrDbgListPtr;
#endif
OS_EXT            OS_OBJ_QTY             OSTmrQty;                    /* Number of timers created                     */
OS_EXT            OS_TCB                 OSTmrTaskTCB;                /* TCB of timer task                            */
OS_EXT            CPU_TS                 OSTmrTaskTimeMax;
OS_EXT            OS_TICK                OSTmrTickCtr;                /* Current time for the timers                  */
OS_EXT            OS_CTR                 OSTmrUpdateCnt;              /* Counter for updating timers                  */
OS_EXT            OS_CTR                 OSTmrUpdateCtr;
#endif

                                                                      /* TCBs --------------------------------------- */
OS_EXT            OS_TCB                *OSTCBCurPtr;                 /* Pointer to currently running TCB             */
OS_EXT            OS_TCB                *OSTCBHighRdyPtr;             /* Pointer to highest priority  TCB             */

#endif
