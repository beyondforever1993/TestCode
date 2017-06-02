/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: File.c
**创   建   人:
**最后修改日期: 2014年08月12日
**描        述: 文件相关的操作
********************************************************************************************************/

#include "includes.h"

struct FILE g_File;

FIL g_Filed;

static  BSP_OS_SEM   BSP_FileLock;
//static UINT8  OpenFile();
UINT8  CreatHcnFile();
static UINT8  AddTofile(UINT8 *pBuf, UINT16 Len);
static UINT8 head[400];
static FATFS lFatfs[1];
static FILINFO Finfo;

//extern unsigned char MMC_INIT_FLAG;
extern unsigned char static_set;

extern int MCI_disk_initialize(void);
/***************初始化fatfs文件系统*******************/

#define FILE_SEARCH_YEAR_START  2000
#define FILE_SEARCH_YEAR_END    2100

const uint32_t month_acc_norm[12] = {0,2678400, 5097600, 7776000, 10368000, 13046400, 15638400, 18316800, \
20995200, 23587200, 26265600, 28857600};

const uint32_t month_acc_leap[12] = {0,2678400, 5184000, 7862400, 10454400, 13132800, 15724800, 18403200, \
21081600, 23673600, 26352000, 28944000};
//================================= RTU =======================================

UINT8 file_init()
{
  spi_flash_init();
  return 1;
}

