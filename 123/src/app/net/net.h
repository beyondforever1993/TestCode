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

typedef enum {//����Э������ 
SL_T,//ˮ��Э��
}enProModDef;

#pragma pack(1)
typedef struct{
/**GPRS��������*/
char        APN_Num[APN_SZ];//APN�����
char        UsrNum[UNM_SZ];//APN�����
char        IP[IP_SZ];
uint16_t    usPort;
uint8_t     ucType;//��������0:tcp,1:udp

/*RTU����Э�����*/
enComModDef  commod;    //0:BD 1:GPRS 2:AUTO
uint8_t      CenAddr;//����վ��ַ
uint8_t      RTU_ID[RTUID_SZ];//ң��վ��ַ
uint8_t      PassWd[PASSWD_SZ];//ˮ��Э������
uint8_t      TypeCode;//ң��վ������
enProModDef  ProMod;//Э������ 0:ˮ��Э��
}stNetParDef;
#pragma pack()

extern void SetIP(uint8_t *pucIp);
extern void NetDefault(uint8_t *pucIp);

#endif
