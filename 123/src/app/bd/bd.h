#ifndef    __BD_H
#define    __BD_H

#define BD_DBG          0

typedef enum{
BdOnLine,//��������
BdOffLine,//��������
}enBdStaDef;

extern enBdStaDef BdChckSta(void);
extern void  BdAskIC(void);
extern void  BdInit(void);
#if BD_DBG
extern void  BdTest(void);
#endif

#endif
