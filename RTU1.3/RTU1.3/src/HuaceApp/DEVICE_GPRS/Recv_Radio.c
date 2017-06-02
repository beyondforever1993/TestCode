/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  �Ϻ����⵼���Ƽ����޹�˾
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: Recv_Radio.c
**��   ��   ��:
**����޸�����: 2014��08��12��
**��        ��: ��̨����������̨��ʼ�����������ݴ����������ݴ�����̨�����
********************************************************************************************************/

#include "includes.h"




//Add by xhz 2011.8.19
//��̨�������ݴ�������
unsigned  char Radio_Data_Process_Buffer[Radio_Data_Process_Buffer_Size];
unsigned  short Radio_Data_Len = 0;
unsigned  short Radio_Data_RdSp = 0;



#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 �ŵ���Χ32k��SRAM��ȥ
unsigned char g_aSPI_Data_Buf[256];
unsigned char b1,b2;
//unsigned char buf_sp=0,buf_sp2=0; //  д����ָ��
unsigned char buf_sp=0,buf_sp2=0; //  д����ָ��
unsigned char buf2_full=0 ;
// unsigned int  Radio_Data_Len=0;
unsigned short rec_temp_sp=0;
unsigned char head_check_flag = 0;

unsigned char Radio_Type;

unsigned char head_flag=0;
unsigned char cc_num=0,bshift=0;
unsigned char rec_buf1[12][3];
unsigned short rec_buf2[20];
unsigned short dat_length=0;
unsigned char rec_temp[31];	 //���ܵ���SPI����






//========================================================trimble==============================================================
#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 �ŵ���Χ32k��SRAM��ȥ
unsigned char input_data[460];  			   

unsigned short data_length = 0;












unsigned char group_change_last_flag;
unsigned char Radio_New_Flag = 0; //�°�radio��־		 //2010.9.22
unsigned char Low_Frequence_Radio = 0;//�ߵ�Ƶ��̨����  0 -��Ƶ     1-��Ƶ	 	 	  //2010.9.26




//�����̨���������־
unsigned char TR_RADIO_Set_Flag = 0;



#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 �ŵ���Χ32k��SRAM��ȥ


//�������ַ��������ĸߵ�ƽ��͵�ƽ��ʱ��Ϊ 3 * Time * _nop_()
void Delay(unsigned short Time)
{
    unsigned short i = 0;
    for(i = 0; i< 11*Time; i++)
    {
    }
}






void shift(unsigned char times)
{
    unsigned char i;

    for(i = 0; i < times; i++)	 //at most 7 time for a byte
    {
        if((b1 & 0x80) == 0x80)//b0 high -b1 low
        {
            b2 <<= 1;	//high bit of byte
            b2 |= 0x01;	  //?
            b1 <<= 1;	//remain bits
        }
        else
        {
            b2 <<= 1;	//high bit of byte
            b1 <<= 1;	//remain bits
        }	
    }
}





