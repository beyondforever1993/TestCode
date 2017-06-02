#ifndef __L2CAP_LAYER_H__
#define __L2CAP_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __L2CAP_LAYER_GLOBAL
#define L2CAP_EXT
#else
#define L2CAP_EXT extern
#endif

#define MAX_DATA_PACK_SIZE	320   //80
    typedef unsigned int CID;

    enum ePSM_T
    {	
        PSM_SDP					= 0x0001,			
        PSM_RFCOMM				= 0x0003,		
        PSM_TEL					= 0x0005,		
        PSM_TCS_BIN_CORDLESS	= 0x0007,
        PSM_BNEP				= 0x000F,
        PSM_HID_CONTROL			= 0x0011,
        PSM_HID_INTERRUPT		= 0x0013,
        PSM_AVCTP				= 0x0017,
        PSM_AVDTP				= 0x0019,
        PSM_UDI_C_PLANE			= 0x001D
    };
    typedef enum ePSM_T PSM_T;

    enum CommandCodes
    {
        CommandReject			= 0x01,
        ConnectionRequest		= 0x02,
        ConnectionResponse		= 0x03,
        ConfigureRequest		= 0x04,
        ConfigureResponse		= 0x05,
        DisconnectRequest		= 0x06,
        DisconnectResponse		= 0x07,
        EchoRequest				= 0x08,
        EchoResponse			= 0x09,
        InformationRequest		= 0x0A,
        InformationResponse		= 0x0B
    };

    enum RejectReason
    {
        UnknownCommand		= 0x0000,
        MTUExceeded			= 0x0001,
        InvalidCID			= 0x0002
    };

    enum ConnectReqResults
    {
        CON_REQ_SUCCESS,
        CON_REQ_PENDING,
        CON_REQ_REFUSED_PSM,
        CON_REQ_REFUSED_SEC,
        CON_REQ_TIMEOUT,
        CON_REQ_FAILED
    };

    enum ConnectReqStatus
    {
        NO_INFO			= 0x00,
        AUTHENT_PEND	= 0x01,
        AUTHER_PEND		= 0x02
    };

    enum ConnectResponse
    {
        CON_RSP_SUCCESS			= 0x0000,
        CON_RSP_PENDING			= 0x0001,
        CON_RSP_REFUSED_PSM		= 0x0002,
        CON_RSP_REFUSED_SEC		= 0x0003,
        CON_RSP_REFUSED_RES		= 0x0004
    };

    enum DisconnectReqResult
    {
        DISC_SUCCESS,
        DISC_TIMEOUT,
        DISC_FAILURE
    };

    enum ConfigResult {
        CFG_RESULT_SUCCESS		= 0x0000,	
        CFG_RESULT_FAILURE_1	= 0x0001,	
        CFG_RESULT_FAILURE_2	= 0x0002,	
        CFG_RESULT_FAILURE_3	= 0x0003	
    };

    enum ConfigParamType {
        CONFIG_PARAM_MTU = 0x01,
        CONFIG_PARAM_FLASH_TIMEOUT = 0x02,
        CONFIG_PARAM_QOS = 0x03
    };

    enum l2cap_cmd_stat {
        L2CAP_CMD_SUCCESS,
        L2CAP_CMD_FAILURE,
        L2CAP_CMD_PARAM_FAILURE,
        L2CAP_CMD_MEM_ALLOC_FAILURE
    };


    struct L2CAPPacket
    {
        unsigned int PacketLength;
        unsigned int ChannelID;
        unsigned char Info[MAX_DATA_PACK_SIZE];
    } ;


    /*
    struct hci_data_packet_st {
	unsigned int	hConnetion;
	unsigned char	PBflag;
	unsigned char	BCflag;
	unsigned int	body_len;
	unsigned char	body[MAX_DATA_PACK_SIZE];	
};
    */
    struct L2CAPCommand
    {
        unsigned char OpCode;
        unsigned char Identifier;
        unsigned int  Length;
        unsigned char Data[20];
    } ;

    L2CAP_EXT struct L2CAPCommand  L2CAP_Command;
    L2CAP_EXT struct L2CAPPacket   L2CAP_Packet;

    L2CAP_EXT unsigned char Hconnection[2];

#ifdef __L2CAP_LAYER_GLOBAL
    unsigned char L2CAP_Receive_SDP_Flag = 0;
    unsigned char L2CAP_Receive_RFCOMM_Flag = 0;
#else
    extern unsigned char L2CAP_Receive_SDP_Flag;
    extern unsigned char L2CAP_Receive_RFCOMM_Flag;
#endif


    L2CAP_EXT unsigned char L2CAP_Signal_Connect_Req_Handle(unsigned char *L2CAP_Data );
    L2CAP_EXT unsigned char L2CAP_Signal_Config_Req_Handle(unsigned char *L2CAP_Data);
    L2CAP_EXT unsigned char L2CAP_Signal_Disconnect_Req_Handle(unsigned char *L2CAP_Data);
    L2CAP_EXT unsigned char L2CAP_Data_Handler(unsigned char *L2CAP_Data);
    //unsigned char SendL2CAPConnectReq(unsigned char Id, unsigned int psmVal, CID localCID);
    L2CAP_EXT unsigned char SendL2CAPConnectRsp(unsigned char Id, CID localCID, CID remoteCID, unsigned int resp, unsigned int stat);
    L2CAP_EXT unsigned char SendL2CAPConfigureReq(unsigned char Id, CID remoteCID, unsigned int nCFlag,unsigned char nNumOfParams,unsigned char *pCfgParamsArray);
    L2CAP_EXT unsigned char SendL2CAPConfigureRsp(unsigned char Id, CID remoteCID, unsigned int nCFlag, unsigned int nResult );
    //unsigned char SendL2CAPDisconnectReq(unsigned char Id, CID remoteCID, CID localCID);
    L2CAP_EXT unsigned char SendL2CAPDisconnectRsp(unsigned char Id, CID localCID, CID remoteCID);
    L2CAP_EXT unsigned char L2CAP_Packets_Sender();//(struct L2CAPPacket L2CAP_Packet );
    L2CAP_EXT unsigned char Huace_BT_Data_Sender(unsigned char *databuf,unsigned int datalength);

    L2CAP_EXT unsigned char L2CAP_Signal_ConnectionResponse_Handle(unsigned char *L2CAP_Data);

#ifdef __cplusplus
}
#endif

#endif