// BinaryMsg.h

#ifndef __BinaryMsg_H__

#define __BinaryMsg_H__

#ifdef __cplusplus

extern "C" {

#endif

/*

 * Copyright (c) 2006  Hemisphere GPS and CSI Wireless Inc.,

 * All Rights Reserved.

 *

 * Use and copying of this software and preparation of derivative works

 * based upon this software are permitted. Any copy of this software or

 * of any derivative work must include the above copyright notice, this

 * paragraph and the one after it.  Any distribution of this software or

 * derivative works must comply with all applicable laws.

 *

 * This software is made available AS IS, and COPYRIGHT OWNERS DISCLAIMS

 * ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE

 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR

 * PURPOSE, AND NOTWITHSTANDING ANY OTHER PROVISION CONTAINED HEREIN, ANY

 * LIABILITY FOR DAMAGES RESULTING FROM THE SOFTWARE OR ITS USE IS

 * EXPRESSLY DISCLAIMED, WHETHER ARISING IN CONTRACT, TORT (INCLUDING

 * NEGLIGENCE) OR STRICT LIABILITY, EVEN IF COPYRIGHT OWNERS ARE ADVISED

 * OF THE POSSIBILITY OF SUCH DAMAGES.

 */

#if defined(WIN32) || (__ARMCC_VERSION >= 300441)

    #pragma pack(push)

    #pragma pack(4)

#endif

#define SBinaryMsgHeader_HEADLENGTH   sizeof(SBinaryMsgHeader)


/****************************************************/

/*  SBinaryMsgHeader                                 */

/****************************************************/

typedef struct

{

    char           m_strSOH[4];      /* start of header ($BIN)      */

    unsigned short m_byBlockID;      /* ID of message (1,2,99,98,97,96,95,94,93 or 80 ) */

    unsigned short m_wDataLength;    /* 52 16,304,68,28,300,128,96,56, or 40 */

} SBinaryMsgHeader;

typedef struct

{

    unsigned long     ulDwordPreamble;    /* 0x4E494224  = $BIN  */

    unsigned long     ulDwordInfo;        /*    0x00340001 or 0x00100002 or 0x01300063 */

} SBinaryMsgHeaderDW;                     /* or 0x00440062 or 0x001C0061 or 0x012C0060 */

                                          /* or 0x0080005F or 0x0060005E or 0x0038005D */

                                          /* or 0x00280050 */

#define BIN_MSG_PREAMBLE    0x4E494224  /* $BIN = 0x4E494224 */

#define BIN_MSG_HEAD_TYPE1  0x00340001  /* 52 = 0x34 */

#define BIN_MSG_HEAD_TYPE2  0x00100002  /* 16 = 0x10 */

#define BIN_MSG_HEAD_TYPE99 0x01300063  /* 99 = 0x63, 304 = 0x130 */

#define BIN_MSG_HEAD_TYPE102 0x01580066 /* 102 = 0x66, 344 = 0x158 */

#define BIN_MSG_HEAD_TYPE101 0x01C00065 /* 101 = 0x65, 448 = 0x1C0 */

#define BIN_MSG_HEAD_TYPE100 0x01040064 /* 100 = 0x64, 260 = 0x104 */

#define BIN_MSG_HEAD_TYPE98 0x00440062  /* 98 = 0x62, 68  = 0x44  */

#define BIN_MSG_HEAD_TYPE97 0x001C0061  /* 97 = 0x61, 28  = 0x1C  */

#define BIN_MSG_HEAD_TYPE96 0x012C0060  /* 96 = 0x60, 300 = 0x12C */

#define BIN_MSG_HEAD_TYPE95 0x0080005F  /* 95 = 0x5F, 128 = 0x80  */

#define BIN_MSG_HEAD_TYPE94 0x0060005E  /* 94 = 0x5E, 96  = 0x60  */

#define BIN_MSG_HEAD_TYPE93 0x0038005D  /* 93 = 0x5D, 56  = 0x38  */

#define BIN_MSG_HEAD_TYPE91 0x0198005B  /* 91 = 0x5B, 408 = 0x198 = total size in bytes -8 -2 -2*/

#define BIN_MSG_HEAD_TYPE89 0x00500059  /* 89 = 0x59, 80  = 0x50  */

#define BIN_MSG_HEAD_TYPE80 0x00280050  /* 80 = 0x50, 40  = 0x28  */

#define BIN_MSG_HEAD_TYPE76 0x01C0004C  /* 76 = 0x4C, 448 = 0x1C0 = total size in bytes -8 -2 -2*/

#define BIN_MSG_HEAD_TYPE71 0x01C00047  /* 71 = 0x47, 448 = 0x1C0 = total size in bytes -8 -2 -2*/

#define BIN_MSG_HEAD_TYPE61 0x0140003D  /* 61 = 0x3D, 320 = 0x140 */

#define BIN_MSG_HEAD_TYPE62 0x0028003E  /* 62 = 0x3E,  40 =  0x28 */

#define BIN_MSG_HEAD_TYPE65 0x00440041  /* 65 = 0x41,  68 =  0x44 */

#define BIN_MSG_HEAD_TYPE66 0x01600042  /* 66 = 0x42, 352 = 0x160 */

#define BIN_MSG_HEAD_TYPE69 0x012C0045  /* 69 = 0x45, 300 = 0x12C */

#define BIN_MSG_HEAD_TYPE59 0x0100003B  /* 59 = 0x3B, 256 = 0x100 */ //GPS L2C

#define BIN_MSG_HEAD_TYPE10 0x0194000A  /* 10 = 0xA, 404 = 0x194 = total size in bytes -8 -2 -2*/

#if defined(_RXAIF_PLOT_MESSAGES_)

   #define BIN_MSG_HEAD_TYPE11   0x0064000B  /* 11 = 0x0B, 100 = 0x64 = total size(112) in bytes -8 -2 -2*/

#endif

#define BIN_MSG_CRLF        0x0A0D      /* CR LF = 0x0D, 0x0A */

#define CHANNELS_12   12

#define CHANNELS_20   20

#define cBPM_SCAT_MEMSIZE 100

#if defined(_RXAIF_PLOT_MESSAGES_)

   #define cBPM_AIFSCAT_MEMSIZE 16

#endif

typedef union

{

    SBinaryMsgHeader   sBytes;

    SBinaryMsgHeaderDW sDWord;

}  SUnionMsgHeader;                      

/****************************************************/

/*  SBinaryMsg1                                     */

/****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned char  m_byAgeOfDiff;       /* age of differential, seconds (255 max)*/

    unsigned char  m_byNumOfSats;       /* number of satellites used (12 max)    */

    unsigned short m_wGPSWeek;          /* GPS week */

    double         m_dGPSTimeOfWeek;    /* GPS tow  */

    double         m_dLatitude;         /* Latitude degrees, -90..90 */

    double         m_dLongitude;        /* Latitude  degrees, -180..180 */

    float          m_fHeight;           /* (m), Altitude ellipsoid */

    float          m_fVNorth;           /* Velocity north       m/s */

    float          m_fVEast;            /* Velocity east        m/s */

    float          m_fVUp;              /* Velocity up  m/s */

    float          m_fStdDevResid;      /* (m), Standard Deviation of   Residuals */

    unsigned short m_wNavMode;          

    unsigned short m_wAgeOfDiff;        /* age of diff using 16 bits  */

    unsigned short m_wCheckSum;         /* sum of all bytes of the header and data */

    unsigned short m_wCRLF;             /* Carriage Return Line Feed */

} SBinaryMsg1;                          /* length = 8 + 52 + 2 + 2 = 64 */

