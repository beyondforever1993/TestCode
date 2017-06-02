#ifndef  GLOBAL_VARIABLES_H
#define  GLOBAL_VARIABLES_H

#define RELEASE  0
#define DEBUG    1

#define PROGRAMME_MODE      RELEASE

//接收机模式
#define STATIC_MODE 0
#define RTK_MODE 1

//模块类型
#define NONE        0x00
#define GL865       0x01        // edit 2013.07.11
#define GE910       0x02        // edit 2013.07.11
#define HE910       0x03
#define GL868_DUAL  0x04
#define Q26ELITE    0x05
#define Q2687       0x06
#define CE910       0x07       // edit 2013.07.11
#define DE910       0x08       // edit 2013.07.11
#define UE910       0x09       //edit  2014.01.26
#define LE910       0x0A       //edit  2014.01.26
#define UL865		0x0B

//AT命令 重试最大次数
#define MAX_REPEAT_TIMES 2

//SIM卡查询次数
#define CHECK_SIMCARD_TIMES 2

//信号强度检测
#define MIN_RSSI_VALUE 10
#define CHECK_ERROR_SIGNAL_TIMES 50
#define CHECK_CORRECT_SIGNAL_TIMES 4

//CGREG错误检测
#define CHECK_TIME 50               //edit 2013.04.30

//模块数据包缓冲区大小
#define Module_Data_Buffer_Size 2048 //heyunchun edit 2013.08.23 //1024
#define INCREASE_MOUDLE_DATA_POINTER(p) { (p)++; (p) %= Module_Data_Buffer_Size; }

//APIS命令数据包大小
#define Apis_Command_Len     550         //edit 2013.12.17

//接收机属性
#define BASE     0
#define ROVER    1

//CORS登录模式
#define MANUL_MODE 0
#define AUTO_MODE 1

//CORS GPGGA发送间隔
#define GPGGA_SEND_INTERVAL 200 //5s
//#define GPGGA_SEND_INTERVAL 40    //1s    edit2013.05.07

//EEPROM数据存储页面和地址 33页 0x00开始
#define PARA_PAGE_ADDR 0x21
#define PARA_PAGE_OFFSET 0x00

//EEPROM数据存储参数判断
#define WRITE_NULL              0
#define WRITE_APN_SERV          1
#define WRITE_DIAL_USER_PASSW   2
#define WRITE_REMOTE_IP_PORT    3
#define WRITE_REMOTE_ADDRESS    4
#define WRITE_BINDING_ID        5
#define WRITE_CORS_INFOR        6

//heyunchun edit 2013.08.20
// 工作类型定义
#define GPRS_MODE              0    // GPRS 模式
#define HUACE_RADIO            1    // 华测电台
#define PACIFIC_CREST_GMSK     5    // 透明传输
#define TRIMTALK_GMSK          6    // TT450
#define SATEL_3AS              8    // Satel协议
#define PACIFIC_CREST_4FSK     9    // 4FSK

/*-------------------------------- Huace Command -----------------------------------------*/
//"VL"-CORS源列表数据
//"VI"-回复命令

#define  VL 175 + 9
#define  VI 175 + 6
#define  VM 175 + 10  //edit 2014.06.05 add by xxw 20140801

//系统设置结构体
struct SYS_Config
{
	unsigned char Remote_IP[4];//IP 地址
	unsigned char Remote_Port[2];//端口号
	unsigned char Protocol_Type[10];//协议类型
	unsigned char Dial_Num[16];//拨号号码
	unsigned char APN_Num[32];//APN接入点
	unsigned char Base_OR_Rover[1];//基准站移动站
	unsigned char Binding_ID[32];//绑定基准站ID号
	unsigned char Pre_Dial_Username_Password[31];//拨号用户名密码  不再使用
	unsigned char Remote_Address[32];//网址
	unsigned char Radio_Frequence[2];//电台频率
	unsigned char Dial_Mode;//拨号协议模式
	unsigned char Work_Mode;//工作模式
	unsigned char Radio_Power;//电台功率
	unsigned char Radio_Baud;//电台空中波特率
	unsigned char Radio_Sensitivity;//电台灵敏度 //edit 2013.03.12
	unsigned char Radio_FEC;//电台FEC校验 //edit 2013.03.20
	unsigned char Radio_Chanel;//电台信道值 //edit 2013.03.20
	unsigned char Radio_CH_Frequence[32];//电台信道值 //edit 2013.03.20//edit 2013.03.21
	unsigned char Radio_Channel_Spacing; //步进值  0-12.5KHZ  1-20KHZ  2-25KHZ   heyunchun edit 2013.08.20
	unsigned char Radio_CallSign_State;       //ON or OFF
	unsigned char Radio_CallSign_Interval;    // 1 - 30 min
	unsigned char Radio_CallSign_Message[17]; //max length is 16
	unsigned char Dial_Username_Password[50]; //新拨号用户名密码
	
};


