#ifndef __HCI_LAYER_H__
#define __HCI_LAYER_H__

#ifdef __HCI_LAYER_GLOBAL
#define HCILAYER_EXT
#else
#define HCILAYER_EXT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __HCI_LAYER_GLOBAL
    unsigned char BT_TYPE = 1;
    unsigned char Enable_Auto_Connetct_Flag = 0;
    unsigned int HC_ACL_Data_Length = 64;
    unsigned int Total_Num_ACL_Data_Packet = 5;
#else
    extern unsigned char BT_TYPE;
    extern unsigned char Enable_Auto_Connetct_Flag;
    extern unsigned char BT_HCI_First_Connect_Flag;
    extern unsigned int HC_ACL_Data_Length;
    extern unsigned int Total_Num_ACL_Data_Packet;
#endif

#define HP_BT 0
#define SOCKET_BT 1
#define RECON_BT 2
    //#define BT_TYPE  SOCKET_BT

    /*---------------------------------------------------------------*/
    enum PacketType
    {
        COMMAND		= 0x01,
        DATA		= 0x02,
        VOICE		= 0x03,
        EVENT		= 0x04
    };
    /*---------------------------------------------------------------*/
    enum Local_BT_Status
    {
        LOCAL_BT_IDLE = 0x01,
        LOCAL_BT_BUSY,
        LOCAL_BT_EVENT_RECEIVED,
        LOCAL_BT_DATA_RECEIVED
    };

    /*---------------------------------------------------------------*/
    enum HCICommands {
        /******Link Control Commands**************************************/
        HCI_INQUIRY										= 0x0401,
        HCI_INQUIRY_CANCEL,
        HCI_PERIODIC_INQUIRY_MODE,
        HCI_EXIT_PERIODIC_INQUIRY_MODE,
        HCI_CREATE_CONNECTION							= 0x0405,
        HCI_DISCONNECT									= 0x0406,
        HCI_ADD_SCO_CONNECTION,
        HCI_ACCEPT_CONNECTION_REQUEST					= 0x0409,
        HCI_REJECT_CONNECTION_REQUEST,
        HCI_LINK_KEY_REQUEST_REPLY,
        HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY,
        HCI_PIN_CODE_REQUEST_REPLY,
        HCI_PIN_CODE_REQUEST_NEGATIVE_REPLY,
        HCI_CHANGE_CONNECTION_PACKET_TYPE,
        HCI_AUTHENTICATION_REQUEST						= 0x0411,
        HCI_SET_CONNECTION_ENCRYPTION					= 0x0413,
        HCI_CHANGE_CONNECTION_LINK_KEY					= 0x0415,
        HCI_MASTER_LINK_KEY								= 0x0417,
        HCI_REMOTE_NAME_REQUEST							= 0x0419,
        HCI_READ_REMOTE_SUPPORTED_FEATURES				= 0x041B,
        HCI_READ_REMOTE_VERSION_INFORMATION				= 0x041D,
        HCI_READ_CLOCK_OFFSET							= 0x041F,
        /******Link Policy Commands**************************************/
        HCI_HOLD_MODE = 0x0801,
        HCI_SNIFF_MODE = 0x0803,
        HCI_EXIT_SNIFF_MODE,
        HCI_PARK_MODE,
        HCI_EXIT_PARK_MODE,
        HCI_QOS_SETUP,
        HCI_ROLE_DISCOVERY = 0x0809,
        HCI_SWITCH_ROLE = 0x080B,
        HCI_READ_LINK_POLICY_SETTINGS,
        HCI_WRITE_LINK_POLICY_SETTINGS,
        /******Host Controller & Baseband Commands *********************/
        HCI_SET_EVENT_MASK = 0x0C01,
        HCI_RESET = 0x0C03,
        HCI_SET_EVENT_FILTER = 0x0C05,
        HCI_FLUSH = 0x0C08,
        HCI_READ_PIN_TYPE,
        HCI_WRITE_PIN_TYPE,
        HCI_CREATE_NEW_UNIT_KEY,
        HCI_READ_STORED_LINK_KEY = 0x0C0D,
        HCI_WRITE_STORED_LINK_KEY = 0x0C11,
        HCI_DELETE_STORED_LINK_KEY,
        HCI_CHANGE_LOCAL_NAME,
        HCI_READ_LOCAL_NAME,
        HCI_READ_CONNECTION_ACCEPT_TIMEOUT,
        HCI_WRITE_CONNECTION_ACCEPT_TIMEOUT,
        HCI_READ_PAGE_TIMEOUT,
        HCI_WRITE_PAGE_TIMEOUT,
        HCI_READ_SCAN_ENABLE,
        HCI_WRITE_SCAN_ENABLE = 0x0C1A,
        HCI_READ_PAGE_SCAN_ACTIVITY,
        HCI_WRITE_PAGE_SCAN_ACTIVITY,
        HCI_READ_INQUIRY_SCAN_ACTIVITY,
        HCI_WRITE_INQUIRY_SCAN_ACTIVITY,
        HCI_READ_AUTHENTICATION_ENABLE,
        HCI_WRITE_AUTHENTICATION_ENABLE,
        HCI_READ_ENCRYPTION_MODE,	
        HCI_WRITE_ENCRYPTION_MODE,
        HCI_READ_CLASS_OF_DEVICE,
        HCI_WRITE_CLASS_OF_DEVICE,
        HCI_READ_VOICE_SETTING,
        HCI_WRITE_VOICE_SETTING,
        HCI_READ_AUTOMATIC_FLUSH_TIMEOUT,
        HCI_WRITE_AUTOMATIC_FLUSH_TIMEOUT,
        HCI_READ_NUM_BROADCAST_RETRANSMISSIONS,
        HCI_WRITE_NUM_BROADCAST_RETRANSMISSIONS,
        HCI_READ_HOLD_MODE_ACTIVITY,
        HCI_WRITE_HOLD_MODE_ACTIVITY,
        HCI_READ_TRANSMIT_POWER_LEVEL,
        HCI_READ_SCO_FLOW_CONTROL_ENABLE,
        HCI_WRITE_SCO_FLOW_CONTROL_ENABLE,
        HCI_SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL = 0x0C31,
        HCI_HOST_BUFFER_SIZE = 0x0C33,
        HCI_HOST_NUMBER_OF_COMPLETED_PACKETS = 0x0C35,
        HCI_READ_LINK_SUPERVISION_TIMEOUT,
        HCI_WRITE_LINK_SUPERVISION_TIMEOUT,
        HCI_READ_NUMBER_OF_SUPPORTED_IAC,
        HCI_READ_CURRENT_IAC_LAP,
        HCI_WRITE_CURRENT_IAC_LAP,
        HCI_READ_PAGE_SCAN_PERIOD_MODE,
        HCI_WRITE_PAGE_SCAN_PERIOD_MODE,
        HCI_READ_PAGE_SCAN_MODE,
        HCI_WRITE_PAGE_SCAN_MODE,
        /******Informational Commands*******************************/
        HCI_READ_LOCAL_VERSION_INFORMATION = 0x1001,
        HCI_READ_LOCAL_SUPPORTED_FEATURES = 0x1003,
        HCI_READ_BUFFER_SIZE = 0x1005,
        HCI_READ_COUNTRY_CODE = 0x1007,
        HCI_READ_BD_ADDR = 0x1009,
        /******Status Commands***************************************/
        HCI_READ_FAILED_CONTACT_COUNTER = 0x1401,
        HCI_RESET_FAILED_CONTACT_COUNTER,
        HCI_GET_LINK_QUALITY,
        HCI_READ_RSSI = 0x1405,
        /******Testing Commands**************************************/
        HCI_READ_LOOPBACK_MODE = 0x1801,
        HCI_WRITE_LOOPBACK_MODE,
        HCI_ENABLE_DEVICE_UNDER_TEST_MODE,
        HCI_USER_COMMAND = 0xF401		
    };//end_of_Registered_Commands
    /*---------------------------------------------------------------*/
    enum Events_Codes {
        DUMMY_EVENT	= 0x0,
        INQUIRY_COMPLETE_EVENT = 0x01,
        INQUIRY_RESULT_EVENT,
        CONNECTION_COMPLETE_EVENT,
        CONNECTION_REQUEST_EVENT,
        DISCONNECTION_COMPLETE_EVENT,
        AUTHENTICATION_COMPLETE_EVENT,
        REMOTE_NAME_REQUEST_COMPLETE_EVENT,
        ENCRYPTION_CHANGE_EVENT,
        CHANGE_CONNECTION_LINK_KEY_COMPLETE_EVENT,
        MASTER_LINK_KEY_COMPLETE_EVENT,
        READ_REMOTE_SUPPORTED_FEATURES_COMPLETE_EVENT,
        READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVENT,
        QOS_SETUP_COMPLETE_EVENT,
        COMMAND_COMPLETE_EVENT,
        COMMAND_STATUS_EVENT,
        HARDWARE_ERROR_EVENT,
        FLUSH_OCCURED_EVENT,
        ROLE_CHANGE_EVENT,
        NUMBER_OF_COMPLETED_PACKETS_EVENT,
        MODE_CHANGE_EVENT,
        RETURN_LINK_KEYS_EVENT,
        PIN_CODE_REQUEST_EVENT,
        LINK_KEY_REQUEST_EVENT,
        LINK_KEY_NOTIFICATION_EVENT,
        LOOPBACK_COMMAND_EVENT,
        DATA_BUFFER_OVERFLOW_EVENT,
        MAX_SLOTS_CHANGE_EVENT,
        READ_CLOCK_OFFSET_COMPLETE_EVENT,
        CONNECTION_PACKET_TYPE_CHANGED_EVENT,
        QOS_VIOLATION_EVENT,
        PAGE_SCAN_MODE_CHANGE_EVENT,
        PAGE_SCAN_REPETITION_MODE_CHANGE_EVENT
    }; //end_of_Events_Codes
    /*---------------------------------------------------------------*/
    typedef struct Inquiry_Result_Event_Data
    {
        unsigned char	Num_Responses;					
        unsigned char	BD_ADDR[6];						
        unsigned char	Page_Scan_Repetition_Mode_Vect;	
        unsigned char	Page_Scan_Period_Mode_Vect;		
        unsigned char	Page_Scan_Mode_Vect;			
        unsigned char	Class_of_Device_Vect[3];			
        unsigned char	Clock_Offset_Vect[2];				
    } Inquiry_Result_Event_Data_T;
    /*---------------------------------------------------------------*/
    typedef struct Connection_Request_Event_Data
    {
        unsigned char	BD_ADDR[6];			
        unsigned char	Class_of_Device_Vect[3];	
        unsigned char	LinkType;				
    } St_ConRequest;
    /*---------------------------------------------------------------*/
    typedef struct Connection_Complete_Event_Data
    {
        unsigned char	Status;		
        unsigned char	hConnection[2];	
        unsigned char	BD_ADDR[6];	
        unsigned char	LinkType;	
        unsigned char	EncryptionMode;	
    } Connection_Complete_Event_Data_T;

    typedef struct Remote_BT_Information
    {
        unsigned char   BD_ADDR[6];
        unsigned char   hConnection[2];
        unsigned char	Page_Scan_Repetition_Mode_Vect;	
        unsigned char	Page_Scan_Period_Mode_Vect;		
        unsigned char	Page_Scan_Mode_Vect;		
        unsigned char	Class_of_Device_Vect[3];
        unsigned char   Link_Type;
        unsigned char   Link_Key[16];
        unsigned int    RemoteCID;
        unsigned char   Channel;
    } Remote_BT_Information_T;

    typedef struct Local_BT_Information
    {
        unsigned char   BD_ADDR[6];
        unsigned char   hConnection[2];
        unsigned int    LocalCID;	
        unsigned char   Protocol_Type;
        unsigned char   Link_Type;
        unsigned char   Channel;
    } Local_BT_Information_T;

