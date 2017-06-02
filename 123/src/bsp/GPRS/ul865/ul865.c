#if  UL865

/*
�ļ�����: UL865.c
����:
    1.����UL865 GPRS ������صĺ���
����: ������
�޸ļ�¼:
    2017-4-20 �ļ�����
��ע:   void
ע��:   void
*/



/******************************************�궨��*************************************************/

/*******************************************����**************************************************/
typedef enum
{
    T_CHECK_SIMCARD,
    T_FLOW_CTRL_CLOSE,
    //T_SET_CGCLASS, //ԭ������Ϊ���������Զ�����ʱ���������, ���˸о�����Ҫ����ʱɾ��
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

/*****************************************��������************************************************/
/********************����Ӧ�����ݼ�⺯��*******************************/
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

//Telit GL868-DUAL HE910 AT ����
static pAtCmd T_ATCmd[]=
{
  "AT+CPIN?\r\n",	// 1    -ȷ��SIM���Ƿ���ҪPIN������
  "AT&K0\r",            //RS232�����ƽ���
  //"ATS0=1\r",           //�������Զ�Ӧ�������Ϊ 1 //��ָ�� ò�Ʋ���Ҫ
  "AT+CGDCONT=1,\"IP\"",//���� APN��
  "AT#SKIPESC=1\r",//���� +++
  "AT+CREG?\r", //�Ƿ�ע��ɹ�       //edit 2013.05.07
  "AT+CSQ\r", //�ź�ǿ��                    //edit 2013.05.07
  "AT+CGATT=1\r",//����״̬
  "AT#USERID=",	//����: ���� ATָ���������Ӧ����SCFG֮��
  "AT#PASSW=",//9
  "AT#GPRS?\r\n",
  "AT#GPRS=1\r",
  "AT#SH=1\r",
  "AT#SCFG=1,1,1024,0,60,1\r",    //edit 2013.12.20
  "AT#SD=1,0",//tcp
  "AT#SD=1,1",//udp
   "+++",//T_DISCONNECT
};

/******************************************��������***********************************************/

/****************************************static��������*********************************************/
/*
����: UL865SetAPN()
����:
    1.  ��UL865��������APN������
����:   
    1.  ucState:    �洢��ǰ��GPRS״̬�ı���(ucGprsState)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: UL865SetUN()
����:
    1.  ��UL865��������user name������
����:   
    1.  ucState:    �洢��ǰ��GPRS״̬�ı���(ucGprsState)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: UL865SetUN()
����:
    1.  ��UL865��������user password������
����:   
    1.  ucState:    �洢��ǰ��GPRS״̬�ı���(ucGprsState)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: UL865SetIP()
����:
    1.  ��UL865��������IP���������������
����:   
    1.  ucState:    �洢��ǰ��GPRS״̬�ı���(ucGprsState)
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: UL865Sta
����:
    1.�����յ������ݼ�֮ǰ��ģ��״̬��ȷ����ǰUL865��״̬
����:   
    1. pucData: ָ������ģ������ݻ����ָ��
    2. usLen:   ����ģ������ݳ���
    3. pucState:   ָ��洢��ǰ��GPRS״̬�ı���(ucGprsState)��ָ��
����ֵ: 
    0:      OK
    Others: Error
����:   void
���:   void
��ע:   void
ע��:   
    1. �ú���ֻ����ģ���"CREG"�����Ӧ������
*/
static enChckRes UL865ChckCREG(uint8_t *pucData, uint16_t usLen, uint8_t *pucState)
{
    char *pcTmp = strstr((char *)pucData, "CREG:");
    enChckRes enRes = Error;
    OS_ERR err = OS_ERR_NONE;

    if (NULL == pcTmp)
    {//�ְ�
        enRes = Packet;
        goto Return;
    }
    if (pcTmp && ((pcTmp[8] == '5') || (pcTmp[8] == '1')))
    {//����
        enRes = OK;
    }
    else
    {//δ����
        enRes = Error;
        OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_DLY, &err);//�ȴ�500ms
    }
Return:
    return enRes;
}

/*
����: UL865ChckPPP
����:
    1.�����յ������ݼ�֮ǰ��ģ��״̬��ȷ����ǰUL865��״̬
����:   
    1. pucData: ָ������ģ������ݻ����ָ��
    2. usLen:   ����ģ������ݳ���
    3. pucState:   ָ��洢��ǰ��GPRS״̬�ı���(ucGprsState)��ָ��
����ֵ: 
    0:      OK
    Others: Error
����:   void
���:   void
��ע:   void
ע��:   
    1. �ú���ֻ����ģ���"GPRS?"�����Ӧ������
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
        (*pucState)++;//����Start PPP
    }
Return:
    return enRes;
}

/*
����: UL865DisConnect()
����:
    1.����������GPRSģ�������,�ж��Ƿ����"OK"�ַ���
    2.����⵽"OK"�ַ����������ý�GPRSģ��״̬��ΪT_CHECK_PPP
����:   
    pucData:    ָ������ģ������ݵ�ָ��
    usLen:      ����ģ������ݳ���
    pucState:   ָ��洢��ǰ��GPRS״̬�ı���(ucGprsState)��ָ��
����ֵ: 
    0:      OK
    others: error
����:   void
���:   void
��ע:   void
ע��:   void
*/
static enChckRes UL865DisConnect(uint8_t *pucData, uint16_t usLen, uint8_t *pucState)
{
    enChckRes enRes = Packet;

    if (strstr((char *)pucData, "OK"))
    {
        *pucState   = T_CHECK_PPP;
    }
    enRes = Error;//�˴�����ΪError�Ա���ִ����ú�����״̬�����Լ�
    return enRes;
}

/*
����: UL865Sta
����:
    1.�����յ������ݼ�֮ǰ��ģ��״̬��ȷ����ǰUL865��״̬
����:   
    1. pucData: ָ������ģ������ݻ����ָ��
    2. usLen:   ����ģ������ݳ���
    3. pucState:   ָ��洢��ǰ��GPRS״̬�ı���(ucGprsState)��ָ��
����ֵ: 
    0:      OK
    Others: Error
����:   void
���:   void
��ע:   void
ע��:   
    1. �ú���ֻ����ģ���"CREG"�����Ӧ������
*/
static enChckRes UL865ChckSinal(uint8_t *pucData, uint16_t usLen, uint8_t *pucState)
{
    char *pcTmp = strstr((char *)pucData, "CSQ:");
    enChckRes enRes = Error;
    uint32_t ucTmp = 0;
    OS_ERR err = OS_ERR_NONE;

    if (NULL == pcTmp)
    {//�ְ�
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
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_DLY, &err);//�ȴ�500ms
    }
Return:
    return enRes;
}

/*
����: UL865Rest
����:
    1.����
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_DLY, &err);//�ȴ�5s
    SendAT(*pucState);
    ClrOnLineFLg();
    return;
}

/*
����: UL865Rest
����:
    1.GPRS���߼��,����⵽������ִ��SoftRest
����:   
    1. pucData:     ָ������ģ������ݻ����ָ��
    2. usLen:       ����ģ������ݳ���
    3. pucState:    ָ��洢��ǰ��GPRS״̬�ı���(ucGprsState)��ָ��
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