/****************************************************/

/*  SBinaryMsg2                                     */

/****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned long  m_ulMaskSatsTracked;  /* SATS Tracked, bit mapped 0..31 */

    unsigned long  m_ulMaskSatsUsed;     /* SATS Used, bit mapped 0..31 */

    unsigned short m_wGpsUtcDiff;        /* GPS/UTC time difference (GPS minus UTC) */

    unsigned short m_wHDOPTimes10;       /* HDOP        (0.1 units) */

    unsigned short m_wVDOPTimes10;       /* VDOP        (0.1 units) */

    unsigned short m_wWAASMask;          /* Bits 0-1: tracked sats, Bits 2-3:

                                            used sats, Bits 5-9 WAAS PRN 1 minus

                                            120, Bits 10-14 WAAS PRN 1 minus 120 */

    unsigned short m_wCheckSum;          /* sum of all bytes of the header and data */

    unsigned short m_wCRLF;              /* Carriage Return Line Feed */

} SBinaryMsg2;                           /* length = 8 + 16 + 2 + 2 = 28 */

/****************************************************/

/*  SChannelData                                    */

/****************************************************/

typedef struct

{

    unsigned char m_byChannel;      /* channel number  */

    unsigned char m_bySV;           /* satellite being tracked, 0 == not tracked  */

    unsigned char m_byStatus;       /* Status bits (code carrier bit frame...)  */

    unsigned char m_byLastSubFrame; /* last subframe processed */

    unsigned char m_byEphmVFlag;    /* ephemeris valid flag */

    unsigned char m_byEphmHealth;   /* ephemeris health */

    unsigned char m_byAlmVFlag;     /* almanac valid flag */

    unsigned char m_byAlmHealth;    /* almanac health */

    char          m_chElev;         /* elevation angle */

    unsigned char m_byAzimuth;      /* 1/2 the Azimuth angle */

    unsigned char m_byURA;          /* User Range Error */

    unsigned char m_byDum;          /* Place Holder */

    unsigned short m_wCliForSNR;    /* code lock indicator for SNR divided by 32 */

    short         m_nDiffCorr;      /* Differential correction * 100 */

    short         m_nPosResid;      /* position residual * 10 */

    short         m_nVelResid;      /* velocity residual * 10 */

    short         m_nDoppHz;        /* expected doppler in HZ */

    short         m_nNCOHz;         /* track from NCO in HZ */

} SChannelData;  /* 24 bytes */

/****************************************************/

/*  SChannelL2Data                                  */

/****************************************************/

//#if defined(_DUAL_FREQ_)

typedef struct

{

    unsigned char m_byChannel;      /* channel number  */

    unsigned char m_bySV;           /* satellite being tracked, 0 == not tracked  */

    unsigned char m_byL2CX;         /* Status bits for L2P (code carrier bit frame...)  */

    unsigned char m_byL1CX;         /* Status bits for L1P (code carrier bit frame...)  */

    unsigned short m_wCliForSNRL2P; /* code lock indicator for SNR divided by 32 */

    unsigned short m_wCliForSNRL1P; /* code lock indicator for L1P SNR divided by 32 */

    short         m_nC1_L1;         /* C1-L1 in meters * 100 */

    short         m_nP2_C1;         /* P2-C1 in meters * 100 */

    short         m_nP2_L1;         /* P2-L1 in meters * 100 */

    short         m_nL2_L1;         /* L2-L1 in meters * 100 */

    short         m_nP2_P1;         /* P2-P1 in meters * 100 */

    short         m_nNCOHz;         /* track from NCO in HZ */

} SChannelL2Data;  /* 20 bytes */

//#endif

/****************************************************/

/*  SChannelL2CData     for USING_GPSL2CL           */

/****************************************************/

typedef struct

{

    unsigned char m_byChannel;      // channel number

    unsigned char m_bySV;           // satellite being tracked, 0 == not tracked

    unsigned char m_byL2CX;         // Status bits for L2P (code carrier bit frame...)

    unsigned char spare1;

    unsigned short m_wCliForSNRL2C; // code lock indicator for SNR divided by 32

    unsigned short spare2;

    short         m_nL2C_L1Ca;      //L2CL - CA code error  meters * 100

    short         m_nL2C_L2P;       //L2CL - L2P code error meters * 100

    short         m_nL2_L1;         //L2CL - L1CA phase error meters * 100

    short         m_nL2_L2P;        //L2CL - L2P phase error meters * 100

    short         spare3;         

    short         m_nNCOHz;         // track from NCO in HZ

} SChannelL2CData;  // 20 bytes

//-****************************************************

//-*  SBinaryMsg3

//-*  Lat/Lon/Hgt, Covariances, RMS, DOPs and COG, Speed, Heading

//-****************************************************

typedef struct

{

    SUnionMsgHeader  m_sHead;              //                                      [8]

    double           m_dGPSTimeOfWeek;     // GPS tow                              [8 bytes] 

    unsigned short   m_wGPSWeek;           // GPS week                             [2 bytes]

    unsigned short   m_wNumSatsTracked;    // SATS Tracked                         [2 bytes]

    unsigned short   m_wNumSatsUsed;       // SATS Used                            [2 bytes]

    unsigned char    m_byNavMode;          // Nav Mode (same as message 1)         [1 byte ]

    unsigned char    m_bySpare00;          // Spare                                [1 byte ]

    double           m_dLatitude;          // Latitude degrees, -90..90            [8 bytes] 

    double           m_dLongitude;         // Longitude degrees, -180..180         [8 bytes]

    float            m_fHeight;            // (m), Altitude ellipsoid              [4 bytes] 

    float            m_fSpeed;             // Horizontal Speed   m/s               [4 bytes] 

    float            m_fVUp;               // Vertical Velocity +up  m/s           [4 bytes]

    float            m_fCOG;               // Course over Ground, degrees          [4 bytes] 

    float            m_fHeading;           // Heading (degrees), Zero unless vector[4 bytes]     

    float            m_fPitch;             // Pitch (degrees), Zero unless vector  [4 bytes] 

    float            m_fSpare01;           // Spare                                [4 bytes] 

    unsigned short   m_wAgeOfDiff;         // age of differential, seconds         [2 bytes]

    unsigned short   m_wSpare02;           // Spare                                [2 bytes]

    unsigned long    m_ulSpare03;          // Spare                                [4 bytes] 

    unsigned long    m_ulSpare04;          // Spare                                [4 bytes] 

    float            m_fHRMS;              // Horizontal RMS                       [4 bytes] 

    float            m_fVRMS;              // Vertical   RMS                       [4 bytes] 

    float            m_fHDOP;              // Horizontal DOP                       [4 bytes] 

    float            m_fVDOP;              // Vertical DOP                         [4 bytes] 

    float            m_fTDOP;              // Time DOP                             [4 bytes] 

    float            m_fCovNN;             // Covaraince North-North               [4 bytes] 

    float            m_fCovNE;             // Covaraince North-East                [4 bytes] 

    float            m_fCovNU;             // Covaraince North-Up                  [4 bytes] 

    float            m_fCovEE;             // Covaraince East-East                 [4 bytes] 

    float            m_fCovEU;             // Covaraince East-Up                   [4 bytes] 

    float            m_fCovUU;             // Covaraince Up-Up                     [4 bytes] 

    unsigned short   m_wCheckSum;          // sum of all bytes of the header and data

    unsigned short   m_wCRLF;              // Carriage Return Line Feed

} SBinaryMsg3;                             // length = 8 + 116 + 2 + 2 = 128  (108 = 74 hex)





