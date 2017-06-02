/*
�ļ�����: Net.c
����:
    1.����GPRSӦ����صĺ���
����: ������
�޸ļ�¼:
    2017-5-19 �ļ�����
��ע:   void
ע��:   void
 */
#include "include.h"



/******************************************�궨��*************************************************/

/*******************************************����**************************************************/

/*****************************************��������************************************************/
static stNetParDef * const pstNetPar = &(stRtuCfg.stNetPar);

/******************************************��������***********************************************/
/****************************************static��������*********************************************/

/****************************************extern��������*********************************************/

/*
����: SetIP()
����:
    1.���� ������IP
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void SetIP(uint8_t *pucIp)
{
    memcpy(pstNetPar->IP, pucIp, IP_SZ);
    return;
}

/*
����: NetDefault()
����:
    1.��������ָ��������ú���
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void NetDefault(uint8_t *pucIp)
{
    /*��������*/
    strcpy(&(pstNetPar->IP[0]), "61.172.254.184");
    pstNetPar->usPort = 5601;
    strcpy(&(pstNetPar->APN_Num[0]), "CMNET");
    memset(&(pstNetPar->UsrNum[0]), 0, UNM_SZ);
    pstNetPar->ucType = 0;

    /*RTU����Э�����*/
    pstNetPar->commod   = AutoComm;
    pstNetPar->CenAddr  = 0;
    //strcpy(&(pstNetPar->RTU_ID[0]), "CMNET");
    memset(&(pstNetPar->RTU_ID[0]), 0, RTUID_SZ);
    memset(&(pstNetPar->PassWd[0]), 0, RTUID_SZ);
    pstNetPar->ProMod = SL_T;
    return;
}

