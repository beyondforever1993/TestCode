/*
文件名称: Net.c
功能:
    1.包含GPRS应用相关的函数
作者: 杜在连
修改记录:
    2017-5-19 文件初创
备注:   void
注意:   void
 */
#include "include.h"



/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/

/*****************************************变量定义************************************************/
static stNetParDef * const pstNetPar = &(stRtuCfg.stNetPar);

/******************************************函数定义***********************************************/
/****************************************static函数定义*********************************************/

/****************************************extern函数定义*********************************************/

/*
名称: SetIP()
功能:
    1.设置 服务器IP
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void SetIP(uint8_t *pucIp)
{
    memcpy(pstNetPar->IP, pucIp, IP_SZ);
    return;
}

/*
名称: NetDefault()
功能:
    1.网络参数恢复出厂设置函数
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void NetDefault(uint8_t *pucIp)
{
    /*联网参数*/
    strcpy(&(pstNetPar->IP[0]), "61.172.254.184");
    pstNetPar->usPort = 5601;
    strcpy(&(pstNetPar->APN_Num[0]), "CMNET");
    memset(&(pstNetPar->UsrNum[0]), 0, UNM_SZ);
    pstNetPar->ucType = 0;

    /*RTU网络协议参数*/
    pstNetPar->commod   = AutoComm;
    pstNetPar->CenAddr  = 0;
    //strcpy(&(pstNetPar->RTU_ID[0]), "CMNET");
    memset(&(pstNetPar->RTU_ID[0]), 0, RTUID_SZ);
    memset(&(pstNetPar->PassWd[0]), 0, RTUID_SZ);
    pstNetPar->ProMod = SL_T;
    return;
}