UINT8 file_write(struct DATA_STRUCT *buf)
{
  spi_flash_write_data(0,(uint8_t *)buf,sizeof(struct DATA_STRUCT),1);
  
  return 1;
}
/**
return: 0 -> 无数据 1 -> 有数据 
**/
UINT8 file_read_last(struct DATA_STRUCT *buf)
{
  uint8_t res;
  
  res = spi_flash_read_data(0,(uint8_t *)buf,sizeof(struct DATA_STRUCT),1);
  
  if( res == 0)
  {
    return 1;
  }
  
  return 0;
}
/**
addr: 字节地址
**/
UINT8 file_read_by_addr(struct DATA_STRUCT *buf, UINT32 addr)
{
  spi_flash_read_data(addr,(uint8_t *)buf,sizeof(struct DATA_STRUCT),2);
  
  return 1;
}
/**
return: 0 -> 失败 1 -> 找到年   2 -> 找到月 3 -> 找到日 4 -> 找到时 5 -> 找到分 6->找到秒

        失败情况的处理：返回离此值最近的一个有效值 ^-^
       
**/
UINT8 file_get_addr_by_time(UINT32 * addr, struct TIME_STRUCT *tm)
{
  uint32_t i,tmp_diff;
  struct TIME_STRUCT buf[1];
  uint8_t flag ;
  uint32_t cmp_low,cmp_mid,cmp_high;// 2分查找
  
  
  if( tm->y > FILE_SEARCH_YEAR_END || tm->y < FILE_SEARCH_YEAR_START)
  {
    return 0;
  }
  
  *addr = NULL;
  
  /** 第一部分: 0 - g_RtuConfig.addr_wr **/
  
  flag = 0;
  
  cmp_low = 0;
  
  cmp_high = g_RtuConfig.addr_wr/sizeof (struct DATA_STRUCT );
  
  if( cmp_high > 0)
  {
    cmp_high -= 1;
  }
  else
  {
    flag = 3;
  }
  
  cmp_mid = (cmp_low + cmp_high)/2;
  
  spi_flash_read_data(cmp_low * sizeof (struct DATA_STRUCT ),(uint8_t *)buf,sizeof(struct TIME_STRUCT),2);
  
  if( time_cmp(tm,buf) > 0)
  {
    flag++;
  }
    
  spi_flash_read_data(cmp_high * sizeof (struct DATA_STRUCT ),(uint8_t *)buf,sizeof(struct TIME_STRUCT),2);
  
  if( time_cmp(tm,buf) < 0)
    flag++;
 
  if(flag != 2)
  {
    
    /** 第二部分: g_RtuConfig.addr_wr - 300*1000*sizeof(struct DATA_STRUCT) **/
    
    flag = 0;
    
    cmp_low = g_RtuConfig.addr_wr/sizeof (struct DATA_STRUCT );
    
    cmp_high = 300 * 1000 - 1;
    
    cmp_mid = (cmp_low + cmp_high)/2;
    
    spi_flash_read_data(cmp_low * sizeof (struct DATA_STRUCT ),(uint8_t *)buf,sizeof(struct TIME_STRUCT),2);
    
    if( (buf->y >= 2015) && (buf->y <= 2100))
    {
      if( time_cmp(tm,buf) > 0)
      flag++;
    }
    
    spi_flash_read_data(cmp_high * sizeof (struct DATA_STRUCT ),(uint8_t *)buf,sizeof(struct TIME_STRUCT),2);
    
    if( (buf->y >= 2015) && (buf->y <= 2100))
    {
      if( time_cmp(tm,buf) < 0)
        flag++;
    }
  }
  
  if( flag != 2)
  {// 存储区中不存在,返回最接近的一个有效值 
    
    
    
    spi_flash_read_data(0,(uint8_t *)buf,sizeof(struct TIME_STRUCT),2);
    
    if( (buf->y >= FILE_SEARCH_YEAR_START) && (buf->y <= FILE_SEARCH_YEAR_END) )
    {
      *addr = 0;
      
      tmp_diff = abs(time_cmp(tm,buf));
    }
    else
    {
      tmp_diff = 0xffffffff;
      *addr = 0xffffffff;
    }
    
    spi_flash_read_data(g_RtuConfig.addr_wr,(uint8_t *)buf,sizeof(struct TIME_STRUCT),2);
    
    if( (buf->y >= FILE_SEARCH_YEAR_START) && (buf->y <= FILE_SEARCH_YEAR_END) )
    {
      if( tmp_diff > abs(time_cmp(tm,buf)) )
      {
        *addr = g_RtuConfig.addr_wr;
        
        tmp_diff = abs(time_cmp(tm,buf));
      }
    }
    
    spi_flash_read_data(g_RtuConfig.addr_wr-sizeof(struct DATA_STRUCT),(uint8_t *)buf,sizeof(struct TIME_STRUCT),2);
    
    if( (buf->y >= FILE_SEARCH_YEAR_START) && (buf->y <= FILE_SEARCH_YEAR_END) )
    {
      if( tmp_diff > abs(time_cmp(tm,buf)) )
      {
        *addr = g_RtuConfig.addr_wr-sizeof(struct DATA_STRUCT);
        
        tmp_diff = abs(time_cmp(tm,buf));
      }
    }
                       
    spi_flash_read_data((300*1000-1) * sizeof (struct DATA_STRUCT ),(uint8_t *)buf,sizeof(struct TIME_STRUCT),2);
    
    if( (buf->y >= FILE_SEARCH_YEAR_START) && (buf->y <= FILE_SEARCH_YEAR_END) )
    {
      if( tmp_diff > abs(time_cmp(tm,buf)) )
      {
        *addr = (300*1000-1) * sizeof (struct DATA_STRUCT );
      }
    }

    //DebugMsg("result :0\r\n");
    
    return 0;
  }
  
  while( cmp_low < cmp_high)
  {
    spi_flash_read_data(cmp_mid * sizeof (struct DATA_STRUCT ),(uint8_t *)buf,sizeof(struct TIME_STRUCT),2);
    
    if( time_cmp(tm,buf) > 0)
    {
      cmp_low = cmp_mid;
      
      cmp_mid = (cmp_low + cmp_high + 1)/2;
      
    }
    else if( time_cmp(tm,buf) < 0)
    {
      if( cmp_high == cmp_mid)
      {
        cmp_high--;
      }
      else
      {
        cmp_high = cmp_mid;
      }
      
      cmp_mid = (cmp_low + cmp_high + 1)/2;
    }
    else 
    {
      break;
    }
  }
  
  *addr = cmp_mid * sizeof (struct DATA_STRUCT );
  
  if( tm->y != buf->y)
  {
   // DebugMsg("result :0x\r\n");
    return 0;
  }
  
  if( tm->m != buf->m)
  {
    //DebugMsg("result :1\r\n");
    return 1;
  }
  
  if( tm->d != buf->d)
  {
   // DebugMsg("result :2\r\n");
    return 2;
  }
  
    if( tm->H != buf->H)
  {
    //DebugMsg("result :3\r\n");
    return 3;
  }
  
    if( tm->M != buf->M)
  {
   // DebugMsg("result :4\r\n");
    return 4;
  }
  
  
  if( tm->S != buf->S)
  {
   // DebugMsg("result :5\r\n");
    return 5;
  }

//  DebugMsg("result :6\r\n");
  return 6; //查找到年月日时分
}

