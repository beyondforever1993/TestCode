#ifndef  FILE_H
#define  FILE_H

#include <ff.h>
#include "includes.h"
struct FILE{
    FIL File;
    //UINT8   bNeedSave;//是否需記錄
    UINT8   bHcnState;//記錄狀態
    char    name[30];
    char    dir[20];
    UINT8   bSDState;//SD卡状态 0：正常   1：无卡   2：满
    UINT32  StartSecond; //文件起始时间
    UINT32  SecondLength;//文件长度
    UINT16 ds;   //最后一次文件的年积日
    UINT8  tail1;//最后一次文件的编号1
    UINT8  tail2;//最后一次文件的编号2
    BSP_OS_SEM   Sem;
    UINT8        *pBuf;
    UINT16       Length;
    UINT8 bEph;
    UINT32 FreeSize;
};
extern struct FILE g_File;
void InitFS(void);
void ListFiles(void);
void FormatSDCard(void);
void GetFreeSize(void);
void CloseFile(void);


int32_t time_cmp(struct TIME_STRUCT *tm1,struct TIME_STRUCT *tm2);

UINT8 file_init();
UINT8 file_write(struct DATA_STRUCT *buf);
UINT8 file_read_last(struct DATA_STRUCT *buf);
UINT8 file_read_by_addr(struct DATA_STRUCT *buf, UINT32 addr);
UINT8 file_get_addr_by_time(UINT32 *addr, struct TIME_STRUCT *tm);




#endif