//CORS参数结构体
struct CORS_Pcb
{
	unsigned char Sourcelist[32];//源列表
	unsigned char Username[32];//CORS用户名
	unsigned char Password[32];//CORS密码
    unsigned char Data_Format;//数据格式

	unsigned char CORS_Log_Mode;//CORS登录模式
	
	unsigned char GPGGA[128];//GPGGA数据
	
	unsigned char Manul_Log_Data[256];//手动CORS登录数据包
	unsigned char Manul_Log_Data_Length;//手动CORS登录数据包长度

    unsigned char Auto_Log_Data[256];//自动CORS登录数据包
	unsigned char Auto_Log_Data_Length;//自动CORS登录数据包长度

	unsigned char Base64_Copy_Buffer[256];//Base 64加密数据缓冲区
	
	unsigned char CORS_Log_Data_Send_Flag;//手动CORS登录数据包发送标志
    unsigned char Repeat_Send_Cnt;//CORS登录数据包发送计数器
	unsigned char GPGGA_Valid_Flag;//可发送GPGGA数据包标志
    unsigned char Get_VLData_Flag;//收到VL数据包标志 手动登录
    unsigned char Click_Log_Botton_Flag;//点击登录按钮标志 手动登录
	unsigned char TCP_Connected_Flag;//TCP连接成功标志
};

//APIS参数结构体
struct APIS_Pcb
{
	unsigned char bApisDataBuff[1024];//APIS数据缓冲区
	unsigned char Apis_Status;//APIS连接状态
	unsigned char Apis_Connect_Cnt;//APIS连接次数
	unsigned char Apis_Reconnect_Failure_Flag;//APIS重连失败标志
    unsigned char APIS_Decode_Flag;//APIS差分数据转义标志
    unsigned char Apis_No_Beat_Cnt;//APIS无心跳包重试次数  //edit 2013.01.23
};

//定时器计时器
struct Timer
{
    unsigned short Wait_Time_Cnt; //AT命令等待时间
    unsigned short TimeOut_Cnt; //AT命令超时计时器
    unsigned char Msg_Data_Timeout;//华测数据组包计时器
	unsigned short Signal_Weak_Timeout;	//信号强度低 计时器
    unsigned char Set_Work_Mode_Timeout;	 //设置工作模式 计时器
    unsigned short Set_Radio_Freq_Timeout;	 //设置电台频率  计时器
    unsigned char Set_Dial_Parameter_Timeout;//设置拨号连接参数 计时器
    unsigned char Set_Protocol_Parameter_Timeout;//设置APIS和CORS参数 计时器	
    unsigned char Request_GPGGA_Timeout; //请求GPGGA数据 计数器
    unsigned short No_Diff_Data_Timeout;//无差分数据计时器
    unsigned char GPGGA_Timeout;	//心跳包计数器
    unsigned short CORS_No_ACK_Timeout; //CORS登录数据包响应计时器
    unsigned short Apis_Beat;	 //心跳包计时器
    unsigned short Simcard_Timeout;	 //SIMCARD检测计时器
};

//edit 2012.08.20
/*-------------------------------- 模块Check 状态 -------------------------*/
//模块工作状态
typedef enum
{
	START,CHECK_MODULETYPE_WAVECOM,CHECK_MODULETYPE_TELIT,CHECK_DONE
} Common_State;

//edit 2012.08.20
//START,CHECK_MODULETYPE,
/*-------------------------------- GPRS Wavecom Q2687模块 -------------------------*/
//模块工作状态
typedef enum
{
	CHECK_SIMCARD,START_WOPEN,SET_CGCLASS,FLOW_CTRL_CLOSE,START_WIPCFG,
	START_WIPBR,SET_APN_SERV,CHECK_CREG,CHECK_SIGNAL,SET_CGATT,SET_DIAL_UN,SET_DIAL_PW,START_PPP,//edit 2012.10.10
	UDP_CLOSE,UDP_SET_IP_PORT,APIS_CONNECT,
	TCP_CLOSE,TCP_DELAY,TCP_SET_IP_PORT,CORS_CONNECT,
    APIS_ANALYSIS,CORS_ANALYSIS,
    DISCONNECT,CLOSE_WIPCFG,SET_WMBS_2
} Q26_State;
//edit 2012.08.20
//C_START,C_CHECK_MODULETYPE,
/*-------------------------------- CDMA Wavecom Q26Elite模块 ----------------------------------------------------*/
//模块工作状态
typedef enum
{
	C_CHECK_SIMCARD,C_CHECK_SIGNAL,
	C_FLOW_CTRL_CLOSE,C_START_WIPCFG,C_START_SIMPLE_IP,C_SET_DIAL_UN,C_SET_DIAL_PW,C_START_PPP,//edit 2012.10.10
	C_UDP_CLOSE,C_UDP_SET_IP_PORT,C_APIS_CONNECT,
	C_TCP_CLOSE,C_TCP_DELAY,C_TCP_SET_IP_PORT,C_CORS_CONNECT,
    C_APIS_ANALYSIS,C_CORS_ANALYSIS,
    C_DISCONNECT,C_CLOSE_WIPCFG
}  Q26Elite_State;