/**
@return: (tm1-tm2) 单位：秒
**/
int32_t time_cmp(struct TIME_STRUCT *tm1,struct TIME_STRUCT *tm2)
{
  uint32_t tm1_cnt,tm2_cnt;
  
  uint8_t leap_flag;
  
  leap_flag = ((tm1->y % 4) == 0) && ((tm1->y % 100) != 0);
  
  
  tm1_cnt = (tm1->y - FILE_SEARCH_YEAR_START)*(3600*24*365)+\
      (tm1->d - 1)*(3600*24)+(tm1->H * 3600)+(tm1->M *60)+tm1->S;
  
  if( leap_flag )
  {
      tm1_cnt += month_acc_leap[tm1->m -1];
  }
  else
  {
      tm1_cnt += month_acc_norm[tm1->m -1];
  }
  
  leap_flag = ((tm2->y % 4) == 0) && ((tm2->y % 100) != 0);
  
  if (tm2->y < FILE_SEARCH_YEAR_START)
  {
    tm2->y = FILE_SEARCH_YEAR_START;
  }
  tm2_cnt = (tm2->y - FILE_SEARCH_YEAR_START)*(3600*24*365)+\
      (tm2->d - 1)*(3600*24)+(tm2->H * 3600)+(tm2->M *60)+tm2->S;
  
  
  if( leap_flag)
  {
      tm2_cnt += month_acc_leap[tm2->m -1];
  }
  else
  {
      tm2_cnt += month_acc_norm[tm2->m -1];
  }
  
  return tm1_cnt-tm2_cnt;
}




static UINT8 OpenFile(void)
{
  //char temp[200],temp1[50];
  //SINT8 fmt ='f';
  // UINT16 HeadLen = 0;
  FILINFO fno;
  FRESULT res;
  
  char dir[20]  = "hcrtu";
  char name[20] = "hcfile";
  char path[20];
  
  if( f_stat(dir,&fno) != FR_OK)//文件夹不存在
  {
    res = f_mkdir (dir);
    
    if( res != FR_OK)
    {
      //DebugMsg("Create dir err,err ID:%d\r\n",res);
      return 0;//文件夹建立失败
    }
    
    sprintf(path, "/%s/%s", dir, name);
    res = f_open (&g_Filed, path, (FA_READ|FA_WRITE));
    if (res != FR_OK)
    {
      res = f_open (&g_Filed, path, (FA_CREATE_ALWAYS|FA_READ|FA_WRITE));
      if( res == FR_OK)
      {
        DebugMsg("Open File OK\r\n");
      }
      else
      {
        DebugMsg("Open File ERR !!!\r\n");
        return 0; 
      }
    }
    else
    {
      DebugMsg("Open File OK 2 \r\n");
      //f_close(&g_File.File);
    }
    
  }
  
  // AddTofile(head, HeadLen);
  
  return 1;
}


