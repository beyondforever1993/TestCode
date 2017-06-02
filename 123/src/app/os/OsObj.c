/*
文件名称: osobj.c
功能:
    1.包含与内核对象定义相关的函数
作者: 杜在连
修改记录:
    2017-4-16 文件初创
备注:   void
注意:   void
*/
#include "include.h"

#include "osTmr.c"


/******************************************宏定义*************************************************/

/*********************************Memory Size***************************/
#define MEM_SIZE                (50 * 1024)//pool 总容量60k

#define MEM_BIG_NUM              15//1024 byte block可申请的最大个数
#define MEM_MIDDLE_NUM           30//512 byte block可申请的最大个数
#define MEM_SMALL_NUM            40//256 byte block可申请的最大个数
#define MEM_SMALL64_NUM          160//64 byte block可申请的最大个数  

#if (((MEM_BIG_NUM * 1024) + (MEM_MIDDLE_NUM * 512) + (MEM_SMALL_NUM * 256) + (MEM_SMALL64_NUM * 64)) != MEM_SIZE)
#error  Memory Cfg err!//内存池总量必须与各内存池之和一致
#endif

/*****************************消息队列深度**********************************/
#define Q_MAX_GPRS              10//GPRS 数据发送/接收队列深度
#define Q_MAX_BD                10//BD  数据发送/队列深度
#define Q_MAX_SenSor            10//485 传感器接收队列深度
#define Q_MAX_COM               10//COM 数据接收队列深度

/*******************************************声明**************************************************/

#pragma pack(1)

typedef struct{
uint8_t aucMemBig[MEM_BIG_NUM][MEM_BIG_SZ];//1024 * 10
uint8_t aucMemMiddle[MEM_MIDDLE_NUM][MEM_MIDDLE_SZ];//512 * 40
uint8_t aucMemSmall[MEM_SMALL_NUM][MEM_SMALL_SZ];//256 * 80
uint8_t aucMemSmall64[MEM_SMALL64_NUM][MEM_SMALL64_SZ];//64 * 160
}stMemBuffDef;

typedef const struct {
OS_Q        *p_q;
CPU_CHAR    *p_name;// name
OS_MSG_QTY   max_qty;//队列深度
}stQ_InfoDef;

typedef const struct {
OS_MEM      *p_mem;
CPU_CHAR    *p_name;
void        *p_addr;
OS_MEM_QTY   n_blks;
OS_MEM_SIZE  blk_size;
}stMemInfoDef;

typedef const struct {
OS_MUTEX    *p_mutex;
CPU_CHAR    *p_name;
}stMutexInfoDef;

#pragma pack()

/*****************************************变量定义************************************************/
static uint8_t  aucMemPool[MEM_SIZE] = {0};
#define pstMemBuff    ((stMemBuffDef *)&aucMemPool[0])

/*若要添加新OS_MEM, 必须同步改动MemGet()*/
OS_MEM MemBig;//1024
OS_MEM MemMiddle;//512
OS_MEM MemSmall;//256
OS_MEM MemSmall64;//64

OS_Q GprsSendQ;
OS_Q BDSendQ;
OS_Q COMRecvQ;
OS_Q GprsRecvQ;
OS_Q BDRecvQ;
OS_Q SenSorQ;//485 SenSor接收队列

OS_MUTEX GprsSendM;
OS_MUTEX SenSorSendM;
OS_MUTEX BDSendM;
OS_MUTEX COMSendM;


stQ_InfoDef astQ_Info[] = {
{//GPRS Send
    &GprsSendQ,
    "Gprs Send",
    (OS_MSG_QTY)Q_MAX_GPRS,
},
{//GPRS Recv
    &GprsRecvQ,
    "Gprs Recv",
    (OS_MSG_QTY)Q_MAX_GPRS,
},
{//BD Send
    &BDSendQ,
    "BD Send",
    (OS_MSG_QTY)Q_MAX_BD,
},
{//BD Recv
    &BDRecvQ,
    "BD Recv",
    (OS_MSG_QTY)Q_MAX_BD,
},
{
    &SenSorQ,
    "SenSor Recv",
    (OS_MSG_QTY)Q_MAX_SenSor,
},
{
    &COMRecvQ,
    "COM Recv",
    (OS_MSG_QTY)Q_MAX_COM,
},

};

stMemInfoDef astMemInfo[] = {
{
    &MemBig,
    "Big Mem",
    &(pstMemBuff->aucMemBig[0][0]),
    MEM_BIG_NUM,
    MEM_BIG_SZ,
},
{
    &MemMiddle,
    "Middle Mem",
    &(pstMemBuff->aucMemMiddle[0][0]),
    MEM_MIDDLE_NUM,
    MEM_MIDDLE_SZ,
},
{
    &MemSmall,
    "Small Mem",
    &(pstMemBuff->aucMemSmall[0][0]),
    MEM_SMALL_NUM,
    MEM_SMALL_SZ,
},
{
    &MemSmall64,
    "Small64 Mem",
    &(pstMemBuff->aucMemSmall64[0][0]),
    MEM_SMALL64_NUM,
    MEM_SMALL64_SZ,
},
}; 

