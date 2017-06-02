#if  UL865

/*
文件名称: UL865.c
功能:
    1.包含UL865 GPRS 驱动相关的函数
作者: 杜在连
修改记录:
    2017-4-20 文件初创
备注:   void
注意:   void
*/



/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/
typedef enum
{
    T_CHECK_SIMCARD,
    T_FLOW_CTRL_CLOSE,
    //T_SET_CGCLASS, //原程序中为设置来电自动接听时的响铃次数, 个人感觉不需要，暂时删除
    T_SET_APN_SERV,
    T_SET_SKIPESC,
    T_CHECK_CREG,
    T_CHECK_SIGNAL,
    T_START_REGISTER,
    T_SET_DIAL_UN,//8
    T_SET_DIAL_PW,
    T_CHECK_PPP,
    T_START_PPP,
    T_SOCKET_CLOSE,
    T_SET_SOCKET_DELAY,
    T_TCP_CORS_CONNECT,
    T_UDP_APIS_CONNECT,
    T_DISCONNECT,
}  Telit_State;
static enChckRes UL865ChckCREG(uint8_t *pucData, uint16_t usLen, uint8_t *pucState);
static enChckRes UL865ChckSinal(uint8_t *pucData, uint16_t usLen, uint8_t *pucState);
static enChckRes UL865ChckPPP(uint8_t *pucData, uint16_t usLen, uint8_t *pucState);
static enChckRes UL865DisConnect(uint8_t *pucData, uint16_t usLen, uint8_t *pucState);
static void UL865SetAPN(uint8_t ucState);
static void UL865SetUN(uint8_t ucState);
static void UL865SetPW(uint8_t ucState);
static void UL865SetIP(uint8_t ucState);

/*****************************************变量定义************************************************/
/********************定义应答数据检测函数*******************************/
static pChckFunc UL865ChckFunc[] =
{
    ChckReady,//T_CHECK_SIMCARD
    ChckOk,//T_FLOW_CTRL_CLOSE
    ChckOk,//T_SET_APN_SERV
    ChckOk,//T_SET_SKIPESC
    UL865ChckCREG,//T_CHECK_CREG
    UL865ChckSinal,//T_CHECK_SIGNAL
    ChckOk,//T_START_REGISTER
    ChckOk,//T_SET_DIAL_UN
    ChckOk,//T_SET_DIAL_PW
    UL865ChckPPP,//T_CHECK_PPP 10
    ChckOk,//T_START_PPP
    ChckOk,//T_SOCKET_CLOSE
    ChckOk,//T_SET_SOCKET_DELAY
    ChckConnect,//T_TCP_CORS_CONNECT
    ChckConnect,//T_UDP_APIS_CONNECT
    UL865DisConnect,//T_DISCONNECT
};

static pAtFunc UL865AtFunc[] =
{
    SendAT,//T_CHECK_SIMCARD
    SendAT,//T_FLOW_CTRL_CLOSE
    UL865SetAPN,//T_SET_APN_SERV
    SendAT,//T_SET_SKIPESC
    SendAT,//T_CHECK_CREG
    SendAT,//T_CHECK_SIGNAL
    SendAT,//T_START_REGISTER
    UL865SetUN,//T_SET_DIAL_UN
    UL865SetPW,//T_SET_DIAL_PW
    SendAT,//T_CHECK_PPP 10
    SendAT,//T_START_PPP
    SendAT,//T_SOCKET_CLOSE
    SendAT,//T_SET_SOCKET_DELAY
    UL865SetIP,//T_SET_SOCKET_DELAY
    SendAT,//T_TCP_CORS_CONNECT
    SendAT,//T_DISCONNECT
};

//Telit GL868-DUAL HE910 AT 命令
static pAtCmd T_ATCmd[]=
{
  "AT+CPIN?\r\n",	// 1    -确定SIM卡是否需要PIN等密码
  "AT&K0\r",            //RS232流控制禁用
  //"ATS0=1\r",           //将来电自动应答次数设为 1 //该指令 貌似不需要
  "AT+CGDCONT=1,\"IP\"",//设置 APN等
  "AT#SKIPESC=1\r",//跳过 +++
  "AT+CREG?\r", //是否注册成功       //edit 2013.05.07
  "AT+CSQ\r", //信号强度                    //edit 2013.05.07
  "AT+CGATT=1\r",//附着状态
  "AT#USERID=",	//疑问: 根据 AT指令集，此命令应该在SCFG之后
  "AT#PASSW=",//9
  "AT#GPRS?\r\n",
  "AT#GPRS=1\r",
  "AT#SH=1\r",
  "AT#SCFG=1,1,1024,0,60,1\r",    //edit 2013.12.20
  "AT#SD=1,0",//tcp
  "AT#SD=1,1",//udp
   "+++",//T_DISCONNECT
};

