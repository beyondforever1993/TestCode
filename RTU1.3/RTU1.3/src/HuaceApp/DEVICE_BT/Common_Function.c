/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  �Ϻ����⵼���Ƽ����޹�˾
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: Common_Function.c
**��   ��   ��: ���ܿ�
**����޸�����: 2004��06��17��
**��        ��: һЩʵ�õ�С����������ȡint�͵ĸߵ��ֽڣ����ݿ�����У�飬��ʱ��
********************************************************************************************************/

#define __COMMON_FUNCTION_GLOBAL
#include "includes.h"

unsigned char Low_Byte(unsigned int Uint_Value)
{
    Uint_Value = Uint_Value&0x00ff;
    return Uint_Value;

}

unsigned char High_Byte(unsigned int Uint_Value)
{
	//Uint_Value = Uint_Value&0xff00;	
    Uint_Value = Uint_Value >>8;
    return Uint_Value;
}

unsigned int Int_Low_Bytes_First(unsigned char *bytes)
{
    unsigned int Uint;
    Uint = *(bytes+1)<<8;
    Uint = Uint+(*bytes);
    return Uint;
}

unsigned int Int_High_Bytes_First(unsigned char *bytes)
{
    unsigned int Uint;
    Uint = *(bytes)<<8;
    Uint = Uint+*(bytes+1);
    return Uint;
}

void Clear_Data_Buffer(unsigned char *databuf,unsigned char length)
{
    unsigned char i;
    if(length == 0)
        return ;
    for(i=0; i<length; i++)
        *(databuf + i) = 0;
}
/*
for F34O DelayMs(1) delays about  0.1ms
DelayMs(10) delays about 1ms
*/
void DelayMs(unsigned int ch)
{
    unsigned int i;
    while(ch)
    {
        for(i=0;i<80;i++);			
        //PCA0CPH4 = 0x80;
        ch--;
    }
}

void Data_Copy(unsigned char *Destination,unsigned char *Source,unsigned int Length)
{
    unsigned int  i;
    for(i=0;i<Length;i++)
    {
        *(Destination + i )= *(Source + i );
    }
}

/*-------------����У�������---------------------------*/
unsigned char Checked_Data_Create(unsigned char *p,unsigned char length)
{
    unsigned char Checked_Data = 0,i;
    for(i=0;i<length;i++)
    {
        Checked_Data ^= *( p+i );
    }
    return Checked_Data;

}

/*-------------F320 ���ݽ��ո�ʽУ��---------------------------*/
unsigned char  Huace_Data_Check(unsigned char *Data)
{
    unsigned char uSum;		//Checksum
    unsigned char uLength;//Data Length
    unsigned char i;

    uSum = 0;
    uLength = 0;
    if((*Data == 'V') && ( *(Data+1) == 'V' ))
        return 1;
    if( (*Data != 0x24) || ( *(Data+1) != 0x24 ) )  //0x24="$"
        return 0;
    uLength = (*(Data+5))&0x3F;
    uLength = uLength+4;   //У�鳤��

    for(i=0;i<uLength;i++)
        uSum ^= *(Data+2+i);

    if( uSum != *(Data+2+uLength) )
    {
        return 0;
    }
    if( *(Data+3+uLength) != 0x0D)  //"\r"
    {
        return 0;
    }
    if( *(Data+4+uLength) != 0x0A)  //"\n"
    {
        return 0;
    }
    return 1;
}

