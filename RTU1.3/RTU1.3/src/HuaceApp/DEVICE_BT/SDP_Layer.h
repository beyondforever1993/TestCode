#ifndef __SDP_LAYER_H
#define __SDP_LAYER_H

#ifdef __SDP_LAYER_GLOBAL
#define SDP_EXT
#else
#define SDP_EXT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif


    /***************PDU ID**************************************/
#define SDP_PDU_UNKNOWN_ID					0x00
#define SDP_PDU_ERROR_RSP_ID				0x01
#define SDP_PDU_SERVICE_SEARCH_REQ_ID 		0x02
#define SDP_PDU_SERVICE_SEARCH_RSP_ID		0x03
#define SDP_PDU_SERVICE_ATTR_REQ_ID			0x04
#define SDP_PDU_SERVICE_ATTR_RSP_ID			0x05
#define SDP_PDU_SERVICE_SEARCH_ATTR_REQ_ID	0x06
#define SDP_PDU_SERVICE_SEARCH_ATTR_RSP_ID	0x07
    /******************ATTRIBUTE ID*****************************/
#define SDP_ATTR_RECORD_HANDLE					0x0000	
#define SDP_ATTR_SERVICE_CLASS_ID_LIST			0x0001	
#define SDP_ATTR_RECORD_STATE					0x0002		
#define SDP_ATTR_SERVICE_ID						0x0003	
#define SDP_ATTR_PROTOCOL_DESCRIPTOR_LIST		0x0004		
#define SDP_ATTR_BROWSE_GROUP_LIST				0x0005	
#define SDP_ATTR_LANG_BASE_ATTR_ID_LIST			0x0006	
#define SDP_ATTR_SERVICE_INFO_TIME_TO_LIVE		0x0007	
#define SDP_ATTR_SERVICE_AVAILABILITY			0x0008	
#define SDP_ATTR_PROFILE_DESCRIPTOR_LIST		0x0009	
#define SDP_ATTR_SERVICE_DOC_URL				0x000A	
#define SDP_ATTR_CLIELNT_EXEC_URL				0x000B	
#define SDP_ATTR_ICON_URL						0x000C	
    /******************PROTOCOL ID*****************************/
#define SDP_PROTOCOL_UUID16                 (0x0001)
#define UDP_PROTOCOL_UUID16                 (0x0002)
#define RFCOMM_PROTOCOL_UUID16              (0x0003)
#define TCP_PROTOCOL_UUID16                 (0x0004)
#define L2CAP_PROTOCOL_UUID16               (0x0100)
#define ServiceDiscoveryServerServiceClassID_UUID16     	(0x1000)
#define BrowseGroupDescriptorServiceClassID_UUID16      	(0x1001)
#define PublicBrowseGroupServiceClassID_UUID16          	(0x1002)
#define SerialPortServiceClassID_UUID16                 	(0x1101)
#define LANAccessUsingPPPServiceClassID_UUID16          	(0x1102)
#define DialupNetworkingServiceClassID_UUID16           	(0x1103)
#define AudioSourceServiceClassID_UUID16                	(0x110A)
#define AudioSinkSourceServiceClassID_UUID16            	(0x110B)

    //#define MAX_DATA_PACK_SIZE	0x20   //128

    typedef struct SDPPduPacket
    {
        unsigned char nPduID;
        unsigned int  nTransNum;
        unsigned int  nParamsLen;
        unsigned char pParams[30];
    }SDPPduPacket_T;
    SDP_EXT unsigned char SDP_Packets_Sender(struct SDPPduPacket SDP_Pdu_Packet );
    SDP_EXT unsigned char SDP_Data_Handler(unsigned char *SDP_Data);
    SDP_EXT unsigned char SDP_Data_Packet_Parse(unsigned char *SDP_Data);
    SDP_EXT unsigned char SDP_Service_Search_Req_Handler(unsigned char *SDP_Data);
    SDP_EXT unsigned char SDP_Service_Attr_Req_Handler(unsigned char *SDP_Data);
    SDP_EXT unsigned char Send_SDP_No_Service_Rsp(unsigned int nTransNum);
    //SDP_EXT unsigned char Send_SDP_Service_Attr_Rsp(unsigned int nTransNum,unsigned int AttrListByteCount,unsigned char *AttrList);
    SDP_EXT unsigned char Send_SDP_Service_Search_Rsp(unsigned int nTransNum,unsigned int TotalSrvRecordCount,unsigned int CurrentSrvRecordCount,unsigned char *SrvRecordHandleList);
    SDP_EXT unsigned char SDP_Service_Search_Attr_Req_Handler(unsigned char *SDP_Data);
    SDP_EXT unsigned char Send_SDP_No_Service_Attr_Rsp(unsigned int nTransNum);
    SDP_EXT unsigned char Send_SDP_Service_Search_Attr_Rsp(unsigned char *SDP_Data);

    SDP_EXT unsigned char Send_SDP_Service_Rsp(unsigned char *SDP_Data);

#ifdef __cplusplus
}
#endif

#endif