/******************************************函数定义***********************************************/

/****************************************static函数定义*********************************************/
/*
名称: UL865SetAPN()
功能:
    1.  向UL865发送设置APN的命令
参数:   
    1.  ucState:    存储当前的GPRS状态的变量(ucGprsState)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void UL865SetAPN(uint8_t ucState)
{
    uint8_t *pucData = NULL;
    uint8_t ucLen = 0;
    
    pucData = MemGet(&MemSmall64);
    if (NULL == pucData)
    {
        goto Return;
    }
    ucLen = sprintf((char *)pucData, "%s,\"%s\"\r", T_ATCmd[ucState], pstNetPar->APN_Num);
    SendData(pucData, ucLen);
    MemPut(&MemSmall64, pucData);
Return:
    return;
}

/*
名称: UL865SetUN()
功能:
    1.  向UL865发送设置user name的命令
参数:   
    1.  ucState:    存储当前的GPRS状态的变量(ucGprsState)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void UL865SetUN(uint8_t ucState)
{
    uint8_t *pucData = NULL;
    uint8_t ucLen = 0;
    
    pucData = MemGet(&MemSmall64);
    if (NULL == pucData)
    {
        goto Return;
    }
    ucLen = sprintf((char *)pucData, "%s\"%s\"\r", T_ATCmd[ucState], pstNetPar->UsrNum);
    SendData(pucData, ucLen);
    MemPut(&MemSmall64, pucData);
Return:
    return;
}

/*
名称: UL865SetUN()
功能:
    1.  向UL865发送设置user password的命令
参数:   
    1.  ucState:    存储当前的GPRS状态的变量(ucGprsState)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void UL865SetPW(uint8_t ucState)
{
    uint8_t *pucData = NULL;
    uint8_t ucLen = 0;
    
    pucData = MemGet(&MemSmall64);
    if (NULL == pucData)
    {
        goto Return;
    }
    ucLen = sprintf((char *)pucData, "%s\"%s\"\r", T_ATCmd[ucState], pstNetPar->UsrNum);
    SendData(pucData, ucLen);
    MemPut(&MemSmall64, pucData);
Return:
    return;
}

/*
名称: UL865SetIP()
功能:
    1.  向UL865发送设置IP等网络参数的命令
参数:   
    1.  ucState:    存储当前的GPRS状态的变量(ucGprsState)
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void UL865SetIP(uint8_t ucState)
{
    uint8_t *pucData = NULL;
    uint8_t ucLen = 0;
    
    pucData = MemGet(&MemSmall64);
    if (NULL == pucData)
    {
        goto Return;
    }
    if (pstNetPar->ucType)
    {//udp
        ucState++;
    }
    ucLen = sprintf((char *)pucData, "%s,%d,\"%s\",0,0,0\r", T_ATCmd[ucState], pstNetPar->usPort, pstNetPar->IP);
    SendData(pucData, ucLen);
    MemPut(&MemSmall64, pucData);
Return:
    return;
}

/*
名称: UL865Sta
功能:
    1.根据收到的数据及之前的模块状态，确定当前UL865的状态
参数:   
    1. pucData: 指向来自模块的数据缓存的指针
    2. usLen:   来自模块的数据长度
    3. pucState:   指向存储当前的GPRS状态的变量(ucGprsState)的指针
返回值: 
    0:      OK
    Others: Error
输入:   void
输出:   void
备注:   void
注意:   
    1. 该函数只解析模块对"CREG"命令的应答数据
*/
static enChckRes UL865ChckCREG(uint8_t *pucData, uint16_t usLen, uint8_t *pucState)
{
    char *pcTmp = strstr((char *)pucData, "CREG:");
    enChckRes enRes = Error;
    OS_ERR err = OS_ERR_NONE;

    if (NULL == pcTmp)
    {//分包
        enRes = Packet;
        goto Return;
    }
    if (pcTmp && ((pcTmp[8] == '5') || (pcTmp[8] == '1')))
    {//就绪
        enRes = OK;
    }
    else
    {//未就绪
        enRes = Error;
        OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_DLY, &err);//等待500ms
    }