/****************************************************/

/*  SObsPacket                                      */

/****************************************************/

typedef struct

{

    unsigned long    m_ulCS_TT_W3_SNR;   /* Bits 0-11 (12 bits) =SNR_value

                                            For L1 SNR = 10.0*log10( 0.1024*SNR_value)

                                            FOR L2 SNR = 10.0*log10( 0.1164*SNR_value) */

                                         /* Bits 12-14 (3 bits) = 3 bits of warning

                                            for potential 1/2 cycle slips.  A warning

                                            exists if any of these bits are set. */

                                         /* bit 15: (1 bit) 1 if Track Time > 25.5 sec,

                                                            0 otherwise */

                                         /* Bits 16-23 (8 bits): Track Time in units

                                            of 1/10 second (range = 0 to 25.5 seconds) */

                                         /* Bits 24-31 (8 bits) = Cycle Slip Counter

                                            Increments by 1 every cycle slip

                                            with natural roll-over after 255 */

    unsigned long    m_ulP7_Doppler_FL;  /* Bit 0: (1 bit) 1 if Valid Phase, 0 otherwise

                                            Bit 1-23: (23 bits) =Magnitude of doppler

                                                LSB = 1/512 cycle/sec

                                                Range = 0 to 16384 cycle/sec

                                            Bit 24: sign of doppler, 1=negative, 0=pos

                                            Bits 25-31 (7 bits) = upper 7 bits of the

                                                23 bit carrier phase.

                                                LSB = 64 cycles,  MSB = 4096 cycles */

    unsigned long    m_ulCodeAndPhase;   /* Bit 0-15 (16 bits) lower 16 bits of code

                                                pseudorange

                                                LSB = 1/256 meters

                                                MSB = 128 meters

                                                Note, the upper 19 bits are given in

                                                m_aulCACodeMSBsPRN[] for CA code

                                            Bit 16-31 lower 16 bits of the carrier phase,

                                                      7 more bits are in m_ulP7_Doppler_FL

                                                LSB = 1/1024 cycles

                                                MSB = 32 cycles */

                                           

} SObsPacket;  /* 12 bytes , note: all zero if data not available */

//-****************************************************
//-*  SBinaryMsg36
//-*  Covariances, RMS and DOPs
//-****************************************************
typedef struct
{
    SUnionMsgHeader m_sHead;    // (8 bytes)
    double m_dTow;              // Time in seconds (8 bytes)
    unsigned short m_wWeek;     // GPS Week Number (2 bytes)
    unsigned short m_wSpare1;   // spare 1 (zero) (2 bytes)
    unsigned long m_uFreqPage;  //[0-19] Spare bits
                                //[20,21,22,23] Number of Pages
                                //[24,25,26,27] Page Number
                                //[28,29,30,31] Signal ID (B1I, B2I, B3I, etc)
    SObsPacket  m_asObs[CHANNELS_20]; // 20 sets of BeiDou observations 
                                     // (20*12=240 bytes)
    unsigned long m_aulCodeMSBsPRN[CHANNELS_20]; // array of 20 words
                                                // (20*4=80 bytes)
                                                // bit 7:0 (8 bits) =
                                                // satellite PRN, 0
                                                // if no satellite
                                                // bit 12:8 (5 bits) =
                                                // spare
                                                // bit 31:13 (19 bits) =
                                                // upper 19 bits
                                                // of B1/B2/B3
                                                // LSB = 256 meters
                                                // MSB = 67108864 meters
    unsigned short m_wCheckSum; // sum of all bytes of header and data (2 bytes)
    unsigned short m_wCRLF;     // Carriage Return Line Feed (2 bytes)
} SBinaryMsg36; // length = 8 + (8+2+2+4+240+80) + 2 + 2 = 348           

/****************************************************/

/*  SBinaryMsg99                                    */

/****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned char  m_byNavMode;         /* Nav Mode FIX_NO, FIX_2D, FIX_3D (high bit =has_diff) */

    char           m_cUTCTimeDiff;      /* whole Seconds between UTC and GPS   */

    unsigned short m_wGPSWeek;          /* GPS week */

    double         m_dGPSTimeOfWeek;    /* GPS tow  */

    SChannelData   m_asChannelData[CHANNELS_12]; /* channel data */

    short          m_nClockErrAtL1;     /* clock error at L1, Hz */

    unsigned short m_wSpare;            /* spare */

    unsigned short m_wCheckSum;         /* sum of all bytes of the header and data */

    unsigned short m_wCRLF;             /* Carriage Return Line Feed */

} SBinaryMsg99;                         /* length = 8 + 304 + 2 + 2 = 316 */

#define CHANNELS_SBAS_E    3

/****************************************************/

/*  SBinaryMsg89  * Supports 3 SBAS Satellites      */

/****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    long           m_lGPSSecOfWeek;     /* GPS tow integer sec */

    unsigned char  m_byMaskSBASTracked; /* SBAS Sats Tracked, bit mapped 0..3 */

    unsigned char  m_byMaskSBASUSED;    /* SBAS Sats Used, bit mapped 0..3 */

    unsigned short m_wSpare;            /* spare */

    SChannelData   m_asChannelData[CHANNELS_SBAS_E];  /* SBAS channel data */

    unsigned short m_wCheckSum;         /* sum of all bytes of the header and data */

    unsigned short m_wCRLF;             /* Carriage Return Line Feed */

} SBinaryMsg89;                         /* length = 8 + 80 + 2 + 2 = 92 */

/****************************************************/

/*  SBinaryMsg100                                    */

/****************************************************/

//#if defined(_DUAL_FREQ_)

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned char  m_byNavMode;         /* Nav Mode FIX_NO, FIX_2D, FIX_3D (high bit =has_diff) */

    char           m_cUTCTimeDiff;      /* whole Seconds between UTC and GPS   */

    unsigned short m_wGPSWeek;          /* GPS week */

    unsigned long  m_ulMaskSatsUsedL2P; /* L2P SATS Used, bit mapped 0..31 */

    double         m_dGPSTimeOfWeek;    /* GPS tow  */

    unsigned long  m_ulMaskSatsUsedL1P; /* L1P SATS Used, bit mapped 0..31 */

    SChannelL2Data m_asChannelData[CHANNELS_12]; /* channel data */

    unsigned short m_wCheckSum;         /* sum of all bytes of the header and data */

    unsigned short m_wCRLF;             /* Carriage Return Line Feed */

} SBinaryMsg100;                        /* length = 8 + 260 + 2 + 2 = 272 */

