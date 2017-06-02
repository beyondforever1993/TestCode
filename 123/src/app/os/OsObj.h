#ifndef   OS_OBJ_H
#define   OS_OBJ_H

/********************************MEMORY******************************/
#define MEM_BIG_SZ               1024
#define MEM_MIDDLE_SZ            512
#define MEM_SMALL_SZ             256
#define MEM_SMALL64_SZ           64

/**********************************TMR*******************************/
typedef enum {//该枚举定义必须与 astTmrInfo[] 一一对应
enBDLedTmr,
enGprsLedTmr,
enOnlineLedTmr,
enSenSorLedTmr,
MaxTmr,//只做计数，不可用
}enOsTmrDef;

extern OS_MEM MemBig;//1024
extern OS_MEM MemMiddle;//512
extern OS_MEM MemSmall;//256
extern OS_MEM MemSmall64;//64

extern OS_Q GprsSendQ;
extern OS_Q BDSendQ;
extern OS_Q COMRecvQ;
extern OS_Q GprsRecvQ;
extern OS_Q BDRecvQ;
extern OS_Q SenSorQ;//485 SenSor接收队列

extern OS_MUTEX GprsSendM;
extern OS_MUTEX SenSorSendM;
extern OS_MUTEX BDSendM;
extern OS_MUTEX COMSendM;

extern void  CreateObj (void);
extern void *MemGet(OS_MEM  *p_mem);
extern void MemPut(OS_MEM  *p_mem, uint8_t *pucData);
extern uint16_t MemGetMaxLen(OS_MEM  *p_mem);
extern void TmrStart(enOsTmrDef enTmr);

#endif
