/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: Recv_Radio.c
**创   建   人:
**最后修改日期: 2014年08月12日
**描        述: 电台处理，包括电台初始化，接收数据处理，发送数据处理，电台分类等
********************************************************************************************************/

#include "includes.h"




//Add by xhz 2011.8.19
//电台接受数据处理缓冲区
unsigned  char Radio_Data_Process_Buffer[Radio_Data_Process_Buffer_Size];
unsigned  short Radio_Data_Len = 0;
unsigned  short Radio_Data_RdSp = 0;



#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去
unsigned char g_aSPI_Data_Buf[256];
unsigned char b1,b2;
//unsigned char buf_sp=0,buf_sp2=0; //  写，读指针
unsigned char buf_sp=0,buf_sp2=0; //  写，读指针
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
unsigned char rec_temp[31];	 //接受到的SPI数据






//========================================================trimble==============================================================
#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去
unsigned char input_data[460];  			   

unsigned short data_length = 0;












unsigned char group_change_last_flag;
unsigned char Radio_New_Flag = 0; //新版radio标志		 //2010.9.22
unsigned char Low_Frequence_Radio = 0;//高低频电台区分  0 -高频     1-低频	 	 	  //2010.9.26




//发射电台设置命令标志
unsigned char TR_RADIO_Set_Flag = 0;



#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去


//采用这种方法产生的高电平或低电平的时间为 3 * Time * _nop_()
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