//#endif

/****************************************************/

/*  SBinaryMsg59   for USING_GPSL2CL                */

/****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned char  m_byNavMode;         /* Nav Mode FIX_NO, FIX_2D, FIX_3D (high bit =has_diff) */ //1 byte

    char           m_cUTCTimeDiff;      /* whole Seconds between UTC and GPS   */                  //1 byte

    unsigned short m_wGPSWeek;          /* GPS week */                                             //2 bytes

    unsigned long  m_ulMaskSatsUsedL2P; /* L2P SATS Used, bit mapped 0..31 */                      //4 bytes

    double         m_dGPSTimeOfWeek;    /* GPS tow  */                                             //8 bytes

    SChannelL2CData m_asChannelData[CHANNELS_12]; /* channel data */                               //20*12 bytes

    unsigned short m_wCheckSum;         /* sum of all bytes of the header and data */

    unsigned short m_wCRLF;             /* Carriage Return Line Feed */

} SBinaryMsg59;                         /* length = 8 + 260 + 2 + 2 = 272 */

/****************************************************/

/*  SSVAlmanData                                    */

/****************************************************/

typedef struct

{

    short         m_nDoppHz;        /* doppler in HZ for stationary receiver */

    unsigned char m_byCountUpdate;  /* count of almanac updates */

    unsigned char m_bySVindex;      /* 0 through 31 (groups of 8)*/

    unsigned char m_byAlmVFlag;     /* almanac valid flag */

    unsigned char m_byAlmHealth;    /* almanac health */

    char          m_chElev;         /* elevation angle */

    unsigned char m_byAzimuth;      /* 1/2 the Azimuth angle */

} SSVAlmanData;  /* 8 bytes */

/****************************************************/

/*  SBinaryMsg98                                    */

/****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    SSVAlmanData   m_asAlmanData[8];    /* SV data, 8 at a time */

    unsigned char m_byLastAlman;        /* last almanac processed */

    unsigned char m_byIonoUTCVFlag;     /* iono UTC flag */

    unsigned short m_wSpare;            /* spare */

    unsigned short m_wCheckSum;         /* sum of all bytes of the header and data */

    unsigned short m_wCRLF;             /* Carriage Return Line Feed */

} SBinaryMsg98;                         /* length = 8 + (64+1+1+2) + 2 + 2 = 80 */

/****************************************************/

/*  SBinaryMsg97                                    */

/****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned long  m_ulCPUFactor;       /* CPU utilization Factor (%=multby 450e-6) */

    unsigned short m_wMissedSubFrame;   /* missed subframes */

    unsigned short m_wMaxSubFramePend;  /* max subframe pending */

    unsigned short m_wMissedAccum;      /* missed accumulations */

    unsigned short m_wMissedMeas;       /* missed measurements */

    unsigned long  m_ulSpare1;          /* spare 1 (zero)*/

    unsigned long  m_ulSpare2;          /* spare 2 (zero)*/

    unsigned long  m_ulSpare3;          /* spare 3 (zero)*/

    unsigned short m_wSpare4;           /* spare 4 (zero)*/

    unsigned short m_wSpare5;           /* spare 5 (zero)*/

    unsigned short m_wCheckSum;         /* sum of all bytes of the headerand data */

    unsigned short m_wCRLF;             /* Carriage Return Line Feed */

} SBinaryMsg97;                         /* length = 8 + (28) + 2 + 2 = 40 */

/****************************************************/

/*  SObservations                                   */

/****************************************************/

typedef struct

{

    unsigned long    m_ulCS_TT_SNR_PRN; /* Bits 0-7 PRN (PRN is 0 if no data) */

                                        /* Bits 8-15 SNR_value

                                           SNR = 10.0*log10( 0.8192*SNR_value) */

                                        /* Bits 16-23 Phase Track Time in units

                                           of 1/10 second (range = 0 to 25.5

                                           seconds  (see next word) */

                                        /* Bits 24-31 Cycle Slip Counter

                                           Increments by 1 every cycle slip

                                           with natural roll over after 255 */

    unsigned long    m_ulDoppler_FL;    /* Bit  0: 1 if Valid Phase, 0 otherwise

                                           Bit  1: 1 if Track Time > 25.5 sec,

                                                   0 otherwise

                                           Bits 2-3: unused

                                           Bits 4-32: Signed (two's compliment)

                                           doppler in units of m/sec x 4096.

                                           (i.e.,  LSB = 1/4096). Range =

                                           +/- 32768 m/sec. Computed as

                                           phase change over 1/10 sec. */

    double           m_dPseudoRange;    /* pseudo ranges (m) */

    double           m_dPhase;          /* phase (m) L1 wave len = 0.190293672798365*/

} SObservations;  /* 24 bytes */

/****************************************************/

/*  SBinaryMsg96                                    */

/****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned short   m_wSpare1;            /* spare 1 (zero)*/

    unsigned short   m_wWeek;              /* GPS Week Number */

    double           m_dTow;               /* Predicted GPS Time in seconds */

    SObservations    m_asObvs[CHANNELS_12];/* 12 sets of observations */

    unsigned short   m_wCheckSum;          /* sum of all bytes of the header and data */

    unsigned short   m_wCRLF;              /* Carriage Return Line Feed */

} SBinaryMsg96;                            /* length = 8 + (300) + 2 + 2 = 312 */

/****************************************************/

/*  SBinaryMsg95                                    */

/****************************************************/

/* sent only upon command or when values change */

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned short   m_wSV;               /* The satellite to which this data belongs. */

    unsigned short   m_wSpare1;           /* spare 1 (chan number (as zero 9/1/2004)*/

    unsigned long    m_TOW6SecOfWeek;     /* time at which this arrived (LSB = 6sec) */

    unsigned long    m_SF1words[10];      /* Unparsed SF 1 message words. */

    unsigned long    m_SF2words[10];      /* Unparsed SF 2 message words. */

    unsigned long    m_SF3words[10];      /* Unparsed SF 3 message words. */

                                          /* Each of the subframe words contains

                                             one 30-bit GPS word in the lower

                                             30 bits, The upper two bits are ignored

                                             Bits are placed in the words from left to

                                             right as they are received */

    unsigned short   m_wCheckSum;         /* sum of all bytes of the header and data */

    unsigned short   m_wCRLF;             /* Carriage Return Line Feed */

} SBinaryMsg95;                           /* length = 8 + (128) + 2 + 2 = 140 */

/****************************************************/

/*  SBinaryMsg94                                    */

/****************************************************/

/* sent only upon command or when values change */

typedef struct