void InitFS(void)
{
  UINT8 res;
  
  // FormatSDCard();
  
  MCI_disk_initialize();
  
  // FormatSDCard();
  
  res = f_mount(&lFatfs[0],"",1);
  if (res != FR_OK)
  {
    DebugMsg("Fatfs init err !!!\r\n");
  }
  
  res = OpenFile();
  if (res != 1)
  {
    DebugMsg("OpenFile err !!!\r\n");
  }
  
}
/******************格式化SD卡***********************/
void FormatSDCard(void)//add by xxw 20141010
{
  UINT8 res;
  DebugMsg("Now begin to format SD card...\r\n");
  res = f_mkfs(0,1,8192);
  if(res)
  {
    //DebugMsg("Format SD card failed,error code:%d\r\n",res);
    return ;
  }
  else
  {
    DebugMsg("Format SD card OK\r\n");
  }
}
void ListFiles()
{
  DIR dir;
  UINT8 res;
  if(g_File.bSDState == 1)
  {
    DebugMsg("SD card err !!!\r\n");
    return;
  }
  res = f_opendir(&dir, "/");
  if(res)
  {
    //DebugMsg("Failed to open /: %d \r\n", res);
  }
  DebugMsg("Listing content of '/'\r\n");
  for(;;)
  {
    res = f_readdir(&dir, &Finfo);
    if((res != FR_OK) || !Finfo.fname[0])
      break;
    DebugMsg(&(Finfo.fname[0]));
    DebugMsg("\r\n");
  }
}
/*****************获取当前SD卡剩余容量*****************/
/**
获取当前SD卡剩余容量，放入g_File.FreeSize中
以MB为单位
**/
void GetFreeSize(void)
{
  DWORD ClustersNum;
  UINT32 SectorsPerCluster;
  UINT32 BytesPerSector;
  
  if(g_File.bSDState == 1)
  {
    DebugMsg("SD card err !!!");
    return;
  }
  FATFS *pFatFs;
  pFatFs = &lFatfs[0];
  f_getfree("/", &ClustersNum, &pFatFs);
  SectorsPerCluster = pFatFs->csize;
  BytesPerSector = 512;
  g_File.FreeSize = ClustersNum * SectorsPerCluster * BytesPerSector / 1024 /1024;
}



