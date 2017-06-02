#ifndef __COMMON_FUNCTION_H
#define __COMMON_FUNCTION_H


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __COMMON_FUNCTION_GLOBAL
#define COMMFUN_EXT
#else
#define COMMFUN_EXT extern
#endif

    COMMFUN_EXT unsigned char Low_Byte(unsigned int Uint_Value);

    COMMFUN_EXT unsigned char High_Byte(unsigned int Uint_Value);

    COMMFUN_EXT unsigned int Int_High_Bytes_First(unsigned char *bytes);

    COMMFUN_EXT unsigned int Int_Low_Bytes_First(unsigned char *bytes);

    COMMFUN_EXT void Clear_Data_Buffer(unsigned char *databuf,unsigned char length);
    COMMFUN_EXT void DelayMs(unsigned int ch);
    COMMFUN_EXT unsigned char Checked_Data_Create(unsigned char *p,unsigned char length) ;
    COMMFUN_EXT unsigned char  Huace_Data_Check(unsigned char *Data) ;
    COMMFUN_EXT void Data_Copy(unsigned char *Destination,unsigned char *Source,unsigned int Length)  ;

#ifdef __cplusplus
}
#endif

#endif