{

    SUnionMsgHeader  m_sHead;

    /* Iono parameters. */

    double         m_a0,m_a1,m_a2,m_a3;  /* AFCRL alpha parameters. */

    double         m_b0,m_b1,m_b2,m_b3;  /* AFCRL beta parameters.  */

    /* UTC conversion parameters. */

    double         m_A0,m_A1;            /* Coeffs for determining UTC time. */

    unsigned long  m_tot;                /* Reference time for A0 & A1, sec of GPS week. */

    unsigned short m_wnt;                /* Current UTC reference week number. */

    unsigned short m_wnlsf;              /* Week number when dtlsf becomes effective. */

    unsigned short m_dn;                 /* Day of week (1-7) when dtlsf becomes effective. */

    short          m_dtls;               /* Cumulative past leap seconds. */

    short          m_dtlsf;              /* Scheduled future leap seconds. */

    unsigned short m_wSpare1;            /* spare 4 (zero)*/

    unsigned short m_wCheckSum;          /* sum of all bytes of the header and data */

    unsigned short m_wCRLF;              /* Carriage Return Line Feed */

} SBinaryMsg94;                          /* length = 8 + (96) + 2 + 2 =  108 */

/****************************************************/

/*  SBinaryMsg93                                   */

/****************************************************/

/* sent only upon command or when values change */

/* WAAS ephemeris */

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned short   m_wSV;               /* The satellite to which this data belongs. */

    unsigned short   m_wWeek;             /* Week corresponding to m_lTOW*/

    unsigned long    m_lSecOfWeekArrived; /* time at which this arrived (LSB = 1sec) */

    unsigned short   m_wIODE;

    unsigned short   m_wURA;              /* See 2.5.3 of Global Pos Sys Std Pos Service Spec */

    long m_lTOW;                          /* Sec of WEEK Bit 0 = 1 sec */

    long m_lXG;                           /* Bit 0 = 0.08 m */

    long m_lYG;                           /* Bit 0 = 0.08 m */

    long m_lZG;                           /* Bit 0 = 0.4 m */

    long m_lXGDot;                        /* Bit 0 = 0.000625 m/sec */

    long m_lYGDot;                        /* Bit 0 = 0.000625 m/sec */

    long m_lZGDot;                        /* Bit 0 = 0.004 m/sec */

    long m_lXGDotDot;                     /* Bit 0 = 0.0000125 m/sec/sec */

    long m_lYGDotDot;                     /* Bit 0 = 0.0000125 m/sec/sec */

    long m_lZGDotDot;                     /* Bit 0 = 0.0000625 m/sec/sec */

    short  m_nGf0;                        /* Bit 0 = 2**-31 sec */

    short  m_nGf0Dot;                     /* Bit 0 = 2**-40 sec/sec */

    unsigned short   m_wCheckSum;         /* sum of all bytes of the header and data */

    unsigned short   m_wCRLF;             /* Carriage Return Line Feed */

} SBinaryMsg93;                           /* length = 8 + (56) + 2 + 2 = 68 */

/****************************************************/

/*  SBinaryMsg80                                    */

/****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned short m_wPRN;              /* Broadcast PRN */

    unsigned short m_wSpare;            /* spare (zero) */

    unsigned long  m_ulMsgSecOfWeek;    /* Seconds of Week For Message */

    unsigned long  m_aulWaasMsg[8];     /* Actual 250 bit waas message*/

    unsigned short m_wCheckSum;         /* sum of all bytes of the headerand data */

    unsigned short m_wCRLF;             /* Carriage Return Line Feed */

} SBinaryMsg80;                         /* length = 8 + (40) + 2 + 2 = 52 */

/****************************************************/

/*  SMsg91Data                                      */

/****************************************************/

typedef struct

{

    unsigned char  bySV;             /* satellite being tracked, 0 == not tracked  */

    unsigned char  byStatus;         /* Status bits (code carrier bit frame...)  */

    unsigned char  byStatusSlave;    /* Status bits (code carrier bit frame...)  */

    unsigned char  byChannel;        /* Not used */

    

    unsigned short wEpochSlew;                /* 20*_20MS_EPOCH_SLEW + _1MS_EPOCH_SLEW */

    unsigned short wEpochCount;               /* epoch_count */

    unsigned long  codeph_SNR;                /* 0-20 = code phase (21 bits), 28-32 = SNR/4096, upper 4 bits */

    unsigned long  ulCarrierCycles_SNR;       /* 0-23 = carrier cycles, 24-32 = SNR/4096 lower 8 bits */

    unsigned short wDCOPhaseB10_HalfWarns;    /* 0-11 = DCO phase, 12-14 = Half Cycle Warn

                                                 15 = half Cycle added */

    unsigned short m_wPotentialSlipCount;     /* potential slip count */

    /* SLAVE DATA */

    unsigned long  codeph_SNR_Slave;             /* 0-20 = code phase (21 bits), 28-32 = SNR/4096, upper 4 bits */

    unsigned long  ulCarrierCycles_SNR_Slave;    /* 0-23 = carrier cycles, 24-32 = SNR/4096 lower 8 bits */

    unsigned short wDCOPhaseB10_HalfWarns_Slave; /* 0-11 = DCO phase, 12-14 = Half Cycle Warn

                                                    15 = half Cycle added */

    unsigned short m_wPotentialSlipCount_Slave;  /* potential slip count */

} SMsg91Data; /* 32 bytes */

/****************************************************/

  /*  SBinaryMsg91                                    */

  /*  Comment: Transmits data from Takemeas.c         */

  /*           debugging structure.                   */

  /*           Added by bbadke 7/07/2003              */

  /****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;                /* 8 */

    double           m_sec;                  /* 8 bytes */

    int              m_iWeek;                /* 4 bytes */

    unsigned long    m_Tic;                  /* 4 bytes */

    long             lTicOfWeek;             /* 4 bytes */

    long             lProgTic;               /* 4 bytes */

    SMsg91Data       s91Data[CHANNELS_12];   /* 12*32= 384 bytes */

    unsigned short   m_wCheckSum;            /* sum of all bytes of the header and data */

    unsigned short   m_wCRLF;                /* Carriage Return Line Feed */

} SBinaryMsg91;                              /* length = 8 + (408) + 2 + 2 = 420 */    



/* A NOTE ON DECODING MESSAGE 76

 * Notation: "code" -- is taken to mean the PseudoRange derived from code phase.

 *           "phase" -- is taken to mean range derived from carrier phase.

 *                      This will contain cycle ambiguities.

 *

 * Only the lower 16 bits of L1P code, L2P code and the lower 23 bits of

 * carrier phase are provided. The upper 19 bits of the L1CA code are found

 * in m_aulCACodeMSBsPRN[].  The upper 19 bits of L1P or L2P must be derived

 * using the fact that L1P and L2P are within 128 meters of L1CA.  To

 * determine L1P or L2P, use the lower 16 bits provided in the message and  

 * set the upper bits to that of L1CA.  Then add or subtract one LSB of the

 * upper bits (256 meters) so that L1P or L2P are within 1/2 LSB (128 meters)

 * of the L1CA code.  

 *     The carrier phase is in units of cycles, rather than meters,

 * and is held to within 1023 cycles of the respective code range.  Only

 * the lower 16+7=23 bits of carrier phase are transmitted in Msg 76.

 * In order to determine the remaining bits, first convert the respective

 * code range (determined above) into cycles by dividing by the carrier

 * wavelength.  Call this the "nominal reference phase". Next extract the 16

 * and 7 bit blocks of carrier phase from Msg 76 and arrange to form the lower

 * 23 bits of carrier phase.  Set the upper bits (bit 23 and above) equal to

 * those of the nominal reference phase.  Then, similar to what was done for

 * L1P and L2P, add or subtract the least significant upper bit (8192 cycles)

 * so that carrier phase most closely agrees with the nominal reference phase

 * (to within 4096 cycles).

 */