/**
建立日期文件夹并打开静态文件
0:文件建立或打开失败
1:文件建立成功
2:文件头写入失败
3:
**/
UINT8  CreatHcnFile(void)
{
  char temp[200],temp1[50];
  SINT8 fmt ='f';
  UINT16 HeadLen = 0;
  FILINFO fno;
  FRESULT res;
  
  /** 关闭原来的文件 **/
  
  f_close(&g_File.File);
  
  /** 建立日期文件夹  **/
  sprintf(g_File.dir, "%04d%02d%02d", g_Gps.y, g_Gps.m, g_Gps.d );
  
  memset(&fno,0,sizeof(fno));
  
  if( f_stat(g_File.dir,&fno) != FR_OK)
  {//文件夹不存在
    res = f_mkdir (g_File.dir);
    
    if( res != FR_OK)
    {
      //DebugMsg("Create dir err,err ID:%d\r\n",res);
      return 0;//文件夹建立失败
    }
    
    /** 新建的记录文件名从 A0开始 **/
    g_File.tail1 = 'A';
    g_File.tail2 = 0;
    
  }
  else
  {
    g_File.tail2 ++;
    if(g_File.tail2 > 9)
    {
      g_File.tail2 = 0;
      g_File.tail1 ++;
      if(g_File.tail1 > 'Z')
        g_File.tail1 = 'A';
    }
  }
  
  /** 创建新文件 **/
  if(static_set == 1)
  {
    sprintf(g_File.name, "/%s/%s%03d%c%d.HCN", g_File.dir,Gps_name, g_Gps.ds, g_File.tail1, g_File.tail2);
  }
  else
  {
    sprintf(g_File.name, "/%s/%s%03d%c%d.HCN", g_File.dir,g_Para.sID, g_Gps.ds, g_File.tail1, g_File.tail2);
  }
  
  res = f_open (&g_File.File, g_File.name, (FA_READ|FA_WRITE));
  
  if (res != FR_OK)
  {
    res = f_open (&g_File.File, g_File.name, (FA_CREATE_ALWAYS|FA_READ|FA_WRITE));
    if( res == FR_OK)
    {
      //DebugMsg("Open File %s OK,File Length:%d\r\n",g_File.name,g_Byte128[43]);
      f_close(&g_File.File);
    }
    else
    {
      return 0; //新建文件错误
    }
  }
  else
  {
    //DebugMsg("Open File %s OK,File Length:%d\r\n",g_File.name,g_Byte128[43]);
    f_close(&g_File.File);
  }
  g_File.ds = g_Gps.ds;
  g_File.StartSecond = g_Gps.Second;
  
  /** 保存 新建的文件名到 EEPROM -------------------------------------------*/
  
  /** 从1月1日算起的天数 **/
  g_Byte128[131] = g_File.ds>>8;
  g_Byte128[132] = g_File.ds;
  
  /** 编号 A -- Z **/
  g_Byte128[133] = g_File.tail1;
  
  /** 编号 0 -- 9 **/
  g_Byte128[134] = g_File.tail2;
  
  WriteFlash();
  
  /** 写入文件头 -----------------------------------------------------------*/
  
  memset(head,0,400);
  
  /** 华测标志 **/
  sprintf(temp, "HUACENAV COLLECTED DATA FILE\r\n");
  memcpy( head, temp, strlen(temp));
  
  HeadLen = strlen(temp);
  
  /** OEM 型号 有Ver和ver之分 **/
  
  if(g_Para.OemType == BD970)
    sprintf(temp, "ver 60.0\r\n");
  else
    sprintf(temp, "ver 54.0\r\n");
  
  
  if( HeadLen + strlen(temp) > sizeof(head))
  {
    return 2; //文件头过长
  }
  
  memcpy( head+HeadLen, temp, strlen(temp));
  
  HeadLen += strlen(temp);
  
  /** 接收机ID **/
  sprintf(temp, "ReceiverID:%s\r\n", g_Para.sID);
  
  if( HeadLen + strlen(temp) > sizeof(head))
  {
    return 2; //文件头过长
  }
  
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 观测时间 **/
  sprintf(temp, "Date:%4d/%d/%d\r\n", g_Gps.y, g_Gps.m, g_Gps.d);
  
  if( HeadLen + strlen(temp) > sizeof(head))
  {
    return 2; //文件头过长
  }
  
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 仪器型号 **/
  sprintf(temp, "Model:%s\r\n", g_Para.sType);	
  
  if( HeadLen + strlen(temp) > sizeof(head))
  {
    return 2; //文件头过长
  }
  
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 天线高 **/
  cfltcvt( g_Gps.fAntLength, temp1, fmt, 3);
  sprintf(temp, "AntHigh:%s\r\n", temp1);	
  
  if( HeadLen + strlen(temp) > sizeof(head))
  {
    return 2; //文件头过长
  }
  
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 天线类型 **/
  sprintf(temp, "AntType:%d\r\n", g_Gps.AntType);	
  
  if( HeadLen + strlen(temp) > sizeof(head))
  {
    return 2; //文件头过长
  }
  
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 测量到 **/
  sprintf(temp, "MeasureTo:%d\r\n", g_Gps.MeasureTo);
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 空间直角坐标 **/
  sprintf(temp, "X:%.4f\r\nY:%.4f\r\nZ:%.4f\r\n",g_Gps.EcefPostion[0],g_Gps.EcefPostion[1],g_Gps.EcefPostion[2]);
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 点名 **/
  if(static_set == 1)
    sprintf(temp, "MARKER NAME:%s\r\n",Gps_name);
  else
    sprintf(temp, "MARKER NAME:\r\n");
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 点号 **/
  if(static_set == 1)
    sprintf(temp, "MARKER NUMBER:\r\n");
  else
    sprintf(temp, "MARKER NUMBER:\r\n");
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 接收机号 **/
  sprintf(temp, "REC #:%s\r\n",g_Para.sID);
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 接收机类型 **/
  sprintf(temp, "REC TYPE:%s\r\n",g_Para.sType);
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 固件版本号 **/
  sprintf(temp, "REC VERS:%s\r\n",g_FirmwareVersion);
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 天线号 **/
  sprintf(temp, "ANT #:%s\r\n",g_Para.sID);
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 天线类型 **/
  sprintf(temp, "ANT TYPE:\r\n");
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 采样间隔 **/
  switch(g_Gps.g_bSampleInterval)
  {
  case 252://10Hz
    sprintf(temp, "INTERVAL:0.1\r\n");
    break;
  case 251://5Hz
    sprintf(temp, "INTERVAL:0.2\r\n");
    break;
  case 250://2Hz
    sprintf(temp, "INTERVAL:0.5\r\n");
    break;
  case 1://1s
    sprintf(temp, "INTERVAL:1.0\r\n");
    break;
  case 2://2s
    sprintf(temp, "INTERVAL:2.0\r\n");
    break;
  case 5://5s
    sprintf(temp, "INTERVAL:5.0\r\n");
    break;
  case 10://10s
    sprintf(temp, "INTERVAL:10.0\r\n");
    break;
  case 15://15s
    sprintf(temp, "INTERVAL:15.0\r\n");
    break;
  case 30://30s
    sprintf(temp, "INTERVAL:30.0\r\n");
    break;
  case 60://60s
    sprintf(temp, "INTERVAL:60.0\r\n");
    break;
  case 253://20Hz
  case 254://50Hz
  default:
    sprintf(temp, "INTERVAL:1.0\r\n");
    break;
  }
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 第一个观测值历元（GPST） **/
  sprintf(temp, "TIME OF FIRST OBS:%04d/%02d/%02d/%02d/%02d/%d.%03d0000\r\n",
          g_Gps.y,g_Gps.m,g_Gps.d,g_Para.FirstObsSeconds/3600%24,g_Para.FirstObsSeconds%3600/60,g_Para.FirstObsSeconds%3600%60,g_Para.FirstObsSecondsPoint);
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  /** 跳秒 **/
  sprintf(temp, "LEAP SECONDS:%d\r\n",g_Para.LeapSeconds);
  if( HeadLen > sizeof(head))
  {
    return 2; //文件头过长
  }
  memcpy( head+HeadLen, temp, strlen(temp));
  HeadLen += strlen(temp);
  
  //DebugMsg("Head Length:%d\r\n",HeadLen);
  AddTofile(head, HeadLen);
  
  return 1;
}