#ifdef __HCI_LAYER_GLOBAL
    unsigned char HCI_Receive_L2CAP_Flag = 0;
#else
    extern unsigned char HCI_Receive_L2CAP_Flag;
#endif

    HCILAYER_EXT struct Remote_BT_Information  Remote_BlueTooth_Information[2];
    HCILAYER_EXT struct Local_BT_Information   Local_BlueTooth_Information;

    /*---------------------------------------------------------------*/
    //unsigned char HCI_Test(void);

    HCILAYER_EXT unsigned char BT_Init(void);

    HCILAYER_EXT unsigned char HCI_Reset(void);
    HCILAYER_EXT unsigned char HCI_Set_Event_Filter(void);
    unsigned char HCI_Change_Local_Name(unsigned char* buff,unsigned char length);
    HCILAYER_EXT unsigned char HCI_Write_Scan_Enable(void);
    //HCILAYER_EXT unsigned char BT_Wait_ACK(void);
    HCILAYER_EXT void BT_Link_Detect_Handler(void);
    HCILAYER_EXT unsigned char HCI_Write_Scan_Disable(void);
    HCILAYER_EXT void  HCI_Write_Link_Policy(void);
    HCILAYER_EXT void HCI_Change_Packet_Type(void);
    HCILAYER_EXT void HCI_Switch_Role(void);
    //unsigned char HCI_Write_Authentication_Enable(void);
    //unsigned char HCI_Inquiry(void);
    //unsigned char HCI_Create_Connection(unsigned char * address, unsigned char nPageScanRepMode,unsigned char nPageScanMode, unsigned char *nClkOffset);
    //unsigned char HCI_Disconnect(unsigned char *hConnection, unsigned char Reason);
    //unsigned char HCI_Accept_Connection_Request(void);
    HCILAYER_EXT unsigned char HCI_Send_ACL_Data(unsigned char *databuf,unsigned int datalength,unsigned char *hConnection);
    //unsigned char HCI_Request_Remote_Name(unsigned char *BD_Addr);
    HCILAYER_EXT unsigned char HCI_Data_Packet_Handler(void);

    //HCILAYER_EXT unsigned char HCI_Role_Discovery(void);

    /*---------------------------------------------------------------*/

    //unsigned char HCI_Event_Inquiry_complete(unsigned char *Event_Para);
    //HCILAYER_EXT unsigned char HCI_Event_Inquiry_result(unsigned char*Event_Para);
    HCILAYER_EXT unsigned char HCI_Event_Connection_complete(unsigned char*Event_Para);
    //unsigned char HCI_Event_Connection_request(unsigned char*Event_Para);
    HCILAYER_EXT void HCI_Pin_Request_Reply_Nagative(unsigned char* Bt_Address);
    HCILAYER_EXT void HCI_Pin_Request_Reply(unsigned char* Bt_Address);
    HCILAYER_EXT void HCI_LinkKey_Notice_Reply(unsigned char* LinkKey);
    HCILAYER_EXT void HCI_LinkKey_Request_Reply(unsigned char* Bt_Address);

    HCILAYER_EXT unsigned char HCI_Read_Local_Name(void);
    HCILAYER_EXT void HCI_Exit_Periodic_Inquiry_Mode(void);
    HCILAYER_EXT void HCI_Change_BT_Name(void);
    HCILAYER_EXT void HCI_Read_Clock(void);
    HCILAYER_EXT void HCI_Read_Remote_Version(void);
    HCILAYER_EXT void HCI_Write_Link_Timeout(void);
    HCILAYER_EXT void HCI_Write_Class_Type2(void);
    HCILAYER_EXT void HCI_Write_Stored_Link_Key(void);
    HCILAYER_EXT void HCI_Read_Stored_Link_Key(void);
    HCILAYER_EXT void HCI_Read_Link_Quality(void);
    HCILAYER_EXT void HCI_Read_RSSI(void);
    HCILAYER_EXT unsigned char HCI_Event_Listen(unsigned char *HCI_Event_Data);
    HCILAYER_EXT unsigned char HCI_ACL_Data_Receive_Handler(void);
    HCILAYER_EXT unsigned char HCI_Disable_Auto_Connect(void);
    HCILAYER_EXT void Data_Copy_EX(unsigned char *Source)   ;

#ifdef __cplusplus
}
#endif

#endif