#define CHANNELS_12_PLUS (CHANNELS_12+2)               /* up to two SBAS satellites */

#define CHANNELS_L1_E    (CHANNELS_12+CHANNELS_SBAS_E) /* All L1 (including SBAS satellites) */

/****************************************************/

/*  SBinaryMsg76                                    */

/****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    double           m_dTow;                    /* GPS Time in seconds */

    unsigned short   m_wWeek;                   /* GPS Week Number */

    unsigned short   m_wSpare1;                 /* spare 1 (zero)*/

    unsigned long    m_ulSpare2;                /* spare 2 (zero)*/

    SObsPacket       m_asL2PObs[CHANNELS_12];   /* 12 sets of L2(P) observations */

    SObsPacket       m_asL1CAObs[CHANNELS_L1_E];  /* 15 sets of L1(CA) observations */

    unsigned long    m_aulCACodeMSBsPRN[CHANNELS_L1_E]; /* array of 15 words.  

                                                    bit 7:0 (8 bits) = satellite PRN, 0

                                                    if no satellite

                                                    bit 12:8 (5 bits) = spare

                                                    bit 31:13 (19 bits) = upper 19 bits

                                                    of L1CA  LSB = 256 meters

                                                             MSB = 67108864 meters */

    unsigned long    m_auL1Pword[CHANNELS_12];  /* array of 12 words relating to L1(P) code.

                                                   Bit 0-15 (16 bits) lower 16 bits of the

                                                   L1P code pseudo range.

                                                   LSB = 1/256 meters

                                                   MSB = 128 meters

                                                   Bits 16-27 (12 bits) = L1P SNR_value

                                                   SNR = 10.0*log10( 0.1164*SNR_value)

                                                   If Bits 16-27 all zero, no L1P track

                                                   Bits 28-31 (4 bits) spare */

    unsigned short   m_wCheckSum;               /* sum of all bytes of the header and data */

    unsigned short   m_wCRLF;                   /* Carriage Return Line Feed */

} SBinaryMsg76;                                 /* length = 8 + (448) + 2 + 2 = 460 */

/****************************************************/

/*  SMsg71DataL1                                    */

/****************************************************/

typedef struct

{

    unsigned char  bySV;                    /* satellite being tracked, 0 == not tracked  */

    unsigned char  byStatus;                /* Status bits (code carrier bit frame...)  */

    unsigned char  byStatusL1P;             /* 0-8 lower 8 bits of L1P SNR/32768, if zero and

                                               if upper two bits of m_wSNR_codeph_L1P are zero

                                               then L1P is not tracking */

    unsigned char  byStatusL2P;             /* Status bits (code carrier phase ...)  */

    unsigned short wEpochSlew;              /* 20*_20MS_EPOCH_SLEW + _1MS_EPOCH_SLEW */

    unsigned short wEpochCount;             /* epoch_count */

    unsigned long  codeph_SNR;              /* 0-20 = code phase (21 bits), 28-32 = SNR/4096, upper 4 bits */

    unsigned long  ulCarrierCycles_SNR;     /* 0-23 = carrier cycles, 24-32 = SNR/4096 lower 8 bits */

    unsigned short wDCOPhaseB10_HalfWarns;  /* 0-11 = DCO phase, 12-14 = Half Cycle Warn

                                               15 = half Cycle added */

    unsigned short m_wPotentialSlipCount;   /* potential slip count */

} SMsg71DataL1; /* 20 bytes */

/****************************************************/

/*  SMsg71DataL1PL2P                                */

/****************************************************/

typedef struct

{

    /* L1P and L2P Data */

 //   unsigned long  codeph_SNR_L1P; NOT USED YET  /* 0-22 = L1 code phase (23 bits), 28-32 = SNR/8192, upper 4 bits */

    unsigned long  codeph_SNR_L2P;                 /* 0-22 = L2P code phase (23 bits), 28-32 = SNR/8192, upper 4 bits */

    unsigned long  ulCarrierCycles_SNR_L2P;        /* 0-23 = carrier cycles, 24-32 = SNR/8192 lower 8 bits */

    unsigned short wDCOPhaseB10_L2P;               /* 0-11 = DCO phase, 12-15 = Spare */

    unsigned short m_wSNR_codeph_L1P;              /* 0-13 = lower 14 bits of L1P code,  14-15 SNR/32768 Upper 2 bits */

                                                   /* To get full L1P code, use upper bits form L2P and adjust by

                                                      +/- 2**14 if necessary */

} SMsg71DataL1PL2P; /* 12 bytes */

/****************************************************/

  /*  SBinaryMsg71                                    */

  /*  Comment: Transmits data from Takemeas.c         */

  /*           debugging structure for Dual Freq.     */

  /****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;                    /* 8 */

    double           m_sec;                      /* 8 bytes */

    int              m_iWeek;                    /* 4 bytes */

    unsigned long    m_Tic;                      /* 4 bytes */

    long             lTicOfWeek;                 /* 4 bytes */

    long             lProgTic;                   /* 4 bytes */

    SMsg71DataL1PL2P s91L2PData[CHANNELS_12];    /* 12*12 = 144 bytes */

    SMsg71DataL1     s91Data[CHANNELS_12_PLUS];  /* 14*20 = 280 bytes */

    unsigned short   m_wCheckSum;                /* sum of all bytes of the header and data */

    unsigned short   m_wCRLF;                    /* Carriage Return Line Feed */

} SBinaryMsg71;                                  /* length = 8 + (448) + 2 + 2 = 460 */      

/////////////////////////////////////////////////////

//  SBinaryMsg10                                   

//  Comment: Transmits scatter plot data from       

//           buffacc.c                              

//

/////////////////////////////////////////////////////

enum eBIN10_TYPE {eBIN10_GPSL1CA=0,eBIN10_GPSL1P,eBIN10_GPSL2P,

                  eBIN10_GLONASSL1,eBIN10_GLONASSL2,eBIN10_GPSL2CL,eBIN10_GPSL5Q};

typedef struct