Return:
    return enRes;
}

/*
名称: UL865ChckPPP
功能:
    1.根据收到的数据及之前的模块状态，确定当前UL865的状态
参数:   
    1. pucData: 指向来自模块的数据缓存的指针
    2. usLen:   来自模块的数据长度
    3. pucState:   指向存储当前的GPRS状态的变量(ucGprsState)的指针
返回值: 
    0:      OK
    Others: Error
输入:   void
输出:   void
备注:   void
注意:   
    1. 该函数只解析模块对"GPRS?"命令的应答数据
*/
static enChckRes UL865ChckPPP(uint8_t *pucData, uint16_t usLen, uint8_t *pucState)
{
	char *pcTmp = strstr((char *)pucData, "#GPRS:");
    enChckRes enRes = Error;

    if (NULL == pcTmp)
    {
        enRes = Packet;
        goto Return;
    }
    enRes = OK;
    if (pcTmp[7] == '1')
    {//ok
        (*pucState)++;//跳过Start PPP
    }
Return:
    return enRes;
}

/*
名称: UL865DisConnect()
功能:
    1.搜索接收自GPRS模块的数据,判断是否包含"OK"字符串
    2.若检测到"OK"字符串，则设置将GPRS模块状态设为T_CHECK_PPP
参数:   
    pucData:    指向来自模块的数据的指针
    usLen:      来自模块的数据长度
    pucState:   指向存储当前的GPRS状态的变量(ucGprsState)的指针
返回值: 
    0:      OK
    others: error
输入:   void
输出:   void
备注:   void
注意:   void
*/
static enChckRes UL865DisConnect(uint8_t *pucData, uint16_t usLen, uint8_t *pucState)
{
    enChckRes enRes = Packet;

    if (strstr((char *)pucData, "OK"))
    {
        *pucState   = T_CHECK_PPP;
    }
    enRes = Error;//此处必须为Error以避免执行完该函数后状态变量自加
    return enRes;
}

/*
名称: UL865Sta
功能:
    1.根据收到的数据及之前的模块状态，确定当前UL865的状态
参数:   
    1. pucData: 指向来自模块的数据缓存的指针
    2. usLen:   来自模块的数据长度
    3. pucState:   指向存储当前的GPRS状态的变量(ucGprsState)的指针
返回值: 
    0:      OK
    Others: Error
输入:   void
输出:   void
备注:   void
注意:   
    1. 该函数只解析模块对"CREG"命令的应答数据
*/
static enChckRes UL865ChckSinal(uint8_t *pucData, uint16_t usLen, uint8_t *pucState)
{
    char *pcTmp = strstr((char *)pucData, "CSQ:");
    enChckRes enRes = Error;
    uint32_t ucTmp = 0;
    OS_ERR err = OS_ERR_NONE;

    if (NULL == pcTmp)
    {//分包
        enRes = Packet;
        goto Return;
    }
    ucTmp = atoi((char *) &pcTmp[5]);
    if ((ucTmp >= 9) && (ucTmp != 99))
    {
        enRes = OK;
    }
    else
    {
        enRes = Error;
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_DLY, &err);//等待500ms
    }
Return:
    return enRes;
}

/*
名称: UL865Rest
功能:
    1.处理
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void UL865Rest(uint8_t *pucState)
{
    OS_ERR  err = OS_ERR_NONE;
        
    if ((T_TCP_CORS_CONNECT == *pucState) || (T_UDP_APIS_CONNECT == *pucState))
    {
        *pucState = T_DISCONNECT;
    }
    else if (*pucState > T_CHECK_PPP)
    {
        *pucState = T_CHECK_PPP;
    }
    SendAT(*pucState);
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_DLY, &err);//等待5s
    SendAT(*pucState);
    ClrOnLineFLg();
    return;
}

/*
名称: UL865Rest
功能:
    1.GPRS离线检测,若检测到离线则执行SoftRest
参数:   
    1. pucData:     指向来自模块的数据缓存的指针
    2. usLen:       来自模块的数据长度
    3. pucState:    指向存储当前的GPRS状态的变量(ucGprsState)的指针
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void UL865OffLine(uint8_t *pucData, uint16_t usLen, uint8_t *pucState)
{
    if (strstr((char *)pucData, "NO CARRIER"))
    {
        *pucState   = T_CHECK_PPP;
        SendAT(*pucState);
        ClrOnLineFLg();
    }
    return;
}
#endif
