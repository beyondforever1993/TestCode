#ifndef _NET_H
#define _NET_H

#define RTUID_SZ        5
#define PASSWD_SZ       2

#define APN_SZ          5
#define UNM_SZ          10//user name
#define IP_SZ           20//user name

typedef enum {
BdComm,
GprsComm,
AutoComm,
}enComModDef;

typedef enum {//网络协议设置 
SL_T,//水文协议
}enProModDef;

#pragma pack(1)
typedef struct{
/**GPRS参数定义*/
char        APN_Num[APN_SZ];//APN接入点
char        UsrNum[UNM_SZ];//APN接入点
char        IP[IP_SZ];
uint16_t    usPort;
uint8_t     ucType;//连接类型0:tcp,1:udp

/*RTU网络协议参数*/
enComModDef  commod;    //0:BD 1:GPRS 2:AUTO
uint8_t      CenAddr;//中心站地址
uint8_t      RTU_ID[RTUID_SZ];//遥测站地址
uint8_t      PassWd[PASSWD_SZ];//水文协议密码
uint8_t      TypeCode;//遥测站分类码
enProModDef  ProMod;//协议类型 0:水文协议
}stNetParDef;
#pragma pack()

extern void SetIP(uint8_t *pucIp);
extern void NetDefault(uint8_t *pucIp);

#endif