{

    SUnionMsgHeader  m_sHead;                // 8 bytes

    unsigned short m_awScatterPlotDataI[cBPM_SCAT_MEMSIZE]; //100*2 = 200 bytes

    unsigned short m_awScatterPlotDataQ[cBPM_SCAT_MEMSIZE]; //100*2 = 200 bytes

    unsigned short m_wChannel;

    unsigned short m_wSigType;               // one of eBIN10_TYPE

    unsigned short   m_wCheckSum;            // sum of all bytes of the header and data

    unsigned short   m_wCRLF;                // Carriage Return Line Feed

} SBinaryMsg10;                              // length = 8 +200 +200 +2 +2 +2 +2 = 416      

#if defined(_RXAIF_PLOT_MESSAGES_)

/////////////////////////////////////////////////////

//  SBinaryMsg11                                   

//  Comment: Transmits scatter plot data for RXGNSS_AIF statistics

//

/////////////////////////////////////////////////////

enum eBIN11_TYPE {eBIN11_COUNTS=0,eBIN11_VALUES};

typedef struct

{

    SUnionMsgHeader  m_sHead;                // 8 bytes

    unsigned short   m_awScatterPlotDataValues[cBPM_AIFSCAT_MEMSIZE];   //16*2 = 32 bytes

    unsigned short   m_awScatterPlotDataCntMag[cBPM_AIFSCAT_MEMSIZE];   //16*2 = 32 bytes

    unsigned short   m_awScatterPlotDataCntDCoff[cBPM_AIFSCAT_MEMSIZE]; //16*2 = 32 bytes

    unsigned short   m_wChannel;             // aif_sel 0: AIF_A, 1: AIF_B, ...

    unsigned short   m_wSigType;             // one of eBIN11_TYPE

    unsigned short   m_wCheckSum;            // sum of all bytes of the header and data

    unsigned short   m_wCRLF;                // Carriage Return Line Feed

} SBinaryMsg11;                              // length = 8 +32 +32 +32 +2 +2 +2 +2 = 112      

#endif

/****************************************************/

/*  SGLONASSChanData                                */

/****************************************************/

typedef struct

{

    unsigned char m_bySV;              /* Bit (0-6) = SV slot, 0 == not tracked

                                        * Bit 7 = Knum flag

                                        * = KNum+8 if bit 7 set

                                        */

    unsigned char m_byAlm_Ephm_Flags;  /* ephemeris and almanac status flags */

                                       /* bit 0: Ephemeris available but timed out

                                        * bit 1: Ephemeris valid

                                        * bit 2: Ephemeris health OK

                                        * bit 3: unused

                                        * bit 4: Almanac available

                                        * bit 5: Almanac health OK  

                                        * bit 6: unused

                                        * bit 7: Satellite doesn't exist

                                        */

    unsigned char m_byStatus_L1;       /* Status bits (code carrier bit frame...)  */

    unsigned char m_byStatus_L2;       /* Status bits (code carrier bit frame...)  */

    char          m_chElev;            /* elevation angle */

    unsigned char m_byAzimuth;         /* 1/2 the Azimuth angle */

    unsigned char m_byLastMessage;     /* last message processed */

    unsigned char m_bySlip01;          /* cycle slip on chan 1 */

    unsigned short m_wCliForSNR_L1;    /* code lock indicator for SNR divided by 32 */

    unsigned short m_wCliForSNR_L2;    /* code lock indicator for SNR divided by 32 */

    short         m_nDiffCorr_L1;      /* Differential correction * 100 */

    short         m_nDoppHz;           /* expected doppler in HZ at glonass L1 */

    short         m_nNCOHz_L1;         /* track from NCO in HZ */

    short         m_nNCOHz_L2;         /* track from NCO in HZ */

    short         m_nPosResid_1;       /* position residual 1 * 1000 */

    short         m_nPosResid_2;       /* position residual 2 * 1000 */

} SGLONASSChanData;  /* 24 bytes */

/****************************************************/

/*  SBinaryMsg69                                    */

/****************************************************/

typedef struct

{

    SUnionMsgHeader    m_sHead;

    long               m_lSecOfWeek;      /* tow  */

    unsigned short     m_wL1usedNavMask;  /* mask of L1 channels used in nav solution */

    unsigned short     m_wL2usedNavMask;  /* mask of L2 channels used in nav solution */

    SGLONASSChanData   m_asChannelData[CHANNELS_12]; /* channel data 12X24 = 288 */

    unsigned short     m_wWeek;          /* week */

    unsigned char      m_bySpare01;      /* spare 1 */

    unsigned char      m_bySpare02;      /* spare 2 */

    unsigned short     m_wCheckSum;      /* sum of all bytes of the header and data */

    unsigned short     m_wCRLF;          /* Carriage Return Line Feed */

} SBinaryMsg69;                          /* length = 8 + 300 + 2 + 2 = 312 */

/****************************************************/

/*  SMsg61Data                                      */

/****************************************************/

typedef struct

{

    unsigned char  bySV;               /* satellite slot  0 == not tracked  */

    unsigned char  byStatusL1;         /* Status bits (code carrier bit frame...)  */

    unsigned char  byStatusL2;         /* Status bits (code carrier bit frame...)  */

    unsigned char  byL1_L2_DCO;        /* 0-3 = upper 4 bits of L1 carrier DCO Phase

                                        * 4-7 = upper 4 bits of L2 carrier DCO Phase

                                        */

    unsigned short wEpochSlewL1;       /* 0-9 = slew, 0 to 1000 count for ms of sec

                                        * 10-15 = 6 bits of L1 slip count */

    unsigned short wEpochCountL1;      /* 0-9 = epoch_count, 0 to 1000 count for ms of sec

                                        * 10-15 = 6 bits of L2 slip count */

    unsigned long  codeph_SNR_L1;      /* 0-20 =  L1 code phase (21 bits = 9+12),

                                        * 21-32 = L1 SNR/4096 (upper 11 of 12 bits) */

    unsigned long  ulCarrierCycles_L1; /* 0-23 = L1 carrier cycles,

                                        * 24-32 = L1 Carrier DCO lower 8 bits */

    unsigned long  codeph_SNR_L2;      /* 0-20 =  L2 code phase (21 bits = 9+12),

                                        * 21-32 = L2 SNR/4096 (upper 11 of 12 bits) */

    unsigned long  ulCarrierCycles_L2; /* 0-23 = L2 carrier cycles,

                                        * 24-32 = L2 Carrier DCO lower 8 bits */

} SMsg61Data; /* 24 bytes */

/****************************************************/

  /*  SBinaryMsg61                                    */

  /*  Comment: Transmits data from TakemeasGLONASS.c  */

  /*           debugging structure for Dual Freq.     */

  /****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;                  /* 8 */

    unsigned long    m_Tic;                    /* 4 bytes */

    unsigned long    ulSpare;                  /* 4 bytes */

    unsigned short   awHalfWarns[CHANNELS_12]; /* 12*2 = 24 bytes */

                                               /* each word is

                                                * bit 0-2  L1 Half Cycle Warn

                                                * bit 3 = L1 half cycle added

                                                * bit 4-6  L2 Half Cycle Warn

                                                * bit 7 = L2 half cycle added

                                                * 8 =  LSB of 12 bit L1 SNR/4096

                                                * 9 =  LSB of 12 bit L2 SNR/4096

                                                * bit 10-15 Ktag of the SV */

    SMsg61Data       as61Data[CHANNELS_12];    /* 12*24 = 288 bytes */

    unsigned short   m_wCheckSum;              /* sum of all bytes of the header and data */

    unsigned short   m_wCRLF;                  /* Carriage Return Line Feed */

} SBinaryMsg61;                                /* length = 8 + (320) + 2 + 2 = 332 */      