//edit 2012.08.20
//T_START,T_CHECK_MODULETYPE,
/*-------------------------------- GPRS Telit HE910 GL868-DUAL模块-------------------------*/
/*---------------------------------------- 3G Telit HE910模块 --------------------------------------------------*/
//模块工作状态
typedef enum
{
	T_CHECK_SIMCARD,
    T_FLOW_CTRL_CLOSE,
    T_SET_CGCLASS,
    T_SET_APN_SERV,
    SET_SKIPESC,
	T_CHECK_CREG,
	T_CHECK_SIGNAL,
	T_START_REGISTER,
	T_SET_DIAL_UN,
	T_SET_DIAL_PW,
	T_START_PPP,
	T_SET_SOCKET_DELAY,
	T_SOCKET_CLOSE,
	T_UDP_APIS_CONNECT,
	T_TCP_CORS_CONNECT,
    T_APIS_ANALYSIS,
    T_CORS_ANALYSIS,
    T_DISCONNECT,
    T_CHECK_PPP
}  Telit_State;


extern struct SYS_Config SYS;

extern struct CORS_Pcb CORS;

extern struct APIS_Pcb APIS;

extern struct Timer Timer_Flag;

extern Common_State Common_Connection_State; //edit 2012.08.20
extern Q26_State Q26_Connection_State;
extern Q26Elite_State Q26Elite_Connection_State;
extern Telit_State Telit_Connection_State;

/*--------------------------------AT Commands--------------------------------*/
//模块类型检测AT命令
extern unsigned char  * Common_ATCmd[3];//edit 2012.08.20

//Wavecom Q2687 AT  命令

extern unsigned char * Q26_ATCmd[20];//edit 2012.08.20 //edit 2013.05.05
//Wavecom Q26Elite AT  命令

extern  unsigned char * Q26EL_ATCmd[15];//edit 2012.08.20 //edit 2012.10.10

//Telit GL868-DUAL HE910 AT 命令

extern unsigned char * T_ATCmd[15];//edit 2012.08.20

//Common AT  命令
extern  unsigned char  _3plus[4];
extern  unsigned char  AT[4];
//Wavecom common AT 命令
extern  unsigned char AT_WIPCFG_0[14];
extern  unsigned char AT_CFUN_1[11];
extern  unsigned char AT_CFUN_0[11]; //edit 2013.04.28
extern  unsigned char AT_CFUN_10[13];
extern  unsigned char WMBS_2[13];  //edit 2013.04.28
//extern  unsigned char AT_GET_PPP[11];
//extern  unsigned char AT_REPEAT[5];	

//Telit common AT 命令
extern unsigned char AT_CHECK_PPP[11];
extern unsigned char AT_CDMA_PPP[14];
/*--------------------------------AT Commands ACK--------------------------------*/
//模块信息
extern unsigned char  * Module_Infor[12]; //edit 2012.08.16

//Wavecom Q26Elite AT 响应
extern  unsigned char  _WIND7[8];

//Wavecom Common AT 响应	
extern  unsigned char  _CSQ[6];
extern  unsigned char  _CREG[7];   //edit 2013.04.15
extern  unsigned char  OK[3];
extern  unsigned char  _ERROR[6];
extern  unsigned char  WIPREADY[10];
extern  unsigned char  CONNECT[8];
extern  unsigned char  _WIPPEERCLOSE[14];
extern  unsigned char  _SHUTDOWN[9];

extern  unsigned char  _WIPBR_ACTIVE[12];
extern  unsigned char  _WIPBR_DEACTIVE[12];

//Telit Common AT 响应
extern unsigned char  _PPP_ACTIVE[9];
extern unsigned char  _PPP_DEACTIVE[9];

//Telit UC864 AT 响应
//extern  unsigned char  PNST[9];

//Wavecom and Telit Common AT 响应	
extern  unsigned char  NO_CARRIER[11];
extern  unsigned char  READY[6];

//华测数据包内容
extern unsigned char _GPGGA[7];
extern unsigned char _GPGSV[7];

/*--------------------------------公用-----------------------------------------*/

//调试使用
extern unsigned char GPGGA[127];
//模块串口数据包缓冲区
extern unsigned  char  Module_Data_Buffer[Module_Data_Buffer_Size];
extern unsigned  short Module_Data_WrSp;
extern unsigned  short Module_Data_RdSp;

//模块类型
extern unsigned char Module_Type;

