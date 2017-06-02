#ifndef    __BD_H
#define    __BD_H

#define BD_DBG          0

typedef enum{
BdOnLine,//北斗在线
BdOffLine,//北斗离线
}enBdStaDef;

extern enBdStaDef BdChckSta(void);
extern void  BdAskIC(void);
extern void  BdInit(void);
#if BD_DBG
extern void  BdTest(void);
#endif

#endif