/****************************************************/

/*  SBinaryMsg66  GLONASS OBS (see notes on mesage 76) */

/****************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    double           m_dTow;                    /* Time in seconds */

    unsigned short   m_wWeek;                   /* GPS Week Number */

    unsigned short   m_wSpare1;                 /* spare 1 (zero)*/

    unsigned long    m_ulSpare2;                /* spare 2 (zero)*/

    SObsPacket       m_asL1Obs[CHANNELS_12];    /* 12 sets of L1(Glonass) observations */

    SObsPacket       m_asL2Obs[CHANNELS_12];    /* 12 sets of L2(Glonass) observations */

    unsigned long    m_aulL1CodeMSBsSlot[CHANNELS_12]; /* array of 12 words.  

                                                    bit 7:0 (8 bits) = satellite Slot, 0

                                                    if no satellite

                                                    bit 12:8 (5 bits) = spare

                                                    bit 31:13 (19 bits) = upper 19 bits

                                                    of L1  LSB = 256 meters

                                                           MSB = 67108864 meters */

    unsigned short   m_wCheckSum;               /* sum of all bytes of the header and data */

    unsigned short   m_wCRLF;                   /* Carriage Return Line Feed */

} SBinaryMsg66;                                 /* length = 8 + (352) + 2 + 2 = 364 */

/****************************************************/

/*  SGLONASS_String, added for glonass strings      */

/****************************************************/

typedef struct

{

   unsigned long m_aul85Bits[3];  /* holds bits 9-85 of the GLONASS string  */

                                  /*

                                   * bit order in message 65

                                   *                MSB             LSB

                                   * m_aul85Bits[0]: 85 84...........54

                                   * m_aul85Bits[1]: 53 52...........22

                                   * m_aul85Bits[2]: 21 20......9

                                   */

} SGLONASS_String;                /* 12 bytes (max of 96 bits) */

/****************************************************/

/*  SBinaryMsg65, added by JL for glonass subframe immediate data + string_5  */

/****************************************************/

/* sent only upon command or when values change (not including changes in tk) */

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned char    m_bySV;                        /* The satellite to which this data belongs. */

    unsigned char    m_byKtag;                      /* The satellite K Number + 8. */

    unsigned short   m_wSpare1;                     /* Spare, keeps alignment to 4 bytes */

    unsigned long    m_ulTimeReceivedInSeconds;     /* time at which this arrived */

    SGLONASS_String  m_asStrings[5];                /* first 5 Strings of Glonass Frame (60 bytes) */

    unsigned short   m_wCheckSum;                   /* sum of all bytes of the header and data */

    unsigned short   m_wCRLF;                       /* Carriage Return Line Feed */

} SBinaryMsg65;                                     /* length = 8 + (68) + 2 + 2 = 80 */

/*********************************************************************/

/*  SBinaryMsg62, Glonass almanac data. Containing string

 *   5 and the two string pair for each satellite after string 5.

 *   String 5 contains the time reference for the glonass almanac

 *   and gps-glonass time differences.

 *

 *********************************************************************/

typedef struct

{

    SUnionMsgHeader  m_sHead;

    unsigned char    m_bySV;                        /* The satellite to which this data belongs. */

    unsigned char    m_byKtag_ch;                   /* Proprietary data */

    unsigned short   m_wSpare1;                     /* Spare, keeps alignment to 4 bytes */

    SGLONASS_String  m_asStrings[3];                /* glonass almanac data  (36 bytes)

                                                       0 & 1 = Two almanac SFs, 3= SF 5*/

    unsigned short   m_wCheckSum;                   /* sum of all bytes of the header and data */

    unsigned short   m_wCRLF;                       /* Carriage Return Line Feed */

} SBinaryMsg62;                                     /* length = 8 + (40) + 2 + 2 = 52 */


//-****************************************************

//-*  SSVSNRData

//-****************************************************

typedef struct

{

    unsigned short m_wStatus_SYS_PRNID; // status, GNSS system, PRN ID

                                        //    Bit 0-5  PRNID  (for SBAS , PRNID = PRN-120)

                                        //    Bit 6-8  SYS: 0 = GPS, 1 = GLONASS, 2 = GALILEO, 3 = BEIDOU,  7 = SBAS

                                        //    Bit 9  = code and Carrier Lock on L1,G1,B1

                                        //    Bit 10 = code and Carrier Lock on L2,G2,B2

                                        //    Bit 11 = code and Carrier Lock on L5,E5,B3

                                        //    Bit 12 = Bit Lock  and Frame lock (decoding data)

                                        //    Bit 13 = Ephemeris Available

                                        //    Bit 14 = Health OK

                                        //    Bit 15 = Satellite used in Navigation Solution

                                        //    m_wStatus_SYS_PRNID = 0 ==> unfilled data

    char          m_chElev;             // Elevation angle, LSB = 1 deg

    unsigned char m_byAzimuth;          // 1/2 the Azimuth angle, LSB = 2 deg

    unsigned long m_ulSNR3_SNR2_SNR1;   // 3 SNRs, 12 bits each SNR = 10.0*log10( 0.1024*SNR_value)

                                        //    Bits 0-11  SNR1  (L1,G1,B1, etc)

                                        //    Bits 12-23 SNR2  (L2,G2,B2, etc)

                                        //    Bits 24-32 SNR3  (L5,E5,B3, etc)

} SSVSNRData;  // 8 bytes

//-****************************************************
//-*  SBinaryMsg209
//-****************************************************
typedef struct
{
    SUnionMsgHeader  m_sHead;              //                                                       [8]
    double           m_dGPSTimeOfWeek;     // GPS tow                                               [8 bytes]
    unsigned short   m_wGPSWeek;           // GPS week                                              [2 bytes]
    char             m_cUTCTimeDiff;       // Whole Seconds between UTC and GPS                     [1 byte]
    unsigned char    m_byPage;             // Bits 0-1 = Antenna: 0 = Master, 1 = Slave, 2 = Slave2 [1 byte]
                                           // Bits 2-4 = Page ID: 0 = page 1, 1 = page 2, etc
                                           // Bits 5-7 = Max page ID: 0 = only 1 page, 1 = 2 pages
    SSVSNRData       m_asSVData[40];       // SNR data                                              [320 bytes]
    unsigned short   m_wCheckSum;          // sum of all bytes of the header and data
    unsigned short   m_wCRLF;              // Carriage Return Line Feed
} SBinaryMsg209; 

#if defined(WIN32) || (__ARMCC_VERSION >= 300441)

    #pragma pack(pop)

#endif

#ifdef __cplusplus

}

#endif

#endif // __BinaryMsg_H_