//edit 2013.02.22 del
//接收机模式
//extern unsigned char g_nReceiverMode;

//缺省参数标志
extern unsigned char  Default_Parameter_Flag[1];

//模块状态
extern unsigned char  Module_Status[4];

//固件版本
extern unsigned char  Firm[6];

//工作模式切换标志
extern unsigned char  Work_Mode_Change_Flag;

//设置包含APN 服务商号码 拨号用户名密码 等拨号连接参数更改标志
extern unsigned char  Dial_Parameter_Change_Flag;

//设置IP端口号 通信协议 基准站移动站模式 网址 源列表用户名密码等通讯协议参数 更改标志
extern unsigned char  Protocol_Parameter_Change_Flag;

//信号强度低标志
extern unsigned char Signal_Weak_Flag;

//SIMCARD检测标志
extern unsigned char Simcard_Check_Flag;

//设置网址和端口号以及协议标志
extern unsigned char Set_Net_Address_Flag;
extern unsigned char Current_State;//上线连网状态
extern unsigned short Err_Code;//错误代码

//通讯模块RST复位标志
extern unsigned char g_bModuleRestartFlag;

//通讯软件复位命令标志
extern unsigned char g_bResetCmdFlag;//edit 2012.09.19
extern unsigned char Auto_Update_Flag; //edit 2012.12.07
//收发电台复位标志
extern unsigned char Reset_TRRadio_Flag;

//通讯模块初始化完成标志
extern unsigned char g_bMoudule_Initialized_Flag;

//华测设置命令包结束标志
extern unsigned char Msg_Set_Receive_End_Flag;

//华测数据包结束标志
extern unsigned char Msg_Data_Receive_End_Flag;

//模块数据输出标志
extern unsigned char g_bPrintDataFlag;

//AT 命令发送计数器
extern unsigned char  AT_Repeat_Cnt;

//AT_WIPCFG0命令发送计数器
extern unsigned char AT_WIPCFG0_Repeat_Cnt;

//AT上线重连标志
extern unsigned char  Reconnect_Flag;

//断开网络按钮标志
extern unsigned char  Disconnect_Click_Flag;

//获取源列表按钮标志
extern unsigned char Get_Sourcelist_Flag;

//数据断线后重连标志
extern unsigned char Service_Relog_Flag;

//指示当前的AT指令数
extern unsigned char   AT_Cmd_Index;

//统计AT指令总数
extern unsigned char   All_AT_Cmd;

//VS命令源地址
extern unsigned char  VSCommand_Source;

//VL命令源地址
extern unsigned char  VLCommand_Source;

//VM命令源地址
extern unsigned char  VMCommand_Source;  //edit 2014.05.29 add by xxw 20140801

//拨号用户名和密码
extern unsigned char  UserName[33];
extern unsigned char  PassWord[17];

//仪器ID
extern unsigned char SYS_ID_PW_Code[22];

/*-------------------------------- APIS -----------------------------------------*/

//APIS 工作域
extern unsigned char Huace[9];

//APIS移动站绑定的基准站ID
extern unsigned char BindID[7];

//APIS 密码
extern unsigned char pWord[17];

//APIS GPRS动态IP地址
extern unsigned char GPRS_Dynamic_IP_Address[5];

/*--------------------------------VRS or CORS-----------------------------------------*/
// Base 64 加密 The 7-bit alphabet used to encode binary information
extern  unsigned char  codes[65];

// CORS 登录成功
extern  unsigned char  Log_CORS_OK[19];
extern  unsigned char  CY_200_OK[10];

//extern  unsigned char  ICY_200_OK[7];
// CORS 登录错误判断及其错误
extern  unsigned char  Unauthorized[17];
extern  unsigned char  Source[19];
extern  unsigned char  ENDSource[15];

//Cors Error1-IP地址和端口号错误
//Cors_Error2-用户名和密码错误or 已经被使用
//Cors_Error3-源列表错误
extern  unsigned char  Cors_Error1[57];
extern  unsigned char  Cors_Error2[74];
extern  unsigned char  Cors_Error3[36];

//CORS 信息数据
extern  unsigned char  Cors_Infor1[6];
extern  unsigned char  Cors_Infor2[12];
extern  unsigned char  Cors_Infor3[44];
extern  unsigned char  Cors_Infor4[14];
extern  unsigned char  Cors_Infor5[20];
extern  unsigned char  Cors_Infor6[22];

/*--------------------------------打印输出信息-----------------------------------------*/
extern unsigned char  Initialized[13];
extern unsigned char  SIMCARD_OK[16];
extern unsigned char  SIGNAL_OK[19];
extern unsigned char  PPP_OK[11];
extern unsigned char  PPP_FAIL[11];
extern unsigned char  APIS_OK[21];
extern unsigned char  CORS_OK[21];

#endif