stMutexInfoDef astMutexInfo[] = {
{
    &GprsSendM,
    "Gprs Send",
},
{
    &SenSorSendM,
    "SenSor Send",
},
{
    &BDSendM,
    "BD Send",
},
{
    &COMSendM,
    "COM Send",
},
};

/******************************************函数定义***********************************************/
/****************************************static函数定义*********************************************/
/****************************************extern函数定义*********************************************/


/*
名称: CreateMem()
功能:
    1.Create a fixed-sized memory partition that will be managed by uC/OS-III.
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void  CreateMem(void)
{
    OS_ERR err = OS_ERR_NONE;
    uint8_t i = 0;

    for (i = 0; i < SizeOfArray(astMemInfo); i++)
    {
        stMemInfoDef *pstMemInfo = &astMemInfo[i];
        
        OSMemCreate(
                    pstMemInfo->p_mem,
                    pstMemInfo->p_name,
                    pstMemInfo->p_addr,
                    pstMemInfo->n_blks,
                    pstMemInfo->blk_size,
                    &err
                    );
        if (OS_ERR_NONE != err)
        {
            goto ErrReturn;
        }
    }

    return;
ErrReturn:
    printf("creat memory err!\r\n");
    return;
}

/*
名称: CreateQ()
功能:
    1.创建消息队列
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void  CreateQ(void)
{
    OS_ERR err = OS_ERR_NONE;
    stQ_InfoDef *pstQ_Info = NULL;
    uint8_t i = 0;

    for (i = 0; i < SizeOfArray(astQ_Info); i++)
    {
        pstQ_Info = &astQ_Info[i];
        
        OSQCreate(
                    pstQ_Info->p_q,
                    pstQ_Info->p_name,
                    pstQ_Info->max_qty,
                    &err
                );
        if (OS_ERR_NONE != err)
        {
            goto ErrReturn;
        }
    }

    return;
ErrReturn:
    printf("Creat Q Error!\r\n");
    return;
}

/*
名称: CreateMutex()
功能:
    1.创建互斥信号量
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void  CreateMutex(void)
{
    OS_ERR err = OS_ERR_NONE;
    uint8_t i = 0;
    stMutexInfoDef *pstMutexInfo = NULL;

    for (i = 0; i < SizeOfArray(astMutexInfo); i++)
    {
        pstMutexInfo = &astMutexInfo[i];
        OSMutexCreate(
                        pstMutexInfo->p_mutex,
                        pstMutexInfo->p_name,
                        &err
                     );
        if (OS_ERR_NONE != err)
        {
            goto ErrReturn;
        }
    }
    return;
ErrReturn:
    printf("Creat Mutex Error!\r\n");
    return;
}


/*
名称: CreateObj()
功能:
    1.创建内核对象
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void CreateObj(void)
{
    CreateMem();//memory
    CreateQ();//消息队列
    CreateMutex();//互斥信号量
    CreateTmr();//创建timer
    return;
}

/*
名称: MemGetMaxLen()
功能:
    1.获取当前缓存支持的最大数据长度
参数:   
    1.p_mem   is a pointer to the memory partition control block(MemBig etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
uint16_t MemGetMaxLen(OS_MEM  *p_mem)
{
    uint16_t    usLen = 0;

    if (p_mem == &MemBig)
    {
        usLen = MEM_BIG_SZ;
    }
    else if (p_mem == &MemMiddle)
    {
        usLen = MEM_MIDDLE_SZ;
    }
    else if (p_mem == &MemSmall)
    {
        usLen = MEM_SMALL_SZ;
    }    
    else if (p_mem == &MemSmall64)
    {
        usLen = MEM_SMALL64_SZ;
    }
    return usLen;
}

/*
名称: MemGet()
功能:
    1.向OS申请内存
参数:   
    1.p_mem   is a pointer to the memory partition control block(MemBig etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.调用该函数申请到的内存用完后,必须调用MemPut()函数释放掉
*/
void *MemGet(OS_MEM  *p_mem)
{
    void        *pMem   = NULL;
    OS_ERR      err     = OS_ERR_NONE;
    uint16_t    usLen = 0;
        
    pMem = OSMemGet(p_mem, &err);
    if (OS_ERR_NONE != err)
    {
        goto Return;
    }
    usLen = MemGetMaxLen(p_mem);
    memset(pMem, 0, usLen);
Return:
    return pMem;
}

/*
名称: MemPut()
功能:
    1.向uCOS申请的内存还回内存池
参数:   
    1.p_mem   is a pointer to the memory partition control block(MemBig etc.)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void MemPut(OS_MEM  *p_mem, uint8_t *pucData)
{
    OS_ERR      err     = OS_ERR_NONE;
    
    OSMemPut(p_mem, pucData, &err);
    return;
}
