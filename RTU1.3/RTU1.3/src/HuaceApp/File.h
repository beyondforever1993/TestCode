#ifndef  FILE_H
#define  FILE_H

#include <ff.h>
#include "includes.h"
struct FILE{
    FIL File;
    //UINT8   bNeedSave;//�Ƿ���ӛ�
    UINT8   bHcnState;//ӛ䛠�B
    char    name[30];
    char    dir[20];
    UINT8   bSDState;//SD��״̬ 0������   1���޿�   2����
    UINT32  StartSecond; //�ļ���ʼʱ��
    UINT32  SecondLength;//�ļ�����
    UINT16 ds;   //���һ���ļ��������
    UINT8  tail1;//���һ���ļ��ı��1
    UINT8  tail2;//���һ���ļ��ı��2
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