/******************关闭静态文件*********************/
void CloseFile(void)
{
  //  BSP_OS_SemWait(&BSP_FileLock, 0);
  //
  //  memset(&g_File.File,0,sizeof(g_File.File));
  //
  //  BSP_OS_SemPost(&BSP_FileLock);
  //
  //  DebugMsg("Close file OK ...\r\n");
  
  //res =  f_close (&g_File.File);
  //不使用f_close是因为有时候f_close会直接导致单片机死掉
}

/**
写入数据到静态文件
0:失败
1:成功
**/
static UINT8 AddTofile(UINT8 *pBuf, UINT16 Len)
{
  UINT32 numWritten;
  FRESULT res;
  FIL * fp = &g_File.File;
  char * fname = g_File.name;
  uint16_t btw;
  
  BSP_OS_SemWait(&BSP_FileLock, 0);
  
  /** 打开文件 **/
  res = f_open (fp, fname, FA_READ|FA_WRITE);
  
  if( res != FR_OK)
  {
    BSP_OS_SemPost(&BSP_FileLock);
    //DebugMsg("Open File %s err,err ID:%d \r\n",fname,res);
    return 0;
  }
  
  f_lseek(fp,f_size(fp));
  
  while(Len > 0)
  {
    
    btw = (Len > 4096) ? 4096 : Len;
    
    res = f_write (fp, pBuf, btw, &numWritten);
    
    if( numWritten != btw)
    {//写入失败
      f_close(fp);
      BSP_OS_SemPost(&BSP_FileLock);
      //DebugMsg("Write File err,err ID:%d \r\n",res);
      return 0;
    }
    
    pBuf += btw;
    Len -= btw;
  }
  
  /** 关闭文件 **/
  f_close(fp);
  
  BSP_OS_SemPost(&BSP_FileLock);
  
  return 1; //写入成功
}