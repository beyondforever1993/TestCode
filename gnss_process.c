#include <common.h>
#include "gnss_trimble.h"
#include "gnss_p307.h"
#include "gnss_buffer.h"
#include "sensors.h"
#include "conf.h"
#include "gnss_process.h"
#include "3des.h"
#include "data_dispatch.h"
#include <curl/curl.h>
#include "rsa_encrypt.h"
#include "utils.h"
#include <pthread.h>
#include<sys/time.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>  
#include <sys/types.h>  
#include <sys/stat.h>  


#define default_web_service "http://223.72.206.62:10100/Services/DataService.asmx/GetRollingData"
#define package_length 3000
#define DISTANCE_THRESHOLD
#define PAI     3.14159265
#define FE_WGS84			(1.0/298.257223563) /* ~{5XGr1bBJ~} (WGS84) */
#define RE_WGS84			6378137.0			/* ~{5XGr0kVa3$~} (WGS84) (m) */
#define EQUAL(a,b)          (!(a > b) && !(a < b))
struct event gnss_listen_ev = { { 0 } };
struct event monitor_ev = { { 0 } };
struct event monitor_nmea_ev = { { 0 } };
struct event port_monitor_ev = { { 0 } };		//used to monitor port baud rate

#define type_compactionData 1
#define type_gravelData     2


static pthread_mutex_t s_lock_encrypt_thread = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_lock_update_thread = PTHREAD_MUTEX_INITIALIZER;
static gnss_set_config GNSS_SET_CONFIG;
static view_nmea Monitor_NMEA;

static int s_get_gga_flag = 1;
static int s_sat_count = 0;
static int s_fix_status = 0;
static int s_dat_frq_wrong_flg = 0;			//gga data out frq wrong and need reset oem bord
static double s_longitude = 0; // ????
static char s_west_east_longitude = 0;   // 'E', 'W'
static double s_latitude = 0;    // ~{N3~}??
static char s_south_north_latitude = 0;  // 'S', 'N'
static double s_high = 0;
static double s_azimuth = 0;
//static int r_data_head = 0; //gga ~{5DFpJ<~}
//static int w_data_mark = 0; //~{P4V8Uk~}
static char s_zda[500] = { 0 };
static char s_zda_t[500] = { 0 };
static int s_gnss_fd = -1;
static char s_gga[500];
#define s_mix_data_length 8000
static char s_mix_data[s_mix_data_length];
static int s_gnss_get_diff_flag = 0;
static BUFFER_STRUCT* s_buffer = NULL;
static nmea_config NMEA_Config_dest = {{0}};
static double vtg_speed;

//~{<SC\O`9X~}--------------------------------------
static int encrypt_flag = 0;
//static pthread_t  encrypt_thread = 0; 
//~{;l:OJ}>]O`9X~}----------------------------------
static NMEA_TYPE nmea_type_flag;

void encrypt_process_thread( union sigval v );
/*
 x = r*dlat;
 dy = r*cos(lat0)*dlon;
 dist = sqrt(dx*dx + dy*dy)
 r = 6378137.0 ?~{0g~}?~{
e>~}
 dlat = lat1 - lat0 ~{d8$g9g:,e:?7.~}
 dlon = lon1 - lon0 ~{d8$g9g?e:~}?~{7.~}
 dx ?~{???e7 ~}
 dy ~{d8???e7 ~}
 lat0,lon0 ~{g,~} ~{d8 d8 ~}?~{9g:,e:~}?~{ g;e:?~}
 lat1,lon1 ~{g,~} ~{d:d8 ~}?~{9gg:~} ~{e:~}?~{ g;e:?~}
 */

void progress_gga(char *data, int len);
void progress_zda(char *data, int len);
void progress_hdt(char *data, int len);
void progress_vtg(char *data, int len);
void progress_gsv(char *data, int len);
void progress_rmc(char *data, int len);
void progress_gll(char *data, int len);
void progress_gst(char *data, int len);
void progress_gsa(char *data, int len);
void progress_rot(char *data, int len);
void progress_vgk(char *data, int len);
void progress_vhd(char *data, int len);
void progress_avr(char *data, int len);
void progress_ggk(char *data, int len);
void progress_bpq(char *data, int len);
void progress_pjk(char *data, int len);
void progress_pjt(char *data, int len);
void progress_blh(char *data, int len);
void progress_time(char *data, int len);
void progress_dop(char *data, int len);
void progress_grs(char *data, int len);
void progress_gns(char *data, int len);
int NMEA_data_extract(char *p_sourse,char *p_head, char *p_tail,char *p_dest);
int DATA_GET(char *p_sourse,char *p_head, char *p_tail,char *p_dest,int *t_offset);
void Set_GNSS_SET_CONFIG(int sno,int para)
{
    switch(sno)
    {
        case 1:
            GNSS_SET_CONFIG.button_main = para;
            break;
        case 2:
            GNSS_SET_CONFIG.msg_type = para;
            break;
        case 3:
            GNSS_SET_CONFIG.freq = para;
            break;
        case 4:
            GNSS_SET_CONFIG.output_type = para;
            break;
        default:
            break;
    }
}

/*
* Function:~{;qH!;l:OJ}>]Jd3v5D~}NMEA~{9&D\EdVC#,2"EP6OJG7qJUFk~}
* Paramer: mask:~{JdHk5D~}NMEA~{@`PMQZBk~}  total_mask:~{4+5]W\5DQZBk~}
* Return:  ~{=SJUW4L,~} 0: ~{JUFkAK~} 1: ~{C;JUFk~} 2:~{JU5=9X<|WVVX845D0|~}  4:mask~{N^P'~}
* History: author:zh   date:2017.01.19
*/
int mixed_nmea_get_status(int mask, int *total_mask ,int status_init_flag)
{
    mixed_data_config standard;
    static int nmea_mask_status = 0;
    get_mixed_data_conf(&standard);
    *total_mask = standard.data_mask;
    standard.data_mask |= BIT_GGA;
    standard.data_mask |= BIT_ZDA;
    standard.data_mask |= BIT_VTG;
    standard.data_mask |= BIT_AVR;
    if (mask)
    {
        if (status_init_flag)
        {
        	loge("nmea_mask_status = 0\r\n");
            nmea_mask_status = 0;
        }
        loge("nmea_mask_status :%d,mask :%d\r\n",nmea_mask_status,mask);
        nmea_mask_status |= mask;
        loge("standard data_mask:%d,freq:%d nmea_mask_status :%d\r\n",standard.data_mask,standard.freq,nmea_mask_status);
        if (nmea_mask_status == standard.data_mask)
        {
            nmea_mask_status = 0;
            return 0;
        }
        else
        {
            if ((nmea_mask_status & standard.data_mask) == standard.data_mask)
            {
                nmea_mask_status = 0;
                return 0;
            }
            else
            {
                return 1;
            }
        }
        if ((nmea_mask_status & mask))
        {
            return 2;
        }
    }
    return 4;
}

/*
* Function:~{4S~}ZDA~{VP;qH!WT~}1900~{Dj?*J<5D@[<FCk~} ~{5%N;Ck~}
* Paramer: p_zda:ZDA~{1(ND~} m_sec:~{:ACkV5~} tt1:~{WT~}1900~{Dj?*J<5D@[<FCk~} ~{5%N;:ACk~}
* Return:  1:fail 0:sucess
* History: author:zh   date:2017.01.19 times:~{6~4NU{:O~}
*/
int get_sec_info_from_ZDA(char *p_zda,int *m_sec,time_t *tt1,UTC_TIME *ttime)
{
    int ret;
    int hour,min,sec,msec,day,mouth,year;
    time_t tt;
    UTC_TIME time_temp;
    ret = sscanf( p_zda, "$%*[^,],%2d%2d%2d.%2d,%d,%d,%d,%*d,%*s", &hour, &min, &sec, &msec, &day, &mouth, &year );
	if ( ret != 7 )
	{
	    *tt1 = 0;
		logd( "zda is not vaild %s\n", p_zda );
		return 1;
	}
    
    time_temp.year     = year;
	time_temp.month    = mouth;
	time_temp.day      = day;
	time_temp.hour     = hour;
	time_temp.minute   = min;
	time_temp.sec      = sec;

    ttime->year = year;
    ttime->month = mouth;
    ttime->day = day;
    ttime->hour = hour;
    ttime->minute = min;
    ttime->sec = sec;
    set_zda_time(&time_temp);
    logd("-----------------------------time 1:%4d%02d%02d %2d:%2d:%2d\r\n",year, mouth, day, hour ,min, sec);
	tt = utcdatetotime_t( year, mouth, day, hour, min, sec );
    logd("-----------------------------time 2:%4d%02d%02d %2d:%2d:%2d\r\n",year, mouth, day, hour ,min, sec);
	time_ttoctcdate( tt, &year, &mouth, &day, &hour, &min, &sec );
    logd("-----------------------------time 3:%4d%02d%02d %2d:%2d:%2d\r\n",year, mouth, day, hour ,min, sec);
    *m_sec = msec;
    *tt1 = tt;
    return 0;
}


/*
* Function:~{<r5%~}NMEA~{J}>]5D4&@m<0W*7"~}
* Paramer: nmea_set: NMEA~{1(ND@`PM~}   p: ~{4S0e?(6KJU5=5D~}NMEA~{J}>]~}
* Return:  none
* History: author:zh   date:2017.01.19
*/
void simple_nmea_process(int nmea_set,char *p)
{
    switch(nmea_set)
    {
        case DAT_ID_GGA:progress_gga(p, strlen(p));break;
        case DAT_ID_GSV:progress_gsv(p, strlen(p));break;
        case DAT_ID_RMC:progress_rmc(p, strlen(p));break;
        case DAT_ID_GLL:progress_gll(p, strlen(p));break;
        case DAT_ID_VTG:progress_vtg(p, strlen(p));break;
        case DAT_ID_ZDA:progress_zda(p, strlen(p));break;
        case DAT_ID_GST:progress_gst(p, strlen(p));break;
        case DAT_ID_GSA:progress_gsa(p, strlen(p));break;
        case DAT_ID_HDT:progress_hdt(p, strlen(p));break;
        case DAT_ID_ROT:progress_rot(p, strlen(p));break;
        case DAT_ID_VGK:progress_vgk(p, strlen(p));break;
        case DAT_ID_VHD:progress_vhd(p, strlen(p));break;
        case DAT_ID_AVR:progress_avr(p, strlen(p));break;
        case DAT_ID_GGK:progress_ggk(p, strlen(p));break;
        case DAT_ID_BPQ:progress_bpq(p, strlen(p));break;
        case DAT_ID_PJK:progress_pjk(p, strlen(p));break;
        case DAT_ID_PJT:progress_pjt(p, strlen(p));break;
        default:break;      
    }
}

/*
* Function:~{;qH!4+8PFwJ}>]~}
* Paramer: pdest:~{4+8PFwJ}>]Wi:OMj3I5DJ}>]~} 
* Return:  size : pdest~{5D3$6H~}
* History: author:zh   date:2017.01.19 times:~{6~4NU{:O~}
*/
int get_sensor_data( char *pdest,int pdest_len)
{
    char *temp_handle = NULL;
	char *acm_handle = NULL;
    char *PILERESULT_handle = NULL;
    char *sensor_temp_handle = NULL;
	char *rfid_handle = NULL;
	int *rfid_len_handle = NULL;
	static int no_refresh_temp_count = 0;
	static int no_refresh_acm_count = 0;
    static int no_refresh_PILERESULT_count = 0;
	static int no_refresh_rfid_count = 0;
    static int no_refresh_sensor_temp_count = 0;
    int size = 0;
	if (0 == get_temp_buff(&temp_handle))
	{
		no_refresh_temp_count++;
	}
	else
	{
		no_refresh_temp_count = 0;
	}
	
	if (0 == get_acm_buff(&acm_handle))
	{
		no_refresh_acm_count++;
	}
	else
	{
		no_refresh_acm_count = 0;
	}
	if (0 == get_PILERESULT_buff(&PILERESULT_handle))
    {no_refresh_PILERESULT_count++;}
    else
    {no_refresh_PILERESULT_count = 0;}
	if (0 == get_rfid_buff(&rfid_handle, &rfid_len_handle))
	{
		no_refresh_rfid_count++;
	}
	else
	{
		no_refresh_rfid_count = 0;
	}

    if (0 == get_sensor_temp_buff(&sensor_temp_handle))
    {
        no_refresh_sensor_temp_count++;
    }
    else
    {
        no_refresh_sensor_temp_count = 0;
    }
	size = 0;

	if (no_refresh_temp_count <= 2)
	{
		strcat(pdest, temp_handle);
		size += strlen(temp_handle);
	}
	if (no_refresh_acm_count <= 2)
	{
		strcat(pdest, acm_handle);
		size += strlen(acm_handle);
	}
    if (no_refresh_PILERESULT_count <= 2)//<=2
    {
        if ((strlen(PILERESULT_handle) + size)<pdest_len)
        {
            strcat(pdest, PILERESULT_handle);
		    size += strlen(PILERESULT_handle);
        }
        else
        {
            loge("out of buffer\r\n");
        }
    }
	if (no_refresh_rfid_count <= 2)
	{
		memcpy(pdest + size, rfid_handle, *rfid_len_handle);
		size += *rfid_len_handle;
	}
    if (no_refresh_sensor_temp_count <= 2)
    {
        strcat(pdest + size, sensor_temp_handle);
		size += strlen(sensor_temp_handle);
    }
	//can
	{
		char asd[1024 * 3] = { 0 };
		get_other_buff(asd);
		memcpy(pdest + size, asd, strlen(asd));
		size += strlen(asd);
	}
    if (size)
    {
        if ((size +2) > pdest_len)
        {
            loge("out of buffer\r\n");
            return 0;
        }
        memcpy(pdest + size, "\r\n", 2);
	    size += 2;
        return size;
    }
	else
	{
        memcpy(pdest , "no data\r\n", 9);
        return 0;
    }
    return 0;
}

/*
* Function:~{;l:OJ}>]5D4r0|!"W*7"~}
* Paramer: data : ~{4S0e?(6KJU5=5D~}NMEA~{J}>]~} data_len:~{J}>]3$6H~}
* Return:  1:fail 0:sucess
* History: author:zh   date:2017.01.19
*/
int mixed_nmea_data_process(char *data,int data_len,int total_mask)
{
    time_t tt;
    char sn[20] = {0};
    int head_len = 0;
    //int body_len = 0;
    int body_cal_len = 0;
    int msec,tt_ret;
    char package_data_buf[s_mix_data_length] = {0};
    char sensor_data_buf[1000] = {0};
    char data_exact[180] = {0};
    int data_exact_cal_len = 0;
    int sensor_data_length = 0;
    char message_temp[200] = {0};
    char gga_now[200] = {0};
    int total_gsv,sn_gsv;
    UTC_TIME utc_timex;
    total_gsv = 0xff;
    sn_gsv = 0xfe;
    //~{J1<d4A4S~}ZDA~{VP;qH!~},~{Hg:NLaH!R;Lu1(ND~}(~{;9C;Mj3I~})
    loge("4\r\n");
    if (0 == NMEA_data_extract(data,"ZDA","\r\n",message_temp))
    {
        if (strstr(message_temp, "ZDA"))
        {
            strncpy(s_zda_t, message_temp, strlen(message_temp));
            //loge("view 3\r\n");
            tt_ret = get_sec_info_from_ZDA(message_temp,&msec,&tt,&utc_timex);
            if (tt_ret == 1)
            {
                loge("no real zda\r\n");
                return 1;
            }
        }
        else
        {
            loge("no zda\r\n");
            return 1;
        }
    }
    else
    {
        loge("NMEA_data_extract no zda\r\n");
        return 1;
    }
    //~{LaH!~}GGA 
    //loge("view 1\r\n");
    memset(message_temp,0,sizeof(message_temp));
    if (0 == NMEA_data_extract(data,"GGA","\r\n",message_temp))
    {
        if (strstr(message_temp, "GGA"))
        {        
            memcpy(gga_now,message_temp,strlen(message_temp));
        }
        else
        {
            loge("no GGA\r\n");
            return 1;
        }
    }
    else
    {
        loge("NMEA_data_extract no gga\r\n");
        return 1;
    }
    //~{;qH!~}sn~{:E~}
    get_sn_number( sn );
    //~{993I~}
    head_len = sprintf( package_data_buf, "$%llu$%s", 1000 * ( unsigned long long ) tt+10*msec, sn );
    body_cal_len = head_len;
    //loge("extract total_mask: %u\r\n",total_mask);
    loge("data_len :%d\r\n",data_len);
    int temp_length1 = 0;

    //~{EP6OJG7qPhR*L^3}~}GGA~{:M~}ZDA
    int data_exact_offset_len = 0;
    data_exact_cal_len = 0;
    char time_out = 0;
    char flag_package = 0;
    temp_length1 = body_cal_len;
    while(((data_len - data_exact_cal_len)>4)&&(time_out <30))
    {
         memset(data_exact,0,180);
         if (DATA_GET(data+data_exact_cal_len,"$","\r\n",data_exact,&data_exact_offset_len) == 0)
         {
            data_exact_cal_len += data_exact_offset_len;
             loge("data_exact_offset_len :%d,data_exact_cal_len:%d \r\n",data_exact_offset_len,data_exact_cal_len);
             if (strstr(data_exact, "ZDA"))
             {
                if (total_mask & BIT_ZDA)
                {flag_package = 1;}
                else
                {flag_package = 0;}
             }
             else if (strstr(data_exact, "GGA"))
             {
                if (total_mask & BIT_GGA)
                {flag_package = 1;}
                else
                {flag_package = 0;}
             }
             else if (strstr(data_exact, "AVR"))
             {
                if (total_mask & BIT_AVR)
                {flag_package = 1;}
                else
                {flag_package = 0;}
             }
             else if (strstr(data_exact, "VTG"))
             {
                if (total_mask & BIT_VTG)
                {flag_package = 1;}
                else
                {flag_package = 0;}
             }
             else
             {flag_package = 1;}
             if (flag_package)
             {
                temp_length1 += strlen(data_exact);
                if (temp_length1 < package_length)
                {
                    memcpy(package_data_buf+body_cal_len,data_exact,strlen(data_exact));
                    body_cal_len += strlen(data_exact);
                    logd("exact :%s\r\n",data_exact);
                }
                else
                {
                    loge("out of buffer :%s\r\n",data_exact);
                }
             }
         }
         else
         {
            loge("DATA_GET err\r\n");
         }
         time_out++;
         
    }
    //~{=+;qH!5D4+8PFwJ}>]8=TZ~}NMEA~{J}>]:sCf~}
    sensor_data_length = get_sensor_data(sensor_data_buf,sizeof(sensor_data_buf));
    loge("get sensor length :%d,%s\r\n",sensor_data_length,sensor_data_buf);
    if (sensor_data_length)
    {
        if ((body_cal_len + sensor_data_length)<package_length)
        {
            memcpy(package_data_buf+body_cal_len,sensor_data_buf,sensor_data_length);
            body_cal_len += sensor_data_length;
        }
        else
        {
            loge("out of buffer :%s\r\n",message_temp);
        }
    }
    else
    {
        loge("body_cal_len :%d\r\n",body_cal_len);
        package_data_buf[body_cal_len] = '\r';
        package_data_buf[body_cal_len+1] = '\n';
        //memcpy(package_data_buf+body_cal_len,"\r\n",2);
        body_cal_len += 2;
         loge("body_cal_len1 :%d\r\n",body_cal_len);
    }
    
    //~{OrMxBg7"~}  (~{PhW"Rb>2V97'V5~})
    //~{>2V97'V55DO^VF~}
    int send_pub_data_flag = 0;
    send_pub_data_flag = distance_threshold_process(gga_now,strlen(gga_now));
	encrypt_transmission_set_config encrypt_conf; 
    get_encrypt_conf( &encrypt_conf );
	if (encrypt_conf.cer_status && encrypt_conf.transmission_status)
	{}//正在进行加密传输时，不要把数据往/mnt/record/ 目录下送
	else
	{
		if (send_pub_data_flag)
	    {
	        pub_data_mixed("/mnt/record/",package_data_buf, &utc_timex, body_cal_len );
	    }
	}
    //#if 0
	limit_file_length("/mnt/send_to_uart.txt",limited_file_len);
	FILE *fp1 = NULL;
	fp1 = fopen("/mnt/send_to_uart.txt","a+");//需要被替换
	if (fp1)
	{
	    loge("send_to_uart length : %d\r\n",body_cal_len);
		fwrite("send_to_uart:\r\n",sizeof(char),strlen("send_to_uart:\r\n"),fp1);
		fwrite(package_data_buf,sizeof(char),body_cal_len,fp1);
        fwrite("\r\n\r\n",sizeof(char),6,fp1);\
		fclose(fp1);
	}
	else
	{
		 loge("open %s err \r\n","/mnt/send_to_uart.txt");
	}
    //#endif
    loge("get sensor total :%d,%s\r\n",body_cal_len,package_data_buf);
    //~{4.?ZW*7"~}
    int m,n,i;
    m = body_cal_len%500;
    n = (body_cal_len - m)/500; 
    for (i = 0;i<n;i++)
    {
        add_to_uart_thread_without( package_data_buf+i*500, 500);
    }
    add_to_uart_thread_without( package_data_buf+n*500, m);
    return 0;
}
/*
* Function:~{4SWV7{4.VPLaH!R;6NH76(AKM7:MN25DWV7{4.~}
* Paramer: p_sourse:~{T4J}>]~}  p_head:~{1;LaH!J}>]5DM7~} p_tail:~{1;LaH!J}>]5DN2~} p_dest:~{WnVULaH!5DJ}>]#,6`AK~}($GP) t_offset:~{N20M=OSZM75DF+RF~}
* Return:  1:fail 0:sucess
* History: author:zh   date:2017.01.19
*/
int get_str_from_sourse(char *p_sourse, char *p_head, char *p_tail,char *p_dest,int *t_offset)
{
    char *p1 = NULL;
    char *p2 = NULL;
    char *p3 = NULL;
    if (strlen(p_head) == strlen(p_tail))
    {
        if (strncpy(p_head,p_tail,strlen(p_head)) == 0)
        {
            loge("head is the same as the tail\r\n");
            return 1;
        }
    }
    
    p1 = strstr(p_sourse,p_head);
    if (p1 != NULL)
    {
        p2 = strstr(p1,p_tail);
        loge("a\r\n");
    }
    else 
    {
        loge("no found head : %s\r\n",p_head);
        return 1;
    }
    if (p2 != NULL)
    {
        int temp_mark = (p1-p_sourse)-10;
        loge("b temp_mark:%d\r\n",temp_mark);
        if (temp_mark >0)
        {
             p3 = strstr(p_sourse+temp_mark,"$");
            if (p3 != NULL)
            {
                loge("c ,P2-P3 :%d\r\n",(p2-p3));
                if ((p2-p3)>0)
                {
                    memcpy(p_dest,p3,(p2-p3));//~{U}:C2;R*N20M~}
                    loge("d ,%s\r\n",p_dest);
                    *t_offset = p2-p_sourse;
                    loge("e ,%d\r\n",*t_offset);
                    return 0;
                }
                else
                {
                    return 2;
                }
            }
            else
            {
                loge("lost $ 1 \r\n");
                return 1;
            }
        }
        else
        {
            loge("lost $ \r\n");
            return 1;
        }
    }
    else 
    {
        loge("no found tail : %s\r\n",p_tail);
        return 1;
    }
    return 0;
}


/*
* Function:~{4S~}NMEA~{@oLaH!~}"$xxGGA...\r\n"~{UbVV8qJ=5DJ}>]~}
* Paramer: p_sourse:~{T4J}>]~}  p_head:~{1;LaH!J}>]5DM7~} p_tail:~{1;LaH!J}>]5DN2~} p_dest:~{WnVULaH!5DJ}>]~}
* Return:  1:fail 0:sucess
* History: author:zh   date:2017.02.02
*/
int NMEA_data_extract(char *p_sourse,char *p_head, char *p_tail,char *p_dest)
{
    char header[10] = {0};
    char *p1 = NULL;
    char *p2 = NULL;
    //GP
    strcat(header,"$GP");
    strcat(header,p_head);
    p1 = strstr(p_sourse,header);
    if (p1 != NULL)
    {
        p2 = strstr(p1,p_tail);
        if (p2 != NULL)
        {
            memcpy(p_dest,p1,p2-p1);
            loge("str:%s\r\n",p_dest);
            return 0;
        }
    }
    //BD
    p1 = NULL;
    p2 = NULL;
    memset(header,0,10);
    strcat(header,"$BD");
    strcat(header,p_head);
    p1 = strstr(p_sourse,header);
    if (p1 != NULL)
    {
        p2 = strstr(p1,p_tail);
        if (p2 != NULL)
        {
            memcpy(p_dest,p1,p2-p1);
            loge("str:%s\r\n",p_dest);
            return 0;
        }
    }
    //GL
    p1 = NULL;
    p2 = NULL;
    memset(header,0,10);
    strcat(header,"$GL");
    strcat(header,p_head);
    p1 = strstr(p_sourse,header);
    if (p1 != NULL)
    {
        p2 = strstr(p1,p_tail);
        if (p2 != NULL)
        {
            memcpy(p_dest,p1,p2-p1);
            loge("str:%s\r\n",p_dest);
            return 0;
        }
    }
    //GN
    p1 = NULL;
    p2 = NULL;
    memset(header,0,10);
    strcat(header,"$GN");
    strcat(header,p_head);
    p1 = strstr(p_sourse,header);
    if (p1 != NULL)
    {
        p2 = strstr(p1,p_tail);
        if (p2 != NULL)
        {
            memcpy(p_dest,p1,p2-p1);
            loge("str:%s\r\n",p_dest);
            return 0;
        }
    }
    return 1;
}
/*
* Function:~{4S~}NMEA~{@oLaH!~}"$...\r\n"~{UbVV8qJ=5DJ}>]~}
* Paramer: p_sourse:~{T4J}>]~}  p_head:~{1;LaH!J}>]5DM7~} p_tail:~{1;LaH!J}>]5DN2~} p_dest:~{WnVULaH!5DJ}>]#,6`AK~}($GP) t_offset:~{N20M=OSZM75DF+RF~}
* Return:  1:fail 0:sucess
* History: author:zh   date:2017.02.02
*/
int DATA_GET(char *p_sourse,char *p_head, char *p_tail,char *p_dest,int *t_offset)
{
    char *p1 = NULL;
    char *p2 = NULL;

    p1 = strstr(p_sourse,p_head);
    if (p1 != NULL)
    {
        p2 = strstr(p1,p_tail);
        if (p2 != NULL)
        {
            memcpy(p_dest,p1,p2-p1);
            loge("str1:%s\r\n",p_dest);
            *t_offset = p2 - p_sourse;
            return 0;
        }
    }
    return 1;
}

/*
* Function:~{W[:O~}NMEA~{4&@m:MW*7"~} ~{0|@(;l:OJ}>]5D4r0|!"W*7"!"N4JUFkJ15D4f4"~}
* Paramer: nmea_set: NMEA~{1(ND@`PM~}   p: ~{4S0e?(6KJU5=5D~}NMEA~{J}>]~}
* Return:  none
* History: author:zh   date:2017.01.19
*/
int nmea_data_process(char *p,int nmea_set)
{
    int mixed_ret = 0;
    int nmea_mask = 0;
    int total_mask = 0;
    int length_temp = 0;
    //int w_data_mark_temp = 0;
    static int finish_flag = 1;
    static int package_data_len = 0;
    char data_tempx[200] = {0};
    int total_gsv,gsv_num;
    //int diff_vaule1 = 0;
    //int diff_vaule2 = 0;
    int nmea_status_init_flag = 0;
    get_nmea_type_conf(&nmea_type_flag);
    if (nmea_type_flag.nmea_type == SIMPLE_NMEA)
    {
        simple_nmea_process(nmea_set,p);
        return 1;
    }
    else 
    {
        //~{OH4&@m#,:sEP6O~}
        //~{=bNv~}GGA ,~{Mj3ISkMxR3=;;%:M<SC\~}
        strncpy(data_tempx,p,strlen(p));
        strcat(data_tempx,"\r\n");
        if (strstr(p, "GGA"))
        {
            //~{N*N,3VU}3#5DMxR3OTJ>>-N36H5HR5Nq~} ~{:M~} ~{<SC\R5Nq~}
            gga_info_extract(p,strlen(p));
        }
        else if (strstr(p, "GSV"))
        {
            progress_gsv(p,strlen(p));
        }
        else if (strstr(p, "RMC"))
        {
            progress_rmc(p,strlen(p));
        }
        else if (strstr(p, "GLL"))
        {
            progress_gll(p,strlen(p));
        }
        else if (strstr(p, "VTG"))
        {
            progress_vtg(p,strlen(p));
        }
        else if (strstr(p, "ZDA"))
        {
            progress_zda(p,strlen(p));
        }
        else if (strstr(p, "GST"))
        {
            progress_gst(p,strlen(p));
        }
        else if (strstr(p, "GSA"))
        {
            progress_gsa(p,strlen(p));
        }
        else if (strstr(p, "HDT"))
        {
            progress_hdt(p,strlen(p));
        }
        else if (strstr(p, "ROT"))
        {
            progress_rot(p,strlen(p));
        }
        else if (strstr(p, "VGK"))
        {
            progress_vgk(p,strlen(p));
        }
        else if (strstr(p, "VHD"))
        {
            progress_vhd(p,strlen(p));
        }
        else if (strstr(p, "AVR"))
        {
            progress_avr(p,strlen(p));
        }
        else if (strstr(p, "GGK"))
        {
            progress_ggk(p,strlen(p));
        }
        else if (strstr(p, "BPQ"))
        {
            progress_bpq(p,strlen(p));
        }
        else if (strstr(p, "PJK"))
        {
            progress_pjk(p,strlen(p));
        }
        else if (strstr(p, "PJT"))
        {
            progress_pjt(p,strlen(p));
        }

        
        if (finish_flag)
        {
            if ((length_temp = strlen(data_tempx))<s_mix_data_length)
            {
                loge("data_tempx length is %d,mix length :%d ,strlen(data_tempx): %d\r\n",length_temp,s_mix_data_length,strlen(data_tempx));
                memset(s_mix_data,0,sizeof(s_mix_data));
                memcpy(s_mix_data,data_tempx,strlen(data_tempx));
                package_data_len = length_temp;
                finish_flag = 0;
                loge("stepxx 1\r\n");
            }
            else
            {
                package_data_len = 0;
                loge("data_tempx length is %d,mix length :%d\r\n",length_temp,s_mix_data_length);
                loge("stepxx 3\r\n");
            }
        }
        else
        {
            package_data_len += strlen(data_tempx);
            if (package_data_len >s_mix_data_length)
            {
                loge("out of nmea buffer\r\n");   
                memset(s_mix_data,0,sizeof(s_mix_data));
                if (s_mix_data_length < strlen(data_tempx))
                {
                    memcpy(s_mix_data,data_tempx,strlen(data_tempx));
                    package_data_len = strlen(data_tempx);
                    finish_flag = 0;
                    nmea_status_init_flag = 1;
                    loge("stepxx 6\r\n");
                }
                else
                {
                    package_data_len = 0;
                    finish_flag = 1;
                    nmea_status_init_flag = 1;
                    loge("data_tempx length1 is %d\r\n",strlen(data_tempx));
                    loge("stepxx 5\r\n");
                }
            }
            else
            {
                strcat(s_mix_data,data_tempx);
                loge("stepxx 4\r\n");
            }               
        }
       // loge("stepxx s_mix_data : %s\r\n",s_mix_data);
        if (strstr(p, "GSV"))
        {
            if (2 == sscanf((const char *)p,"$%*[^,],%d,%d,",&total_gsv,&gsv_num))
            {
                if (total_gsv != gsv_num)
                {
                    return 1;
                }
            }
        }
        //GSA----
       // #ifdef debug_log
        loge("s_mix_data :%s\r\n",p);
      //  #endif
        
        nmea_mask = mesgnmea_to_bitnmea(nmea_set,6);
        loge("nmea_set :%d,nmea_mask :%d\r\n",nmea_set,nmea_mask);
        mixed_ret = mixed_nmea_get_status(nmea_mask,&total_mask,nmea_status_init_flag);
        nmea_status_init_flag = 0;
       // #ifdef debug_log
        loge("s_mix_data nmea_mask:%u,total_mask:%u mixed_ret:%d \r\n",nmea_mask,total_mask,mixed_ret);
      //  #endif
        if (mixed_ret == 0)
        {
            
            //~{4&@mJUFk5D1(ND~},~{;qH!CkJ1<d#,4r0|~}-------------
            //~{TY4N9}BK5t2;PhR*5D1(ND~}
            struct timeval cal_now;
            struct timeval cal_end;
            double now_1,now_2;
            
            gettimeofday(&cal_now, NULL);
            now_1 = cal_now.tv_sec+(cal_now.tv_usec*0.000001);
             
            //~{JUFkO`S&5D0|:s#,Tr4&@m~}
            mixed_nmea_data_process(s_mix_data,strlen(s_mix_data),total_mask);
            loge("s_mix_data total :%s\r\n",s_mix_data);
            memset(s_mix_data,0,sizeof(s_mix_data));
            finish_flag = 1;

            gettimeofday(&cal_end, NULL);
            now_2 = cal_end.tv_sec+(cal_end.tv_usec*0.000001);
            logd("calx_mix ----%lf s:%d,%d;e:%d,%d\r\n",(now_2-now_1),cal_now.tv_sec,cal_now.tv_usec,cal_end.tv_sec,cal_end.tv_usec);
            
            return 1;
        }
        
    }
    return 1;
}

static void get_gnss_data(int sock, short event, void *arg)
{
	char tmp[1024] = { 0 };
    //char close_flag_temp = 0; //~{:M~}$DEBUG~{SP9X~}
	char *p = NULL;
	int size = 0;
	int isok = 1;
	int ret = 0;
	//do
	{
		loge("get data\r\n");
		size = read(sock, tmp, sizeof(tmp) - 1);
		ret = add_to_buffer(s_buffer, (unsigned char *) tmp, size);
		bzero(tmp, sizeof(tmp));
		if (ret < 0)
		{
			logd("buffer overflow\n");
			clean_buffer(s_buffer);
			return;
		}
	} //while (size);
	while (isok)
	{
		int removelen = 0;
		isok = 0;
		bzero(tmp, sizeof(tmp));
		size = get_from_buffer(s_buffer, (unsigned char *) tmp, 1);
		if (size != 1)
		{
			return;
		}
		else if (tmp[0] != '$')
		{
			remove_from_buffer(s_buffer, 1);
			isok = 1;
			continue;
		}
		bzero(tmp, sizeof(tmp));
		size = get_from_buffer(s_buffer, (unsigned char *) tmp, sizeof(tmp) - 1);

		if (strstr(tmp, "\r\n"))
		{
			p = strtok(tmp, "\r\n");
		}

		while (p)
		{
                if (strstr(p, "GGA"))
    			{
    			   isok = nmea_data_process(p,DAT_ID_GGA);
                    Monitor_NMEA.MSG_GGA = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
    			else if (strstr(p, "ZDA"))
    			{
    			    isok = nmea_data_process(p,DAT_ID_ZDA);
                    Monitor_NMEA.MSG_ZDA = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
    			else if (strstr(p, "HDT"))
    			{
    			    isok = nmea_data_process(p,DAT_ID_HDT);
                    Monitor_NMEA.MSG_HDT = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
    			else if (strstr(p, "VTG"))
    			{
    				isok = nmea_data_process(p,DAT_ID_VTG);
                    Monitor_NMEA.MSG_VTG = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "GSV"))
    			{
    				isok = nmea_data_process(p,DAT_ID_GSV);
                    Monitor_NMEA.MSG_GSV = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "RMC"))
    			{
    				isok = nmea_data_process(p,DAT_ID_RMC);
                    Monitor_NMEA.MSG_RMC = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "GLL"))
    			{
    				isok = nmea_data_process(p,DAT_ID_GLL);
                    Monitor_NMEA.MSG_GLL = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "GST"))
    			{
    				isok = nmea_data_process(p,DAT_ID_GST);
                    Monitor_NMEA.MSG_GST = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "GSA"))
    			{
    				isok = nmea_data_process(p,DAT_ID_GSA);
                    Monitor_NMEA.MSG_GSA = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "ROT"))
    			{
    				isok = nmea_data_process(p,DAT_ID_ROT);
                    Monitor_NMEA.MSG_ROT = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "VGK"))
    			{
    				isok = nmea_data_process(p,DAT_ID_VGK);
                    Monitor_NMEA.MSG_VGK = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "VHD"))
    			{
    				isok = nmea_data_process(p,DAT_ID_VHD);
                    Monitor_NMEA.MSG_VHD = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "AVR"))
    			{
                    
    				isok = nmea_data_process(p,DAT_ID_AVR);
                    Monitor_NMEA.MSG_AVR = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "GGK"))
    			{
    				isok = nmea_data_process(p,DAT_ID_GGK);
                    Monitor_NMEA.MSG_GGK = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "BPQ"))
    			{
    				isok = nmea_data_process(p,DAT_ID_BPQ);
                    Monitor_NMEA.MSG_BPQ = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "PJK"))
    			{
    				isok = nmea_data_process(p,DAT_ID_PJK);
                    Monitor_NMEA.MSG_PJK = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "PJT"))
    			{
    				isok = nmea_data_process(p,DAT_ID_PJT);
                    Monitor_NMEA.MSG_PJT = 1;
    				removelen += strlen(p) + 2;
                    loge("isok:%d\r\n",isok);
    				//isok = 1;
    			}
                else if (strstr(p, "BLH"))
                {
                    progress_blh(p, strlen(p));
                    Monitor_NMEA.MSG_BLH = 1;
    				removelen += strlen(p) + 2;
    				isok = 1;
                }
                else if (strstr(p, "TIME"))
                {
                    progress_time(p, strlen(p));
                    Monitor_NMEA.MSG_TIME = 1;
    				removelen += strlen(p) + 2;
    				isok = 1;
                }
                else if (strstr(p, "DOP"))
                {
                    progress_dop(p, strlen(p));
                    Monitor_NMEA.MSG_DOP = 1;
    				removelen += strlen(p) + 2;
    				isok = 1;
                }
                else if (strstr(p, "GRS"))
                {
                    progress_grs(p, strlen(p));
                    Monitor_NMEA.MSG_GRS = 1;
    				removelen += strlen(p) + 2;
    				isok = 1;
                }
                else if (strstr(p, "GNS"))
                {
                    progress_gns(p, strlen(p));
                    Monitor_NMEA.MSG_GNS = 1;
    				removelen += strlen(p) + 2;
    				isok = 1;
                }
    			if (strstr(p + strlen(p) + 2, "\r\n"))
    			{
    				p = strtok(NULL, "\r\n");
    			}
    			else
    			{
    				p = NULL;
    			}
		}
		remove_from_buffer(s_buffer, removelen);
	}

// fprintf(stderr, "%s", tmp_gga);
// fprintf(stderr, "%s", tmp_zda);

	return;
}


void trimble_dev_point_changed_init( )
{
	int tmp_usb_fd = 0;

	clean_buffer(s_buffer);

	tmp_usb_fd = get_trimble_usb_fd();

	uart_close( tmp_usb_fd );

	tmp_usb_fd = uart_open( "/dev/trimble_gps0", 0 );
	if( tmp_usb_fd < 0 )
	{
		fprintf( stderr, "cant open trimble_gps0\n" );
		return;
	}

	uart_option_set( tmp_usb_fd, 115200, 8, 1, 'n' );
	tcflush( tmp_usb_fd, TCIOFLUSH );

	//trimble_set_nmea_data( DAT_FRQ_10HZ, DAT_ID_GGA );
	usleep( 100000 );
	//trimble_set_nmea_data( DAT_FRQ_10HZ, DAT_ID_ZDA );
	usleep( 100000 );
	//trimble_set_nmea_data( DAT_FRQ_10HZ, DAT_ID_HDT );
	usleep( 100000 );

	event_del( &gnss_listen_ev );
	event_assign( &gnss_listen_ev, get_main_base(), tmp_usb_fd, EV_READ | EV_PERSIST, get_gnss_data, NULL );
	event_priority_set( &gnss_listen_ev, 9 );
	event_add( &gnss_listen_ev, NULL );

	logd("++ 982 trimble_gps0 reinit...\n");
}


static void p307_baudrate_control_and_data_request()
{
	int portc_flag = 0;
	int tmp_fd = 0;
	struct sys_info tmp = { { 0 } };

	get_sys_info(&tmp);

	portc_flag = get_p307_portc_flag();

	tmp_fd = get_p307_fd();

	if( 1 == portc_flag )
	{
		//p307_portc_send_command_ctrol_portb("$JBAUD,19200,PORTB\r\n");
		p307_portc_send_command_ctrol_portb("$JBAUD,115200,PORTB\r\n");
	}
	else
	{
		//p307_portb_send_command_ctrol_portb("$JBAUD,19200,PORTB\r\n");
		p307_portb_send_command_ctrol_portb("$JBAUD,115200,PORTB\r\n");
	}
	if (strncmp(tmp.pn, "1172", 4) == 0)
	{
		p307_send_command( tmp_fd, "$JASC,GPGGA,1\r\n" );
		p307_send_command( tmp_fd, "$JASC,GPVTG,0.5\r\n" );
	}
	else if (strncmp(tmp.pn, "1176", 4) == 0)
	{
		p307_send_command( tmp_fd, "$JASC,GPGGA,5\r\n" );
    p307_send_command( tmp_fd, "$JASC,GPVTG,0.5\r\n" );
	}

	logd("++ 307 baudrate control and command send...\n");
}

static void nmea_setting_show(char *pdata,int freq)
{
    switch(freq)
    {
        case DAT_FRQ_START:
            loge("%s,freq:DAT_FRQ_START\n",pdata);
            break;
        case DAT_FRQ_OFF:
            loge("%s,freq:DAT_FRQ_OFF\n",pdata);
            break;
        case DAT_FRQ_10HZ:
            loge("%s,freq:DAT_FRQ_10HZ\n",pdata);   
                break;
        case DAT_FRQ_5HZ:
           loge("%s,freq:DAT_FRQ_5HZ\n",pdata); 
            break;
        case DAT_FRQ_2HZ:
            loge("%s,freq:DAT_FRQ_2HZ\n",pdata); 
            break;
        case DAT_FRQ_1HZ:
            loge("%s,freq:DAT_FRQ_1HZ\n",pdata); 
            break;
        case DAT_FRQ_2S:
            loge("%s,freq:DAT_FRQ_2S\n",pdata); 
            break;
        case DAT_FRQ_5S:
            loge("%s,freq:DAT_FRQ_5S\n",pdata); 
            break;
        case DAT_FRQ_10S:
            loge("%s,freq:DAT_FRQ_10S\n",pdata); 
            break;
        default:
            loge("%s,freq:%d\n",freq); 
            break;
    }
}

static void set_nmea_data(int gnss_type,DAT_FRQ frq, DAT_ID dat_id )
{
    if (gnss_type)
    {
        trimble_set_nmea_data( frq, dat_id );
    }
    else
    {
        hemisphere_set_nmea_data(frq, dat_id);
    }
}
static void nmea_message_supply_again(int gnss_type)
{
    if ( Monitor_NMEA.MSG_GGA == 0)
    {
        if (NMEA_Config_dest.MSG_GGA.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_GGA.msg_freq != DAT_FRQ_OFF)
        {
            if (NMEA_Config_dest.MSG_GGA.msg_freq == DAT_FRQ_10S || NMEA_Config_dest.MSG_GGA.msg_freq == DAT_FRQ_5S || NMEA_Config_dest.MSG_GGA.msg_freq == DAT_FRQ_2S)
            {
                set_nmea_data(gnss_type, DAT_FRQ_1HZ, DAT_ID_GGA );
                nmea_setting_show("--------------- require gga ",DAT_FRQ_1HZ);
            }
            else
            {
                set_nmea_data(gnss_type, NMEA_Config_dest.MSG_GGA.msg_freq, DAT_ID_GGA );
                nmea_setting_show("--------------- require gga ",NMEA_Config_dest.MSG_GGA.msg_freq);
            }
        }
        else
        {
            set_nmea_data(gnss_type, DAT_FRQ_1HZ, DAT_ID_GGA );
            nmea_setting_show("--------------- require gga ",DAT_FRQ_1HZ);
        }
        loge("--------------- require gga \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_GSV.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_GSV.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_GSV == 0)
    {
        set_nmea_data(gnss_type, NMEA_Config_dest.MSG_GSV.msg_freq, DAT_ID_GSV );
        nmea_setting_show("--------------- require gsv ",NMEA_Config_dest.MSG_GSV.msg_freq);
        loge("--------------- require gsv \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_RMC.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_RMC.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_RMC == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_RMC.msg_freq, DAT_ID_RMC );
        nmea_setting_show("--------------- require rmc ",NMEA_Config_dest.MSG_RMC.msg_freq);
        loge("--------------- require rmc \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_GLL.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_GLL.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_GLL == 0)
    {
        set_nmea_data(gnss_type, NMEA_Config_dest.MSG_GLL.msg_freq, DAT_ID_GLL );
        nmea_setting_show("--------------- require gll ",NMEA_Config_dest.MSG_GLL.msg_freq);
        loge("--------------- require gll \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_VTG.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_VTG.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_VTG == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_VTG.msg_freq, DAT_ID_VTG );
        nmea_setting_show("--------------- require vtg ",NMEA_Config_dest.MSG_VTG.msg_freq);
        loge("--------------- require vtg \n");
        usleep( 100000 );
    }
    if ( Monitor_NMEA.MSG_ZDA == 0)
    {
        if (NMEA_Config_dest.MSG_ZDA.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_ZDA.msg_freq != DAT_FRQ_OFF )
        {
            set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_ZDA.msg_freq, DAT_ID_ZDA );
            nmea_setting_show("--------------- require zda ",NMEA_Config_dest.MSG_ZDA.msg_freq);
        }
        else
        {
            set_nmea_data(gnss_type,  DAT_FRQ_1HZ, DAT_ID_ZDA );
            nmea_setting_show("---------------require zda ",DAT_FRQ_1HZ);
        }
        loge("--------------- require zda \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_GST.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_GST.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_GST == 0)
    {
        set_nmea_data(gnss_type, NMEA_Config_dest.MSG_GST.msg_freq, DAT_ID_GST );
        nmea_setting_show("--------------- require gst ",NMEA_Config_dest.MSG_GST.msg_freq);
        loge("---------------require gst \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_GSA.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_GSA.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_GSA == 0)
    {
        set_nmea_data(gnss_type, NMEA_Config_dest.MSG_GSA.msg_freq, DAT_ID_GSA );
        nmea_setting_show("--------------- require gsa ",NMEA_Config_dest.MSG_GSA.msg_freq);
        loge("--------------- require gsa \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_HDT.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_HDT.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_HDT == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_HDT.msg_freq, DAT_ID_HDT );
        nmea_setting_show("--------------- require hdt ",NMEA_Config_dest.MSG_HDT.msg_freq);
        loge("--------------- require hdt \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_ROT.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_ROT.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_ROT == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_ROT.msg_freq, DAT_ID_ROT );
        nmea_setting_show("--------------- require rot ",NMEA_Config_dest.MSG_ROT.msg_freq);
        loge("--------------- require rot \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_VGK.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_VGK.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_VGK == 0)
    {
        set_nmea_data(gnss_type, NMEA_Config_dest.MSG_VGK.msg_freq, DAT_ID_VGK );
        nmea_setting_show("--------------- require vgk ",NMEA_Config_dest.MSG_VGK.msg_freq);
        loge("--------------- require vgk \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_VHD.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_VHD.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_VHD == 0)
    {
        set_nmea_data(gnss_type, NMEA_Config_dest.MSG_VHD.msg_freq, DAT_ID_VHD );
        nmea_setting_show("--------------- require vhd ",NMEA_Config_dest.MSG_VHD.msg_freq);
        loge("--------------- require vhd \n");
        usleep( 100000 );
    }
    if ( Monitor_NMEA.MSG_AVR == 0)
    {
        if (NMEA_Config_dest.MSG_AVR.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_AVR.msg_freq != DAT_FRQ_OFF )
        {
            set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_AVR.msg_freq, DAT_ID_AVR );
            nmea_setting_show("--------------- require avr ",NMEA_Config_dest.MSG_AVR.msg_freq);
        }
        else
        {
            set_nmea_data(gnss_type,  DAT_FRQ_1HZ, DAT_ID_AVR );
            nmea_setting_show("---------------require avr ",DAT_FRQ_1HZ);
        }
        loge("--------------- require avr \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_GGK.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_GGK.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_GGK == 0)
    {
        set_nmea_data(gnss_type, NMEA_Config_dest.MSG_GGK.msg_freq, DAT_ID_GGK );
        nmea_setting_show("--------------- require ggk ",NMEA_Config_dest.MSG_GGK.msg_freq);
        loge("--------------- require ggk \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_BPQ.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_BPQ.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_BPQ == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_BPQ.msg_freq, DAT_ID_BPQ );
        nmea_setting_show("--------------- require bpq ",NMEA_Config_dest.MSG_BPQ.msg_freq);
        loge("--------------- require bpq \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_PJK.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_PJK.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_PJK == 0)
    {
        set_nmea_data(gnss_type, NMEA_Config_dest.MSG_PJK.msg_freq, DAT_ID_PJK );
        nmea_setting_show("--------------- require pjk ",NMEA_Config_dest.MSG_PJK.msg_freq);
        loge("--------------- require pjk \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_PJT.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_PJT.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_PJT == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_PJT.msg_freq, DAT_ID_PJT );
        nmea_setting_show("--------------- require pjt ",NMEA_Config_dest.MSG_PJT.msg_freq);
        loge("--------------- require pjt \n");
        usleep( 100000 );
    }

    //-----------------------------for p307--------------------------
    if (NMEA_Config_dest.MSG_BLH.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_BLH.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_BLH == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_BLH.msg_freq, DAT_ID_BLH );
        nmea_setting_show("--------------- require blh ",NMEA_Config_dest.MSG_BLH.msg_freq);
        loge("---------------require blh \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_TIME.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_TIME.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_TIME == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_TIME.msg_freq, DAT_ID_TIME );
        nmea_setting_show("---------------require time ",NMEA_Config_dest.MSG_TIME.msg_freq);
        loge("--------------- require time \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_DOP.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_DOP.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_DOP == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_DOP.msg_freq, DAT_ID_DOP );
        nmea_setting_show("---------------require dop ",NMEA_Config_dest.MSG_DOP.msg_freq);
        loge("---------------require dop \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_GRS.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_GRS.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_GRS == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_GRS.msg_freq, DAT_ID_GRS );
        nmea_setting_show("--------------- require grs ",NMEA_Config_dest.MSG_GRS.msg_freq);
        loge("--------------- require grs \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_GNS.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_GNS.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_GNS == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_GNS.msg_freq, DAT_ID_GNS );
        nmea_setting_show("--------------- require gns ",NMEA_Config_dest.MSG_GNS.msg_freq);
        loge("--------------- require gns \n");
        usleep( 100000 );
    }
    if (NMEA_Config_dest.MSG_PJT.msg_freq != DAT_FRQ_START && NMEA_Config_dest.MSG_PJT.msg_freq != DAT_FRQ_OFF && Monitor_NMEA.MSG_PJT == 0)
    {
        set_nmea_data(gnss_type,  NMEA_Config_dest.MSG_PJT.msg_freq, DAT_ID_PJT );
        nmea_setting_show("--------------- require pjt ",NMEA_Config_dest.MSG_PJT.msg_freq);
        loge("--------------- require pjt \n");
        usleep( 100000 );
    }

    //~{RQ>-9X1U5DJ}>]N49X1U#,VXPB9X1U~}
    if ((NMEA_Config_dest.MSG_GSV.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_GSV.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_GSV == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_GSV );
        loge("--------------- close gsv \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_RMC.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_RMC.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_RMC == 1)
    {
        set_nmea_data(gnss_type, DAT_FRQ_OFF, DAT_ID_RMC );
        loge("--------------- close rmc \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_GLL.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_GLL.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_GLL == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_GLL );
        loge("--------------- close gll \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_VTG.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_VTG.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_VTG == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_VTG );
        loge("--------------- close vtg \n");
        usleep( 100000 );
    }
    
    if ((NMEA_Config_dest.MSG_GST.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_GST.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_GST == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_GST );
        loge("--------------- close gst \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_GSA.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_GSA.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_GSA == 1)
    {
        set_nmea_data(gnss_type, DAT_FRQ_OFF, DAT_ID_GSA );
        loge("--------------- close gsa \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_HDT.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_HDT.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_HDT == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_HDT );
        loge("---------------close hdt \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_ROT.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_ROT.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_ROT == 1)
    {
        set_nmea_data(gnss_type, DAT_FRQ_OFF, DAT_ID_ROT );
        loge("--------------- close rot \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_VGK.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_VGK.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_VGK == 1)
    {
        set_nmea_data(gnss_type, DAT_FRQ_OFF, DAT_ID_VGK );
        loge("--------------- close vgk \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_VHD.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_VHD.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_VHD == 1)
    {
        set_nmea_data(gnss_type, DAT_FRQ_OFF, DAT_ID_VHD );
        loge("--------------- close vhd \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_GGK.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_GGK.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_GGK == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_GGK );
        loge("--------------- close ggk \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_BPQ.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_BPQ.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_BPQ == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_BPQ );
        loge("--------------- close bpq \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_PJK.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_PJK.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_PJK == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_PJK );
        loge("--------------- close pjk \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_PJT.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_PJT.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_PJT == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_PJT );
        loge("--------------- close pjt \n");
        usleep( 100000 );
    }
    //-----------------------------for p307--------------------------
    if ((NMEA_Config_dest.MSG_BLH.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_BLH.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_BLH == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_BLH );
        loge("--------------- close blh \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_TIME.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_TIME.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_TIME == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_TIME );
        loge("--------------- close time \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_DOP.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_DOP.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_DOP == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_DOP );
        loge("--------------- close dop \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_GRS.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_GRS.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_GRS == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_GRS );
        loge("--------------- close grs \n");
        usleep( 100000 );
    }
    if ((NMEA_Config_dest.MSG_GNS.msg_freq == DAT_FRQ_START || NMEA_Config_dest.MSG_GNS.msg_freq == DAT_FRQ_OFF )&& Monitor_NMEA.MSG_GNS == 1)
    {
        set_nmea_data(gnss_type,  DAT_FRQ_OFF, DAT_ID_GNS );
        loge("--------------- close gns \n");
        usleep( 100000 );
    }
}

static void trimble_nmea_monitor(int sock, short event, void *arg)
{
    struct sys_info tmp = { { 0 } };
    int button;
    get_sys_info(&tmp);
    logd("-------------pthread_no trimble_nmea_monitor:%lu\r\n",pthread_self());
    if (strncmp(tmp.pn + 4, "13", 2) == 0)		//p307
	{
        get_uart_nmea_conf(&NMEA_Config_dest,&button);
        nmea_message_supply_again(0);
        memset((char*)&Monitor_NMEA,0,sizeof((int*)&Monitor_NMEA));
    }
	else		//982 for mc100
	{
	 //~{Hg9{EdVC1mIOSP5D~}NMEA~{EdVCC;SPJU5=O`S&1(ND#,TrVXPBGkGs!#~}
	    get_uart_nmea_conf(&NMEA_Config_dest,&button);
        nmea_message_supply_again(1);
        memset((char*)&Monitor_NMEA,0,sizeof((int*)&Monitor_NMEA));
	}
}

static void gga_monitor_callback(int sock, short event, void *arg)
{
	struct sys_info tmp = { { 0 } };
	int portc_flag = 0;
	static int cal_exit = 0;
	get_sys_info(&tmp);
	portc_flag = get_p307_portc_flag();

	logd("++ s_get_gga_flag[%d] ++ s_dat_frq_wrong_flg[%d] ++ portc_flag[%d] ++ \n", s_get_gga_flag, s_dat_frq_wrong_flg, portc_flag );
	if (s_get_gga_flag == 0)
	{
		if (strncmp(tmp.pn + 4, "13", 2) == 0)		//p307
		{
			p307_baudrate_control_and_data_request();
			loge("++ 307 can not get data ...\n");
			cal_exit++;
		}
		else		//982 for mc100
		{
			// if this happens, maybe because of softlink trimble_gps0 has changed,
			// at the same time, the cpu percent of compaction at least 80-90%! ! !
			// temporary can not find a better way to deal with this, so restart app...
			//trimble_dev_point_changed_init( );
			cal_exit++;
			loge("++ 982 can not get data...\n");
		}
        loge("--------------------- 982 do exit -1 !!!\r\n");
		if(cal_exit >= 5)
		{
			exit(-1);
		}
	}
	else
	{
		if( 1 == s_dat_frq_wrong_flg )		//data output frq random
		{
			if (strncmp(tmp.pn + 4, "13", 2) == 0)		//p307, need reset and reboot system
			{
				if( 1 == portc_flag )
				{
					p307_portc_send_command_ctrol_portb("$JRESET,BOOT\r\n");
					sleep(5);
				}
				else
				{
					p307_portb_send_command_ctrol_portb("$JRESET,BOOT\r\n");
					sleep(5);
				}
				loge("--------------------- 307 do reboot 1...\n");
				system("reboot");
			}
			else		//982 for mc100
			{
				loge("++ 982 get data frq random...\n");
                //loge("--------------------- 982 do exit -2 !!!\r\n");
				//exit(-1);
			}
		}
		cal_exit = 0;
		s_get_gga_flag = 0;
	}
}


//this was used to keep the port baudrate is always 19200
//in case of the sudden change mysterious....
static void port_monitor_callback(int sock, short event, void *arg)
{
	int p307_fd = -1;
	struct sys_info tmp = { { 0 } };

	p307_fd = get_p307_fd();

	get_sys_info(&tmp);

	if (strncmp(tmp.pn + 4, "13", 2) == 0)		//p307
	{
		//p307_send_command( p307_fd, "$JBAUD,19200\r\n");
		p307_send_command( p307_fd, "$JBAUD,115200\r\n");
	}
}


void gnss_process_init()
{
	struct event_base *base = NULL;
	struct sys_info tmp = { { 0 } };
    encrypt_transmission_set_config encrypt_para;
	int gnss_fd = -1;
    //int ret;
	struct timeval tv;
	get_sys_info(&tmp);

	s_buffer = create_buffer(1024 * 100);

	if (strncmp(tmp.pn + 4, "06", 2) == 0)
	{
		gnss_trimble_device_init();
		s_gnss_fd = gnss_fd = get_trimble_usb_fd();
	}
	else
	{
		p307_init();
		gnss_fd = s_gnss_fd = get_p307_fd();
	}

	tcflush(gnss_fd, TCIOFLUSH);
    //~{44=(<SC\O_3L~}
   // ret = pthread_create( &encrypt_thread, NULL, encrypt_process_thread, NULL );
   // if (ret != 0)
  //  {
  //      loge("encrypt_flag error\r\n");
  //  }
    
	get_encrypt_conf( &encrypt_para );
    timer_init();
    timer_update_init();
	base = get_main_base();
    
	event_assign(&gnss_listen_ev, base, gnss_fd, EV_READ | EV_PERSIST, get_gnss_data, NULL);
	//event_priority_set( &gnss_listen_ev, 9 );
	event_add(&gnss_listen_ev, NULL);

    event_assign(&monitor_nmea_ev, base, -1, EV_TIMEOUT | EV_PERSIST, trimble_nmea_monitor, NULL);
	tv.tv_sec = 25;
	tv.tv_usec = 0;
	event_add(&monitor_nmea_ev, &tv);
    
	event_assign(&monitor_ev, base, -1, EV_TIMEOUT | EV_PERSIST, gga_monitor_callback, NULL);
	tv.tv_sec = 60;
	tv.tv_usec = 0;
	event_add(&monitor_ev, &tv);

	event_assign(&port_monitor_ev, base, -1, EV_TIMEOUT | EV_PERSIST, port_monitor_callback, NULL);
	tv.tv_sec = 300;
	tv.tv_usec = 0;
	event_add(&port_monitor_ev, &tv);

	signal(SIGPIPE, SIG_IGN);

}

int gga_utc_time_stick_check( char *gga_data )
{
	int ret =0;
	char tmp[512] = {0};
	double utc_time = 0;
	static int flag_into_fun = 1;
	static int last_utc_time = 0;
	static int wrong_times = 0;

	memcpy( tmp, gga_data, strlen(gga_data) );

	//get utc time, for data frq random...
	if( 1 == sscanf(tmp, "$%*[^,],%lf,%*s",&utc_time) )
	{
		if( 1 == flag_into_fun )
		{
			last_utc_time = (int)utc_time;
			flag_into_fun = 0;
		}
		else
		{
			ret = (int)utc_time - last_utc_time;
			if( ret > 30 )
			{
				if( (41==ret) || (4041==ret) )	//seconds jump to minute, minutes jump to hours
				{
					last_utc_time = (int)utc_time;
				}
				else
				{
					wrong_times++;
					loge("++ data put frq wrong times [%d] ...\n", wrong_times);
					last_utc_time = (int)utc_time;
				}
				if( 100 == wrong_times )
				{
					loge("++ data frq wrong, need reset oem-bord and reboot receiver...\n");
					s_dat_frq_wrong_flg = 1;
				}
			}
			else		//normal
			{
				wrong_times = 0;
				last_utc_time = (int)utc_time;
			}
		}
	}
	return 0;
}
/*
~{JdHk~}:~{4}W*7"5DJ}Wi~}*temp,~{Q!VP5D1(ND@`PM~}
*/
void nmea_data_output_authority(char *temp,int length,int message_type)
{
    if (GNSS_SET_CONFIG.button_main == 0)
    {
        if (GNSS_SET_CONFIG.output_type & UART_OUT)
        {
            add_to_uart_thread(temp, length);
        }
        if (GNSS_SET_CONFIG.output_type & CAN_OUT)
        {
            add_to_can_thread(temp, length);
        }
    }
}
/* ~{0Q>-N36H~}(geodetic)~{W*;;3I?U<dV1=GWx1jO5~}(ecef position) ---------------------
* ~{2NJ}~}   : double *pos      I   ~{>-N36HN;VC~} {lat,lon,h} (~{;!6H~},m)
*          double *r        O   ~{?U<dV1=GWx1jO5~} {x,y,z} (m)
* ~{75;XV5~} : none
* ~{K5Cw~}  : WGS84, ellipsoidal height
*-----------------------------------------------------------------------------*/
void pos2ecef(const double *pos, double *r)
{
	double sinp=sin(pos[0]),cosp=cos(pos[0]),sinl=sin(pos[1]),cosl=cos(pos[1]);
	double e2=FE_WGS84*(2.0-FE_WGS84),v=RE_WGS84/sqrt(1.0-e2*sinp*sinp);
    
	r[0]=(v+pos[2])*cosp*cosl;
	r[1]=(v+pos[2])*cosp*sinl;
	r[2]=(v*(1.0-e2)+pos[2])*sinp;
}

void xyz2enu(const double *pos,double *E)
{
    double sinp=sin(pos[0]),cosp=cos(pos[0]),sinl=sin(pos[1]),cosl=cos(pos[1]);
    E[0] = -sinl;           E[3] = cosl;            E[6] = 0.0;
    E[1] = -sinp*cosl;      E[4] = -sinp*sinl;      E[7] = cosp;
    E[2] = cosp*cosl;       E[5] = cosp*sinl;       E[8] = sinp;
}

void matmul(const char *tr,int n,int k,int m,double alpha,const double *A,const double *B,double beta,double *C)
{
    double d;
    int i,j,x,f = tr[0] == 'N'?(tr[1] == 'N'?1:2):(tr[1] == 'N'?3:4);
    for (i = 0;i < n;i++)
    {
        for (j = 0;j < k;j++)
        {
            d = 0.0;
            switch(f)
            {
                case 1: for (x=0;x<m;x++) d+=A[i+x*n]*B[x+j*m];break;
                case 2: for (x=0;x<m;x++) d+=A[i+x*n]*B[j+x*k];break;
                case 3: for (x=0;x<m;x++) d+=A[x+i*m]*B[x+j*m];break;
                case 4: for (x=0;x<m;x++) d+=A[x+i*n]*B[j+x*k];break;
                default:break;
            }
            if (EQUAL(beta,0.0))
            {
                C[i+j*n] = alpha * d;
            }
            else
            {
                C[i+j*n] = alpha * d + beta*C[i+j*n];
            }
        }
    }
}

void ecef2enu(const double *pos,const double *r,double *e)
{
    double E[9];
    xyz2enu(pos,E);
    matmul("NN",3,1,3,1.0,E,r,0.0,e);
}

/*
function:~{<FKcG0:sA=J1?L#,M,R;5c5D>`@k2n~},~{<FKcH}N,?U<d~}
input:BLH(~{;!6H#,CW~})
return:distance(m)
*/
double cal_distance_of_two_point(double *BLH_postion0,double *BLH_postion1)
{
    double XYZ_postion0[3] = {0};
    double XYZ_postion1[3] = {0};
    double dxyz_postion[3] = {0};
    //double enu[3] = {0};
    double distance;
    pos2ecef(BLH_postion0, XYZ_postion0);
    pos2ecef(BLH_postion1, XYZ_postion1);
    loge("-------------cal ----XYZ0 : %lf, %lf, %lf\r\n",XYZ_postion0[0],XYZ_postion0[1],XYZ_postion0[2]);
    loge("-------------cal ----XYZ1 : %lf, %lf, %lf\r\n",XYZ_postion1[0],XYZ_postion1[1],XYZ_postion1[2]);
    dxyz_postion[0] = XYZ_postion1[0] - XYZ_postion0[0];
    dxyz_postion[1] = XYZ_postion1[1] - XYZ_postion0[1];
    dxyz_postion[2] = XYZ_postion1[2] - XYZ_postion0[2];
    distance = sqrt(dxyz_postion[0]*dxyz_postion[0] + dxyz_postion[1]*dxyz_postion[1] + dxyz_postion[2]*dxyz_postion[2]);
    return distance;
}
#define web_service_address "http://192.168.200.2:80/Handler.ashx"
HC_INT32 curl_http_send_data(HC_IN HC_INT8 *url, HC_IN HC_INT8 *sourse_data)
{
    CURL *curl;
    CURLcode res;

    /* In windows, this will init the winsock stuff */ 
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */ 
    curl = curl_easy_init();
    if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */ 
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);//设置不验证证书 
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);//~{G?VFP-RiN*~}1.1
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4 );//~{G?VFJ9SC~}IPV4~{P-Ri=bNvSrC{~}
    /* Now specify the POST data */ 
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sourse_data);
    
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
    {
        loge("curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
    }
    loge("----------------------------curl senddata err:%s\r\n",curl_easy_strerror(res));
    /* always cleanup */ 
    curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0;
}
static char data_temp[1000] = {0};
static HC_INT8 writeToString(void *ptr, HC_INT8 size, HC_INT8 nmemb, void *userdata)
{
    HC_INT8 written;
    memset(data_temp,0,1000);
    strncpy(data_temp,(char*)ptr,size*nmemb);
    written = size*nmemb;
    return written;
}
HC_INT32 curl_http_receive_data(HC_IN HC_INT8 *url)
{
    CURL *curl;
    CURLcode res;
    char ack_data[1000] = {0};
    /* In windows, this will init the winsock stuff */ 
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */ 
    curl = curl_easy_init();
    if(curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);//设置不验证证书 
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, ack_data);
        res = curl_easy_perform(curl);
        /* Check for errors */ 
        if(res != CURLE_OK)
        {
            loge("curl_easy_perform() failed1: %s\n",curl_easy_strerror(res));
        }
        else
        {
            logd("curl0:%s\r\n",data_temp);
            logd("curl1:%s\r\n",ack_data);
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0;
}

long writer(void *data, int size, int nmemb, char *content)
{
    long sizes = size * nmemb;
	loge("writer_view_ing: %d,%s\r\n",sizes,data);
	if ((strlen(content) + sizes) < 1000)
	{
		strcat(content,(char*)data);
	}
    else
    {
		loge("data over buffer\r\n");
	}
    return sizes;
}
HC_INT32 curl_http_send_data1(HC_IN HC_INT8 *url, HC_IN HC_INT8 *sourse_data)
{
    CURL *curl;
   // CURLcode res;
    char content[1000];
    CURLcode code;
    char error[300];
    /* In windows, this will init the winsock stuff */ 
    curl_global_init(CURL_GLOBAL_DEFAULT);

    /* get a curl handle */ 
    curl = curl_easy_init();
    if(curl) {
    code = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);//CURLOPT_ERRORBUFFER属性，让libcurl缓存许多可读的错误信息。
    if (code != CURLE_OK)
    {
        loge( "Failed to set error buffer [%d]\n", code );
        goto err_process;
    }
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);//设置不验证证书 
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);//~{G?VFP-RiN*~}1.1
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4 );//~{G?VFJ9SC~}IPV4~{P-Ri=bNvSrC{~}
    code = curl_easy_setopt(curl, CURLOPT_URL, url);//URL地址
    if (code != CURLE_OK)
    {
        loge("Failed to set URL [%s]\n", error);
        goto err_process;
    }
    code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);//设置这个选项为一个非零值(象“Location:“)的头，服务器会把它当做HTTP头的一部分发送(注意这是递归的，PHP将发送形如“Location: “的头)。
    if (code != CURLE_OK)
    {
        loge( "Failed to set redirect option [%s]\n", error );
        goto err_process;
    }
    code = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sourse_data);
    if (code != CURLE_OK)
    {
        loge( "Failed to set send data [%s]\n", error);
        goto err_process;
    }
	loge("writer_view_begin--------------------------------------\r\n");
    code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
    if (code != CURLE_OK)
    {
        loge( "Failed to set writer [%s]\n", error);
        goto err_process;
    }
    code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, content);
	loge("writer_view_end----------------------------------------\r\n");
    if (code != CURLE_OK)
    {
        loge( "Failed to set write data [%s]\n", error );
        goto err_process;
    }
    code = curl_easy_perform(curl);
    if (code != CURLE_OK)
    {
        loge( "Failed to get '%s' [%s]\n", url, error);
        goto err_process;
    }
    long retcode = 0;
    code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &retcode);
    if ( (code == CURLE_OK) && retcode == 200 )
    {
        double length = 0;
        code = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD , &length);
        loge("%d\r\n",retcode);

        loge("data :%s\r\n",content);
        if (strstr(content,"/tempuri.org"))
        {
            char *p = strstr(content,"/tempuri.org");
            int response_res = 4;
            if (p != NULL)
            {
                sscanf(p,"/tempuri.org/\">%d<",&response_res );
                loge("response_res :%d\r\n",response_res);
            }
        }
    }
    else
    {
        goto err_process;
    }
 }   
    curl_easy_cleanup(curl);
    return 0;
err_process:
{
    curl_easy_cleanup(curl);
    return 1;
}
    
}
//~{Q9J5J}>]Wi0|~}--------------------------------------
double frequency = 0;
double compacrate = 0;
int amplitude = 0;
int gps_drct = 0;

int compaction_data_package(char *response_string1,encrypt_transmission_set_config *tmp)
{
    //~{C?Lu<GB<5DJ1<d4A1jJ6#,>+H75=:ACk~}
    int year, mouth, day;
    int hour, min, sec;
    xdata_message sourse_message = {{0}};
    char sn[ 20 ] = { 0 };
    int ret = 0;
    time_t tt = 0;
    static int static_speed_mark = 0;
    ret = sscanf( s_zda_t, "$%*[^,],%2d%2d%2d.%*d,%d,%d,%d,%*d,%*s", &hour, &min, &sec, &day, &mouth, &year );
    if ( ret != 6 )
    {
    	logd( "zda is not vaild %s\n", s_zda_t );
    	return 0;
    }
    
    // logd("-----------------------------time 1:%4d%02d%02d %2d:%2d:%2d\r\n",year, mouth, day, hour ,min, sec);
    tt = utcdatetotime_t( year, mouth, day, hour, min, sec );
    // logd("-----------------------------time 2:%4d%02d%02d %2d:%2d:%2d\r\n",year, mouth, day, hour ,min, sec);
    time_ttoctcdate( tt, &year, &mouth, &day, &hour, &min, &sec );
    // logd("-----------------------------time 3:%4d%02d%02d %2d:%2d:%2d\r\n",year, mouth, day, hour ,min, sec);
    get_sn_number( sn );

    //loge( "sn : %s\n", sn );
    get_acm_data(&frequency,&compacrate,&amplitude,&gps_drct);
    
    sprintf( sourse_message.seqid,      "%llu", 1000 * ( unsigned long long ) tt );
    //~{Ih18N(R;1`:E~}
    sprintf( sourse_message.unit_id,    "hc_%s", sn );

    //~{;zP5J)9$9$7(@`PM~}  ~{3eDk~}:01 ~{UqDk~}:02 ~{G?:;~}:03 ~{KiJ/W.;z~}:04 cfg~{W.;z~}:05
    if ((tmp->gf >5)|| (tmp->gf<1))
    {
        tmp->gf = 1;
    }

    //约束条件:对于定位、授时失败的信息，即经度、纬度为0，或者GPS时间无效的施工记录，不保存。
    //约束条件:对于速度大于36km/h压实施工信息，不保存。
    //约束条件:对于速度小于0.36km/h的压实施工信息，视为静止数据，静止数据的采集信息为2分钟/条。
    if ((s_longitude == 0)||(s_latitude == 0)|| (year < 2017)|| (vtg_speed > 36))
    {
        return 0;
    }
    loge("vtg_speed :%lf\r\n",vtg_speed);
    if (vtg_speed <= 0.36)//这个任务，5s调用一次。
    {
        static_speed_mark++;
        logd("static_speed_mark :%d\r\n",static_speed_mark);
        if (static_speed_mark >=23)
        {
            static_speed_mark = 0;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        static_speed_mark = 0;
    }
    
    sprintf(sourse_message.gf ,"%02d", tmp->gf);
    //GPS~{J1<d~}
    sprintf( sourse_message.gps_time,"%04d-%02d-%02d %02d:%02d:%02d",year, mouth, day, hour, min, sec);
    //~{>-6HV5~}
    double s_longitude_temp,s_longitude_temp1,s_longitude_temp2;
    s_longitude_temp1 = (int)s_longitude / 100;    //~{U{J}~}
    s_longitude_temp2 = (s_longitude - s_longitude_temp1*100) / 60;//~{P!J}~}
    s_longitude_temp = s_longitude_temp1 + s_longitude_temp2;
    sprintf( sourse_message.lon,        "%.08lf", s_longitude_temp );
    loge("lon:%lf,lon_t:%lf\r\n",s_longitude,s_longitude_temp);
    //~{>-6HKyTZ0kGr#(~}E-~{6+0kGr~} W-~{Nw0kGr#)~}
    sprintf( sourse_message.ew,         "%c", s_west_east_longitude );
    //~{N36HV5~}
    double s_latitude_temp,s_latitude_temp1,s_latitude_temp2;
    s_latitude_temp1 = (int)s_latitude / 100;    //~{U{J}~}
    s_latitude_temp2 = (s_latitude - s_latitude_temp1*100) / 60;//~{P!J}~}
    s_latitude_temp = s_latitude_temp1 + s_latitude_temp2;
    sprintf( sourse_message.lat,        "%.08lf", s_latitude_temp );
    loge("lat:%lf,lat_t:%lf\r\n",s_latitude,s_latitude_temp);
    //~{N36HKyTZ0kGr#(~}S-~{DO0kGr~} N-~{110kGr#)~}
    sprintf( sourse_message.ns,         "%c", s_south_north_latitude );
    //~{8_3LV5~}
    sprintf( sourse_message.height,     "%lf", s_high );
    //~{PP=x7=Or#(~}0-~{G0=x~} 1-~{:sMK#)~}
    sprintf( sourse_message.gps_drct,   "%d", gps_drct );//~{2;Ge3~4SDD@oH!~}
    //Gps~{W4L,~}
    sprintf( sourse_message.gps_state,  "%d", s_fix_status );
    //~{35A>Uq6/F5BJ~}
    sprintf( sourse_message.frequency,  "%lf", frequency );
    //~{Uq7y~}
    sprintf( sourse_message.amplitude,  "%d", amplitude );
    //~{C\J56HV5#,N*~}0-100~{V5~}
    sprintf( sourse_message.compacrate, "%lf", compacrate );
    //~{35A>KY6H#(~}km/h~{#)~}
    sprintf( sourse_message.speed,      "%lf", vtg_speed );

    //~{SC~}json~{8qJ=Iz3I~}
    char *response_string = NULL;
    //char response_string1[600]= {0};
    JSON_Value *val = NULL;
    JSON_Object *obj = NULL;
    val = json_value_init_object();
    obj = json_value_get_object( val );
    if ( !obj )
    {
        json_value_free( val );
        loge( "json_value_get_object error\n" );
        return 0;
    }
	json_object_set_string( obj, "seqid",       sourse_message.seqid );
    json_object_set_string( obj, "unit_id",     sourse_message.unit_id );
    
    json_object_set_string( obj, "gps_time",    sourse_message.gps_time );
    json_object_set_string( obj, "lon",         sourse_message.lon );
    json_object_set_string( obj, "ew",          sourse_message.ew );
    json_object_set_string( obj, "lat",         sourse_message.lat );
    json_object_set_string( obj, "ns",          sourse_message.ns );
    json_object_set_string( obj, "height",      sourse_message.height );
    json_object_set_string( obj, "gps_drct",    sourse_message.gps_drct );
    json_object_set_string( obj, "gps_state",   sourse_message.gps_state );
    json_object_set_string( obj, "frequency",   sourse_message.frequency );
    json_object_set_string( obj, "amplitude",   sourse_message.amplitude );
    json_object_set_string( obj, "compacrate",  sourse_message.compacrate );
    json_object_set_string( obj, "speed",       sourse_message.speed );
    json_object_set_string( obj, "remark",      "CHC" );
    json_object_set_string( obj, "gf",          sourse_message.gf);
    response_string = json_serialize_to_string( val );
    json_value_free( val );

    sprintf(response_string1,"%s",response_string);
	if (strlen(response_string1) > 2000)
	{
		return 0;
	}
    //for test-------------------------------------------
    #if 0
    JSON_Value *val1 = NULL;
    JSON_Object *obj1 = NULL;
    //char *tresponse_string = NULL;
    
    val1 = json_value_init_object();
    obj1 = json_value_get_object( val1 );
    if ( !obj1 )
    {
        json_value_free( val1 );
        loge( "json_value_get_object error\n" );
        return 0;
    }
    #endif
   // json_object_set_string( obj1, "GetHCRollingData",       response_string1 );
   //json_object_set_string( obj1, "upload",       response_string1 );
    //tresponse_string = json_serialize_to_string( val1 );
    //sprintf(tresponse_string1,"%s",tresponse_string);
    logd("encrypt begin\r\n");

    
	#if 0
	FILE *fp1 = NULL;
	fp1 = fopen("/mnt/mingwen.txt","a+");
	if (fp1)
	{
	
		fwrite(response_string1,sizeof(char),strlen(response_string1),fp1);
        fwrite("\r\n\r\n\r\n",sizeof(char),6,fp1);
		fclose(fp1);
	}
    else
    {
        return 0;
    }
	#endif
    return 1;
}



//~{KiJ/J}>]Wi0|~}--------------------------------------
int gravel_data_package(char *response_string1,encrypt_transmission_set_config *tmp)
{
   
    //~{C?Lu<GB<5DJ1<d4A1jJ6#,>+H75=:ACk~}
    int year, mouth, day;
    int hour, min, sec;
    gravel_message sourse_message = {{0}};
    char sn[ 20 ] = { 0 };
    int ret = 0;
    time_t tt = 0;
	int err_info = 0;
	char pileresult_data_temp[500] = {0};

    ret = sscanf( s_zda_t, "$%*[^,],%2d%2d%2d.%*d,%d,%d,%d,%*d,%*s", &hour, &min, &sec, &day, &mouth, &year );
    if ( ret != 6 )
    {
    	logd( "zda is not vaild %s\n", s_zda_t );
    	goto err;
    }
    
    // logd("-----------------------------time 1:%4d%02d%02d %2d:%2d:%2d\r\n",year, mouth, day, hour ,min, sec);
    tt = utcdatetotime_t( year, mouth, day, hour, min, sec );
    // logd("-----------------------------time 2:%4d%02d%02d %2d:%2d:%2d\r\n",year, mouth, day, hour ,min, sec);
    time_ttoctcdate( tt, &year, &mouth, &day, &hour, &min, &sec );
    // logd("-----------------------------time 3:%4d%02d%02d %2d:%2d:%2d\r\n",year, mouth, day, hour ,min, sec);
    get_sn_number( sn );

    //loge( "sn : %s\n", sn );
    get_acm_data(&frequency,&compacrate,&amplitude,&gps_drct);
    
    sprintf( sourse_message.seqid,      "%llu", 1000 * ( unsigned long long ) tt );
	if (str_add_end(sourse_message.seqid,strlen(sourse_message.seqid),sizeof(sourse_message.seqid)) == HC_ERR)
	{
		loge("1 : %d , %d\r\n",strlen(sourse_message.seqid),sizeof(sourse_message.seqid));
		err_info = 1;
		goto err1;
	}
	
    //~{Ih18N(R;1`:E~}
    sprintf( sourse_message.unit_id,    "hc_%s", sn );
	if (str_add_end(sourse_message.unit_id,strlen(sourse_message.unit_id),sizeof(sourse_message.unit_id)) == HC_ERR)
	{
		err_info = 2;
		goto err1;
	}
    
    //~{;zP5J)9$9$7(@`PM~}  ~{3eDk~}:01 ~{UqDk~}:02 ~{G?:;~}:03 ~{KiJ/W.;z~}:04 cfg~{W.;z~}:05
    loge("gf:%d\r\n",tmp->gf);
    if ((tmp->gf >5)|| (tmp->gf<1))
    {
        tmp->gf = 1;
    }
    sprintf(sourse_message.gf ,"%02d", tmp->gf);
	if (str_add_end(sourse_message.gf,strlen(sourse_message.gf),sizeof(sourse_message.gf)) == HC_ERR)
	{
		err_info = 3;
		goto err1;
	}
    if (HC_ERR == get_senior_gravel_data((senior_gravel *)&sourse_message.gravel_info,pileresult_data_temp))
    {
		goto err1;
	}

    //如果四个时间均为-1，就不传
    if ((strncmp(sourse_message.gravel_info.down_begin_time,"-1",2) == 0)&&(strncmp(sourse_message.gravel_info.down_end_time,"-1",2) == 0)&&(strncmp(sourse_message.gravel_info.up_begin_time,"-1",2) == 0)&&(strncmp(sourse_message.gravel_info.up_end_time,"-1",2) == 0))
    {
        loge("view four_time are -1\r\n");
        goto err;
    }
    long down_begin_time_t = 0,down_end_time_t = 0,up_begin_time_t = 0,up_end_time_t = 0;
    down_begin_time_t = atoi(sourse_message.gravel_info.down_begin_time);
    down_end_time_t   = atoi(sourse_message.gravel_info.down_end_time);
    up_begin_time_t   = atoi(sourse_message.gravel_info.up_begin_time);
    up_end_time_t     = atoi(sourse_message.gravel_info.up_end_time);
    if ((down_begin_time_t<1000) && (down_begin_time_t != -1))
    {
        loge("four_time down_begin_time_t :%ld\r\n",down_begin_time_t);
        reflash_flag_set(0);
        goto err;
    }
    if ((down_end_time_t<1000) && (down_end_time_t != -1))
    {
        loge("four_time down_end_time_t :%ld\r\n",down_end_time_t);
        reflash_flag_set(0);
        goto err;
    }
    if ((up_begin_time_t<1000) && (up_begin_time_t != -1))
    {
        loge("four_time up_begin_time_t :%ld\r\n",up_begin_time_t);
        reflash_flag_set(0);
        goto err;
    }
    if ((up_end_time_t<1000) && (up_end_time_t != -1))
    {
        loge("four_time up_end_time_t :%ld\r\n",up_end_time_t);
        reflash_flag_set(0);
        goto err;
    }
    //~{>-6HV5~}
    double s_longitude_temp,s_longitude_temp1,s_longitude_temp2;
    s_longitude_temp1 = (int)s_longitude / 100;    //~{U{J}~}
    s_longitude_temp2 = (s_longitude - s_longitude_temp1*100) / 60;//~{P!J}~}
    s_longitude_temp = s_longitude_temp1 + s_longitude_temp2;
    //~{>-6HKyTZ0kGr#(~}E-~{6+0kGr~} W-~{Nw0kGr#)~}
    if (s_west_east_longitude == 'W')
    {
        s_longitude_temp *= (-1.0);
    }
    sprintf( sourse_message.lon,        "%.08lf", s_longitude_temp );
	if (str_add_end(sourse_message.lon,strlen(sourse_message.lon),sizeof(sourse_message.lon)) == HC_ERR)
	{
		err_info = 4;
		goto err1;
	}
    loge("lon:%lf,lon_t:%lf\r\n",s_longitude,s_longitude_temp);
    
    //~{N36HV5~}
    double s_latitude_temp,s_latitude_temp1,s_latitude_temp2;
    s_latitude_temp1 = (int)s_latitude / 100;    //~{U{J}~}
    s_latitude_temp2 = (s_latitude - s_latitude_temp1*100) / 60;//~{P!J}~}
    s_latitude_temp = s_latitude_temp1 + s_latitude_temp2;
    //~{N36HKyTZ0kGr#(~}S-~{DO0kGr~} N-~{110kGr#)~}
    if (s_south_north_latitude == 'S')
    {
        s_latitude_temp *= (-1.0);
    }
    sprintf( sourse_message.lat,        "%.08lf", s_latitude_temp );
	if (str_add_end(sourse_message.lat,strlen(sourse_message.lat),sizeof(sourse_message.lat)) == HC_ERR)
	{
		err_info = 5;
		goto err1;
	}
    loge("lat:%lf,lat_t:%lf\r\n",s_latitude,s_latitude_temp);
    

    char *response_string = NULL;
    
    JSON_Value *val = NULL;
    JSON_Object *obj = NULL;
    val = json_value_init_object();
    obj = json_value_get_object( val );
    if ( !obj )
    {
        json_value_free( val );
        loge( "json_value_get_object error\n" );
        goto err;
    }
	
	json_object_set_string( obj, "seqid",       sourse_message.gravel_info.record_time);
	
    json_object_set_string( obj, "unit_id",     sourse_message.unit_id );
    if (strncmp(sourse_message.gravel_info.longitude,"-1",2) == 0)
    {
        json_object_set_string( obj, "lon",         sourse_message.lon );
    }
    else
    {
        json_object_set_string( obj, "lon",         sourse_message.gravel_info.longitude);
    }

    if (strncmp(sourse_message.gravel_info.latitude,"-1",2) == 0)
    {
        json_object_set_string( obj, "lat",         sourse_message.lat );
    }
    else
    {
        json_object_set_string( obj, "lat",         sourse_message.gravel_info.latitude);
    }
    



    json_object_set_string( obj, "down_begin_time",          sourse_message.gravel_info.down_begin_time);
    loge("down_begin_time\r\n");

    json_object_set_string( obj, "down_end_time",      sourse_message.gravel_info.down_end_time);
    loge("down_end_time\r\n");
    //~{0NW.?*J<J1<d~}(~{J1<d4A#,>+H75=:ACk~})
    json_object_set_string( obj, "up_begin_time",    sourse_message.gravel_info.up_begin_time);

    //~{0NW.=aJxJ1<d~}(~{J1<d4A#,>+H75=:ACk~})
    json_object_set_string( obj, "up_end_time",    sourse_message.gravel_info.up_end_time);
    
    //~{3AW.5gAw~}(~{5%N;~}A,~{>+H75=P!J}5c~}2~{N;SPP'J}WV~})
    json_object_set_string( obj, "down_current",    sourse_message.gravel_info.down_current);

    //~{0NW.5gAw~}(~{5%N;~}A,~{>+H75=P!J}5c~}2~{N;SPP'J}WV~})
    json_object_set_string( obj, "up_current",    sourse_message.gravel_info.up_current);
    
    //~{742e5gAw#(5%N;~} A~{#,~} ~{>+H75=P!J}5c~}2~{N;SPP'J}WV#)~}
    json_object_set_string( obj, "repeatinto_current",    sourse_message.gravel_info.repeatinto_current);
    
    //~{3VA&2c5gAw#(5%N;~} A~{#,~} ~{>+H75=P!J}5c~}2~{N;SPP'J}WV#)~}
    json_object_set_string( obj, "uptomax_current",    sourse_message.gravel_info.uptomax_current);
    
    //~{3AW.In6H#(5%N;~}m,~{>+H75=P!J}5c~}2~{N;SPP'J}WV#)~}
    json_object_set_string( obj, "down_depth",    sourse_message.gravel_info.down_depth);
    
    //~{J5J1In6H#(5%N;~}m,~{>+H75=P!J}5c~}2~{N;SPP'J}WV#)~}
    json_object_set_string( obj, "realtime_depth",    sourse_message.gravel_info.realtime_depth);
    
    //~{742eIn6H#(5%N;~}m,~{>+H75=P!J}5c~}2~{N;SPP'J}WV#)~}
    json_object_set_string( obj, "repeatinto_depth",    sourse_message.gravel_info.repeatinto_depth);
    
    //~{742e4NJ}#(5%N;4N#)~}
    json_object_set_string( obj, "repeatinto_times",    sourse_message.gravel_info.repeatinto_times);
    
    //~{742eJ13$#(5%N;~}s~{#)~}
    json_object_set_string( obj, "repeatinto_duration",    sourse_message.gravel_info.repeatinto_duration);
    
    //~{9`HkA?#(5%N;~}kg, ~{>+H75=P!J}5c~}2~{N;SPP'J}WV#)~}
    json_object_set_string( obj, "weight",    sourse_message.gravel_info.weight);
    json_object_set_string( obj, "remark",      "CHC" );
    json_object_set_string( obj, "gf",          sourse_message.gf);
    response_string = json_serialize_to_string( val );
    json_value_free( val );
	
    sprintf(response_string1,"%s",response_string);
	if (strlen(response_string1)>2000)
	{
		goto err;
	}

    return 1;

    err:
    {
        return 0;
    }

	err1:
	{
		return 0;
	}
}

/*
* Function:Encrypted plaintext 
* Paramer:   tmp:encrypt para ;response_string1:plaintext ;
* Return:  1:normal fail 0:sucess 
* History: author:zh   date:2017.03.28
*/
unsigned int data_encrypt_and_update(encrypt_transmission_set_config *tmp,char *response_string1)
{
    unsigned char data_dest[3000] = {0};
    char tresponse_string1[4000]= {0};
    loge("process1\r\n");
    //~{JG7q<SC\~}
    if (tmp->encryption_status)
    {
        struct timeval cal_now;
        struct timeval cal_end;
        struct timeval cal_end1;
        double now_1 = 0,now_2 = 0,now_3 = 0;
       

        //~{;qH!~}KEY~{:M~}IV ~{5D:/J}~}
        unsigned char xkey[32] = {"12345678abcdefgh12345678"};
        unsigned char xkey_64BASE[100] = {0};
        unsigned char iv[8] = {"hijklmno"};
        unsigned char iv_64BASE[50] = {0};
        unsigned char separator[20] = {"<--->"};
        //KEY~{:M~}IV~{F40|~}
        unsigned char publickey_minwen[100] = {0};
        unsigned char publickey_base64_miwen[1000] = {0};
        //base64~{<SC\~}KEY~{:M~}IV
        base64_encode( (const unsigned char *)xkey, (char *)xkey_64BASE, 24 );
        base64_encode( (const unsigned char *)iv, (char *)iv_64BASE, 8 );
        memcpy(publickey_minwen,(char *)xkey_64BASE,strlen((char *)xkey_64BASE));
        memcpy(publickey_minwen+strlen((char *)xkey_64BASE),(char *)separator,strlen((char *)separator));
        memcpy(publickey_minwen+strlen((char *)xkey_64BASE)+strlen((char *)separator),(char *)iv_64BASE,strlen((char *)iv_64BASE));
        //RSA~{<SC\~}
        gettimeofday(&cal_now, NULL);
        now_1 = cal_now.tv_sec+(cal_now.tv_usec*0.000001);
        
        ENCRYPT_RSA(publickey_minwen,publickey_base64_miwen);
        gettimeofday(&cal_end, NULL);
        now_2 = cal_end.tv_sec+(cal_end.tv_usec*0.000001);
        logd("calx-rsa----%lf s:%d,%d;e:%d,%d\r\n",(now_2-now_1),cal_now.tv_sec,cal_now.tv_usec,cal_end.tv_sec,cal_end.tv_usec);
		
       	
		int write_length = 0;
		limit_file_length("/mnt/send_to_back.txt",limited_file_len);
		FILE *fp3 = NULL;
    	fp3 = fopen("/mnt/send_to_back.txt","a+");//需要被替换
    	if (fp3)
    	{
    		
    	    loge("encrypted_data length : %d\r\n",strlen(response_string1));
			write_length = fwrite("sourse_data:\r\n",sizeof(char),strlen("sourse_data:\r\n"),fp3);
			loge("write_length0 :%d\r\n",write_length);
    		write_length = fwrite(response_string1,sizeof(char),strlen(response_string1),fp3);
			loge("write_length1 :%d\r\n",write_length);
            fwrite("\r\n\r\n",sizeof(char),4,fp3);
			fclose(fp3);
    	}
		else
		{
			loge("fopen file %s err\r\n","/mnt/send_to_back.txt");
		}
        //DES~{<SC\~}
        if (1 == ENCRYPT_3DES_CBC((unsigned char*)response_string1,data_dest))
       	{
			return send_err;
		}
        gettimeofday(&cal_end1, NULL);
        now_3 = cal_end1.tv_sec+(cal_end1.tv_usec*0.000001);
        logd("calx-3des----%lf s:%d,%d;e:%d,%d\r\n",(now_3-now_2),cal_end.tv_sec,cal_end.tv_usec,cal_end1.tv_sec,cal_end1.tv_usec);

        //~{Wi:O3I~} : 3DES~{<SC\5D~}json~{J}>]TZG0~},RSA~{<SC\5D~}KEY~{:M~}iv~{TZ:s~},~{VP<dM(9}~}separator~{8t?*~}
        char encrypted_data[3000] = {0};
        char encrypted_data_base64[4000] = {0};
		int len_compare = strlen((char *)data_dest);
		if (len_compare > sizeof(encrypted_data))
		{
			return send_err;
		}
        memcpy(encrypted_data,(char *)data_dest,strlen((char *)data_dest));
		len_compare += strlen((char *)separator);
		if (len_compare > sizeof(encrypted_data))
		{
			return send_err;
		}
        memcpy(encrypted_data + strlen((char *)data_dest),(char *)separator,strlen((char *)separator));
		len_compare += strlen((char *)publickey_base64_miwen);
		if (len_compare > sizeof(encrypted_data))
		{
			return send_err;
		}
        memcpy(encrypted_data + strlen((char *)data_dest) + strlen((char *)separator),(char *)publickey_base64_miwen,strlen((char *)publickey_base64_miwen));
        #if 0
        FILE *fp2 = NULL;
    	fp2 = fopen("/mnt/miwen.txt","a+");
       
    	if (fp2)
    	{
    	    loge("encrypted_data length : %d\r\n",strlen(encrypted_data));
    		fwrite(encrypted_data,sizeof(char),strlen(encrypted_data),fp2);
            fwrite("\r\n\r\n\r\n",sizeof(char),6,fp2);
    	}
        #endif
		fp3 = NULL;
    	fp3 = fopen("/mnt/send_to_back.txt","a+");//需要被替换
    	if (fp3)
    	{
    	    loge("encrypted_data_unbase64 length : %d\r\n",strlen(encrypted_data));
			write_length = fwrite("encrypted_data_unbase64:\r\n",sizeof(char),strlen("encrypted_data_unbase64:\r\n"),fp3);
			loge("write_length2 :%d\r\n",write_length);
    		write_length = fwrite(encrypted_data,sizeof(char),strlen(encrypted_data),fp3);
			loge("write_length3 :%d\r\n",write_length);
            fwrite("\r\n\r\n",sizeof(char),4,fp3);
			fclose(fp3);
    	}
        //~{Wi:O:s5DJ}>];9PhR*>-9}~}Base64~{<SC\~}
        base64_encode( (const unsigned char *)encrypted_data, encrypted_data_base64, strlen(encrypted_data) );
		if (strlen(encrypted_data_base64) > sizeof(encrypted_data_base64))
		{
			return send_err;
		}
        #if 0
        if (fp2)
    	{
    	    loge("encrypted_data_base64 length :%d\r\n",strlen(encrypted_data_base64));
    	    fwrite(encrypted_data_base64,sizeof(char),strlen(encrypted_data_base64),fp2);
            fwrite("\r\n\r\n\r\n\r\n\r\n\r\n",sizeof(char),12,fp2);
            fclose(fp2);
        }
        #endif
        char data_1[20] = "jsonStr=";
        int send_data_ack = 0xff;
		if (strlen(data_1) > sizeof(tresponse_string1))
		{
			return send_err;
		}
        memcpy(tresponse_string1,data_1,strlen(data_1));
		if (strlen(data_1) + strlen(encrypted_data_base64) > sizeof(tresponse_string1))
		{
			return send_err;
		}
        memcpy(tresponse_string1 + strlen(data_1),encrypted_data_base64,strlen(encrypted_data_base64));


		fp3 = NULL;
    	fp3 = fopen("/mnt/send_to_back.txt","a+");//需要被替换
    	if (fp3)
    	{
    	    loge("encrypted_data length : %d\r\n",strlen(tresponse_string1));
			write_length = fwrite("encrypt_data:\r\n",sizeof(char),strlen("encrypt_data:\r\n"),fp3);
			loge("write_length4 :%d\r\n",write_length);
    		write_length = fwrite(tresponse_string1,sizeof(char),strlen(tresponse_string1),fp3);
			loge("write_length5 :%d\r\n",write_length);
            fwrite("\r\n\r\n",sizeof(char),4,fp3);
			fclose(fp3);
    	}

		
        gettimeofday(&cal_now, NULL);
        now_1 = cal_now.tv_sec+(cal_now.tv_usec*0.000001);
        send_data_ack = curl_http_send_data1(tmp->web_service_addr,tresponse_string1);
        gettimeofday(&cal_end1, NULL);
        now_3 = cal_end1.tv_sec+(cal_end1.tv_usec*0.000001);
        logd("calx-http----%lf s:%d,%d;e:%d,%d\r\n",(now_3-now_1),cal_now.tv_sec,cal_now.tv_usec,cal_end1.tv_sec,cal_end1.tv_usec);
        loge("view send_data_ack :%d\r\n",send_data_ack);

		fp3 = NULL;
    	fp3 = fopen("/mnt/send_to_back.txt","a+");//需要被替换
		if (fp3)
    	{
    	    
			fwrite("background ack:\r\n",sizeof(char),strlen("background ack:\r\n"),fp3);
			if (send_data_ack == 0)//发送成功
			{
				write_length = fwrite("view_return_update_ok\r\n",sizeof(char),strlen("view_return_update_ok\r\n"),fp3);
				loge("write_length6 :%d\r\n",write_length);
			}
			else
			{
				write_length = fwrite("view_return_update_fail\r\n",sizeof(char),strlen("view_return_update_fail\r\n"),fp3);
				loge("write_length7 :%d\r\n",write_length);
			}
    		
            fwrite("\r\n\r\n\r\n",sizeof(char),6,fp3);
			fclose(fp3);
    	}
		
        if (send_data_ack == 0)//发送成功
        {
            loge("view return update ok\r\n");
             return send_sucess;
        }
        else
        {
             loge("view return update fail\r\n");
             return send_fail;
        }
       
    }
    else
    {
        char data_1[20] = "jsonStr=";
        memcpy(tresponse_string1,data_1,strlen(data_1));
        memcpy(tresponse_string1 + strlen(data_1),response_string1,strlen(response_string1));
        //curl_http_send_data(web_service_address ,response_string1);
        curl_http_send_data1(tmp->web_service_addr,tresponse_string1);
    }
    return send_err;
}


//data_type:~{J}>]VV@`~} compactionData:1  gravelData:2
int produce_sourse_data(encrypt_transmission_set_config *tmp,int data_type,int send_muti)
{
    char response_string1[2000]= {0};
    int ans = 0;
    loge("begin to process\r\n");
    //~{;qH!CwNDJ}>]~}
    if (data_type == type_compactionData)
    {
    	memset(response_string1,'\0',2000);
        if (0 == compaction_data_package(response_string1,tmp))
        {
            return 0;
        }
    }
    if (data_type == type_gravelData)
    {
    	memset(response_string1,'\0',2000);
        ans = gravel_data_package(response_string1,tmp);
        loge("gravel_data_package return %d\r\n",ans);
        if (0 == ans)
        {
            loge("gravel_data_package return 0\r\n");
            return 0;
        }
    }

    //采集和上传分开
    //这是存储

    if (data_type == type_compactionData)
    {//对压实数据而言，还是上传
        char file_data[100] = {0};
        if (compaction_data_judge(file_data) == no_file)
        {
            if (send_fail == data_encrypt_and_update(tmp,response_string1))
            {
                goto data_save;
            }
        }
        else
        {
            loge("view have compaction file\r\n");
            goto data_save;
        }
    }
    else
    {//对碎石数据而言，存储到一个路径，存储和上传分开
        char file_data[100] = {0};
        if (gravel_data_judge(file_data)  == no_file) 
        {
            if (send_fail == data_encrypt_and_update(tmp,response_string1))
            {
                if (reflash_flag_get() == gravel_data_vaild)
                {
                    loge("view four time valid data realtime\r\n");
                    goto data_save;
                }
            }
            else
            {
                reflash_flag_set(0);
            }
        }
        else
        {
            loge("view have gravel file\r\n");
            goto data_save;
        }
    }
    return 1;
data_save:
{
    int tt_ret,msec;
    time_t tt;
    UTC_TIME utc_timex;
    int response_string1_len = 0;
    loge("view 2\r\n");
    tt_ret = get_sec_info_from_ZDA(s_zda_t,&msec,&tt,&utc_timex);
    if (tt_ret == 1)
    {
        loge("no real zda_for_gravel\r\n");
        return 0;
    }
    response_string1_len = strlen(response_string1);
    if ((response_string1_len +2) < sizeof(response_string1) )
    {
        response_string1[response_string1_len] = '\r';
        response_string1[response_string1_len+1] = '\n';
        response_string1_len += 2;
        loge("view save_data_in_cache\r\n");
        if (data_type == type_compactionData)
        {
            logd("save compaction data in cache\r\n");
            save_data_in_cache(COMPACTION_FILE_PATH,response_string1, &utc_timex, response_string1_len );
        }
        if (data_type == type_gravelData)
        {
            logd("save gravel data in cache\r\n");
            save_data_in_cache(GRAVEL_FILE_PATH,response_string1, &utc_timex, response_string1_len );
        }
    }
    if (data_type == type_gravelData)
    {
        reflash_flag_set(0);
    }
    return 0;
}
}

 

//定时执行加密函数
void encrypt_process_thread( union sigval v )
{
    static int times = 0;
    int send_gravel_flag = 0;
    times++;

    if (pthread_mutex_trylock(&s_lock_encrypt_thread) != 0)
    {
        loge("encrypt_process_thread is running\n");
        return;
    }

    loge("encrypt_process1\r\n");
    encrypt_transmission_set_config encrypt_conf; 
    get_encrypt_conf( &encrypt_conf );
	if (encrypt_conf.cer_status && encrypt_conf.transmission_status)
	{}
	else
	{
        goto RET;
	}
    //这一块的逻辑是如果是碎石数据，如果四个时间均有效，则不管是否到了触发(时间器间隔*页面设置)的时间，均处理。
    if (times < encrypt_conf.upload_interval)
    {
        if (encrypt_conf.gravelData == 1)//碎石数据
        {
            if (reflash_flag_get() == 0)
            {
                loge("up_end_time == -1\r\n");
                send_gravel_flag = 0;
                goto RET;
            }
            else
            {
                if (encrypt_conf.gravelData == 0)
                {
                    send_gravel_flag = 0;
                    goto RET;
                }
                else
                {
                    loge("up_end_time != -1\r\n");
                    send_gravel_flag = 1;
                    loge ("view real four time are vaild\r\n");
                }
            }
        }
        else
        {
            if (encrypt_conf.compactionData == 1)//压实数据
            {
                goto RET;
            }
        }
        
    }
    else
    {
        if (encrypt_conf.gravelData == 1)//碎石数据
        {
            send_gravel_flag = 0;
            times = 0;
        }
    }
   
        //loge("encrypt_flag:%d\r\n",encrypt_flag);
        loge("encrypt_process2\r\n");
        if (encrypt_conf.cer_status && encrypt_conf.transmission_status)
        {
            loge("encrypt_flag----xxx\r\n");
            if (strlen(encrypt_conf.web_service_addr) == 0)
            {
                memcpy(encrypt_conf.web_service_addr,default_web_service,strlen(default_web_service));
                loge("encrypt_addr default copy\r\n");
            }
            struct timeval cal_now;
            struct timeval cal_end;
            double now_1 = 0,now_2 = 0;
            gettimeofday(&cal_now, NULL);
            now_1 = cal_now.tv_sec+(cal_now.tv_usec*0.000001);
            //---------------~{EP6OJG7qWi0|Q9J5J}>]~}------------------------
            if (encrypt_conf.compactionData)
            {
                produce_sourse_data(&encrypt_conf,type_compactionData,0);
            }
            else
            {}
            //---------------~{EP6OJG7qWi0|KiJ/J}>]~}------------------------
            if (encrypt_conf.gravelData)
            {
                
                produce_sourse_data(&encrypt_conf,type_gravelData,send_gravel_flag);
                send_gravel_flag = 0;
            }
            else
            {}
            
            gettimeofday(&cal_end, NULL);
            now_2 = cal_end.tv_sec+(cal_end.tv_usec*0.000001);
            logd("calx-calend----%lf s:%d,%d;e:%d,%d\r\n",(now_2-now_1),cal_now.tv_sec,cal_now.tv_usec,cal_end.tv_sec,cal_end.tv_usec);
        }
    
RET:
    pthread_mutex_unlock(&s_lock_encrypt_thread);
	return;
}

//删除指定行数据 num从1开始
/*
* Function:del the num th row
* Paramer: data : path_file; num :the num th
* Return: 0:del sucess; 1:num>total row;  2:
* History: author:zh   date:2017.03.28
*/
int file_del_numth_line(char *path_file,int num)
{
    //char data_print[100] = {0};
    FILE *fp1;
    char tmp[1024] = { 0 }, *tmp_p = NULL;
    int i = 0;
    //printf("-1---\r\n");
    fp1 = fopen(path_file,"r+");
    if (NULL == fp1)
    {
        return 1;
    }
    //printf("0---\r\n");
    #ifdef lock_flie
    lock_set(fp1, F_WRLCK);  //给文件上写入锁
    #endif
    for (i = 0;i<(num-1);i++)
    {
        //printf("file num : %d\r\n",i);
        tmp_p = fgets(tmp, sizeof( tmp ) - 1, fp1 );
        if (tmp_p == NULL)
        {
            //printf("get err\r\n");
            return 1;
        }
        //printf("file num : %d  -1\r\n",i);
    }
    //printf("1---\r\n");
    long file_dest_pos = ftell(fp1);
    long file_sourse_pos = 0;
    tmp_p = fgets(tmp, sizeof( tmp ) - 1, fp1 );
    if (tmp_p == NULL)
    {
        return 1;
    }
   // printf("2---\r\n");
    //读一行，写到上一行。
    int k = num-1;
    while (NULL != fgets(tmp, sizeof( tmp ) - 1, fp1 ))
    {
        file_sourse_pos = ftell(fp1);//源文件读完num+1行的位置
        fseek(fp1,file_dest_pos,SEEK_SET);//删除后的文件num-1行的位置
        //把第num+1行写到第num行
        fputs(tmp,fp1);
        //记录写完的位置
        file_dest_pos = ftell(fp1);
        fseek(fp1,file_sourse_pos,SEEK_SET);//将指针移到num+1行读完的位置
        k++;
        //printf("move line %d to line:%d\r\n",(k+1),k);
    }
    //printf("3---\r\n");
    long now_seek = 0;
	long end_seek = file_dest_pos;
    // printf("4---\r\n");
    #ifdef lock_flie
    lock_set(fp1, F_UNLCK);  //给文件解锁
    #endif
    fclose(fp1);
    if (now_seek == end_seek)
    {
        remove( path_file );
        loge("view remove %s---\r\n",path_file);
    }
    else
    {
        truncate( path_file, end_seek - now_seek );
        loge("view truncate---\r\n");
    }
   // printf("end\r\n");
   
   return 0;
}

void history_update(encrypt_transmission_set_config *para,char *file_data)
{
    FILE *fp;
    char tmp[2000] = { 0 }, *tmp_p = NULL;
    loge("view update_process_thread\r\n");
    if (file_data != NULL)
    {
        //从一个文件里取出数据
		fp = fopen( file_data, "r" );
		if (!fp)
		{
			loge("file null\n");
		}
        else
        {
            #ifdef lock_flie
            lock_set(fp, F_RDLCK);  //给文件上读取锁
            #endif
            //读一行
            tmp_p = fgets( tmp, sizeof( tmp ) - 1, fp );
			if ( tmp_p == NULL )
			{
				loge("++++ fgets error :%d \n", errno);
                //需要对读取文件读不到内容做个判断，是否文件为空
				return;
			}
            loge("view fgets : %s\r\n",tmp);
            //-------------------------------------碎石数据----------------------------------------
            if (para->gravelData != 0)
            {
                //判断4个时间有效    
                char *p1 = NULL;
                long int down_begin_time = 0,down_end_time = 0,up_begin_time = 0,up_end_time = 0;
                
                p1 = strstr(tmp,"down_begin_time");
                if (p1 == NULL)
                {
                    #ifdef lock_flie
                    lock_set(fp, F_UNLCK);  //给文件解锁
                    #endif
                    fclose(fp);
                    loge("view no gravel data\r\n");
                    if (0 == file_del_numth_line(file_data,1))
                    {
                        loge("view file_del_numth_line ok\r\n");
                    }
                    return;
                }
                sscanf(p1,"down_begin_time\":\%ld\",\"down_end_time\":\"%ld\",\"up_begin_time\":\"%ld\",\"up_end_time\":\"%ld\"",&down_begin_time,&down_end_time,&up_begin_time,&up_end_time);
                //判断是否是当天的包
                int gravel_data_send_ack_flag = 0;//gravel data vaild :1 inviail:0
                if ((down_begin_time != -1)&&(down_end_time != -1)&&(up_begin_time != -1)&&(up_end_time != -1))
                {
                    int year_t = 0,month_t = 0,day_t = 0,hour_t= 0;
                    sscanf(file_data,GRAVEL_FILE_PATH"%4d%02d%02d%02d.dat",&year_t,&month_t,&day_t,&hour_t);
                    UTC_TIME time_temp1;
                    get_zda_time(&time_temp1);
                    if ((year_t == time_temp1.year) && (month_t == time_temp1.month) && (day_t == time_temp1.day) && (hour_t == time_temp1.hour))
                    {
                        loge("view four time vaild \r\n");
                        gravel_data_send_ack_flag = 1;
                    }
                    else
                    {
                        loge("view four time invaild 1\r\n");
                        gravel_data_send_ack_flag = 0;
                    }
                }
                else
                {
                    loge("view four time invaild 2\r\n");
                    gravel_data_send_ack_flag = 0;
                }
                //上传
                
                #ifdef lock_flie
                lock_set(fp, F_UNLCK);  //给文件解锁
                #endif
                loge("view gravel_data_send_ack_flag:%d\r\n",gravel_data_send_ack_flag);
                if(1 == data_encrypt_and_update(para,tmp))
                {
                    if (gravel_data_send_ack_flag)
                    {
                        loge("view close file\r\n");
                        fclose(fp);
                        return;
                    }
                }
               //上传成功 或者 上传不为gravel valid数据,可将这行文件删除
                fclose(fp);
                loge("view update file data sucess\r\n");
                if (0 == file_del_numth_line(file_data,1))
                {
                    loge("view file_del_numth_line ok\r\n");
                }
            }
            else
            {}
            //-------------------------------------压实数据----------------------------------------
            if (para->compactionData != 0)
            {
                //判断是压实数据
                char *p1 = NULL;
                p1 = strstr(tmp,"gps_time");
                if (p1 == NULL)//如果这行数据不是压实数据，则删除
                {
                    #ifdef lock_flie
                    lock_set(fp, F_UNLCK);  //给文件解锁
                    #endif
                    fclose(fp);
                    loge("view no compaction data\r\n");
                    if (0 == file_del_numth_line(file_data,1))
                    {
                        loge("view file_del_numth_line ok\r\n");
                    }
                    return;
                }
                //上传
                
                #ifdef lock_flie
                lock_set(fp, F_UNLCK);  //给文件解锁
                #endif
                if(1 == data_encrypt_and_update(para,tmp))
                {
                    loge("view close file\r\n");
                    fclose(fp);
                    return;
                }
                 //上传成功 ,可将这行文件删除
                fclose(fp);
                loge("view update file data sucess\r\n");
                if (0 == file_del_numth_line(file_data,1))
                {
                    loge("view file_del_numth_line ok\r\n");
                }
            }
            else
            {}

        }
    }
       
}




//上传函数
void update_process_thread( union sigval v )
{
    //判断文件路径有没有文件
    char file_data[100] = {0};
    encrypt_transmission_set_config encrypt_conf;

    if (pthread_mutex_trylock(&s_lock_update_thread) != 0)
    {
        loge("update_process_thread is running\n");
        return;
    }

    get_encrypt_conf( &encrypt_conf );
	if (encrypt_conf.cer_status && encrypt_conf.transmission_status)
	{}
	else
	{
        goto RET;
	}
    if ((gravel_data_judge(file_data)  == have_flie)&& (encrypt_conf.gravelData != 0)) 
    {
        history_update(&encrypt_conf,file_data);
       
    }
    else
    {
        logd("gravel judge no file\r\n");
    }
    memset(file_data,0,sizeof(file_data));
    if ((compaction_data_judge(file_data)  == have_flie)&& (encrypt_conf.compactionData != 0)) 
    {
        history_update(&encrypt_conf,file_data);
       
    }
    else
    {
        logd("compaction judge no file\r\n");
    }
RET:
    pthread_mutex_unlock(&s_lock_update_thread);
    return;
}




#define CLOCKID CLOCK_REALTIME


//采集定时器
int timer_init(void)
{
	// XXX int timer_create(clockid_t clockid, struct sigevent *evp, timer_t *timerid);
	// clockid--值：CLOCK_REALTIME,CLOCK_MONOTONIC，CLOCK_PROCESS_CPUTIME_ID,CLOCK_THREAD_CPUTIME_ID
	// evp--存放环境值的地址,结构成员说明了定时器到期的通知方式和处理方式等
	// timerid--定时器标识符
	timer_t timerid;
	struct sigevent evp;
	memset(&evp, 0, sizeof(struct sigevent));		//清零初始化

	//evp.sigev_value.sival_int = 111;			//也是标识定时器的，这和timerid有什么区别？回调函数可以获得
	evp.sigev_notify = SIGEV_THREAD;			//线程通知的方式，派驻新线程
	evp.sigev_notify_function = encrypt_process_thread;		//线程函数地址
    
	if (timer_create(CLOCKID, &evp, &timerid) == -1)
	{
		perror("fail to timer_create");
		exit(-1);
	}
    loge("timer_init0\r\n");
	// XXX int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value,struct itimerspec *old_value);
	// timerid--定时器标识
	// flags--0表示相对时间，1表示绝对时间
	// new_value--定时器的新初始值和间隔，如下面的it
	// old_value--取值通常为0，即第四个参数常为NULL,若不为NULL，则返回定时器的前一个值
	
	//第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长,就是说it.it_value变0的时候会装载it.it_interval的值
	struct itimerspec it;
	it.it_interval.tv_sec = 5;
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = 1;
	it.it_value.tv_nsec = 0;

	if (timer_settime(timerid, 0, &it, NULL) == -1)
	{
		perror("fail to timer_settime");
		exit(-1);
	}
     loge("timer_init1\r\n");
	//pause();

	return 0;
}


//上传定时器
int timer_update_init(void)
{
	// XXX int timer_create(clockid_t clockid, struct sigevent *evp, timer_t *timerid);
	// clockid--值：CLOCK_REALTIME,CLOCK_MONOTONIC，CLOCK_PROCESS_CPUTIME_ID,CLOCK_THREAD_CPUTIME_ID
	// evp--存放环境值的地址,结构成员说明了定时器到期的通知方式和处理方式等
	// timerid--定时器标识符
	timer_t timerid;
	struct sigevent evp;
	memset(&evp, 0, sizeof(struct sigevent));		//清零初始化

	//evp.sigev_value.sival_int = 111;			//也是标识定时器的，这和timerid有什么区别？回调函数可以获得
	evp.sigev_notify = SIGEV_THREAD;			//线程通知的方式，派驻新线程
	evp.sigev_notify_function = update_process_thread;		//线程函数地址
    
	if (timer_create(CLOCKID, &evp, &timerid) == -1)
	{
		perror("fail to timer_create");
		exit(-1);
	}
    loge("timer_init1\r\n");
	// XXX int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value,struct itimerspec *old_value);
	// timerid--定时器标识
	// flags--0表示相对时间，1表示绝对时间
	// new_value--定时器的新初始值和间隔，如下面的it
	// old_value--取值通常为0，即第四个参数常为NULL,若不为NULL，则返回定时器的前一个值
	
	//第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长,就是说it.it_value变0的时候会装载it.it_interval的值
	struct itimerspec it;
	it.it_interval.tv_sec = 5;
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = 10;
	it.it_value.tv_nsec = 0;

	if (timer_settime(timerid, 0, &it, NULL) == -1)
	{
		perror("fail to timer_settime");
		exit(-1);
	}
     loge("timer_init1\r\n");
	//pause();

	return 0;
}


/*
 * int timer_gettime(timer_t timerid, struct itimerspec *curr_value);
 * 获取timerid指定的定时器的值，填入curr_value
 *
 */


void gga_info_extract(char *data, int len)
{
    static unsigned int divl = 0;
    static unsigned int send_count = 0;
	char tmp_gga[1024] = { 0 };
	strncpy(tmp_gga, data, len);
	strcpy(s_gga, tmp_gga);
    int freq_temp = 0;
    static int freq_temp1 = 0;
    
    struct sys_info tmp = { { 0 } };
	get_sys_info(&tmp);
   
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_GGA,1);
    logd("-------------gga output:%d\r\n",GNSS_SET_CONFIG.output_type);

    freq_temp = get_Trimble_NMEA_Config(DAT_ID_GGA,2);
    logd("-------------gga f_freq:%d , n_freq:%d\r\n",freq_temp1,freq_temp);
    if (freq_temp != freq_temp1)
    {
        send_count = 0;
        
    }
    freq_temp1 = freq_temp;
    
    switch (freq_temp)
    {
        case DAT_FRQ_10S:
            logd("-------------gga send_count:%d \r\n",send_count);
            if (send_count % 10 == 0)
            {
                send_count = 0;
                 logd("-------------gga DAT_FRQ_10S output \r\n");
                nmea_data_output_authority(tmp_gga,strlen(tmp_gga),DAT_ID_GGA);
            }
            send_count++;
            break;
        case DAT_FRQ_5S:
            logd("-------------gga send_count:%d \r\n",send_count);
            if (send_count % 5 == 0)
            {
                send_count = 0;
                logd("-------------gga DAT_FRQ_5S output \r\n");
                nmea_data_output_authority(tmp_gga,strlen(tmp_gga),DAT_ID_GGA);
            }
            send_count++;
            break;
        case DAT_FRQ_2S:
            logd("-------------gga send_count:%d \r\n",send_count);
            if (send_count % 2 == 0)
            {
                send_count = 0;
                logd("-------------gga DAT_FRQ_2S output \r\n");
                nmea_data_output_authority(tmp_gga,strlen(tmp_gga),DAT_ID_GGA);
            }
            send_count++;
            break;
        default:
            logd("-------------gga default output \r\n");
            nmea_data_output_authority(tmp_gga,strlen(tmp_gga),DAT_ID_GGA);
            break;
    }

	s_get_gga_flag = 1;

	
	
	if (strncmp(tmp.pn + 4, "06", 2) == 0)		//bd982
	{
        switch(freq_temp)
        {
            case DAT_FRQ_10HZ:
                if (divl++ % 10 != 0)//~{7VF5TZUb6y~}
				{
					return;
				}
                break;
            case DAT_FRQ_5HZ:
                if (divl++ % 5 != 0)//~{7VF5TZUb6y~}
				{
					return;
				}
                break;
            case DAT_FRQ_2HZ:
                if (divl++ % 2 != 0)//~{7VF5TZUb6y~}
				{
					return;
				}
                break;
            case DAT_FRQ_1HZ:
                
                break;
            default:
                
                break;
        }
	}
	
//~{MxR3OTJ>J}>]5HLaH!~}
	{
        unsigned char sv_num_char[4] = {0};
        unsigned char sLat[16] = {0};
        unsigned char sLon[16] = {0};
        unsigned char Lat_NS[2] = {0};
        unsigned char Lon_EW[2] = {0};
        unsigned char sHigh[16] = {0};
        unsigned char sHigh2[16] = {0};
        unsigned char sState[4] = {0};  //~{6(N;W4L,~}
        
        //~{N@PG?EJ}~}
        GetField((unsigned char *)tmp_gga,sv_num_char,7);
        s_sat_count = atoi((const char*)sv_num_char);
        //~{6(N;W4L,~}
        GetField((unsigned char *)tmp_gga,sState,6);
        s_fix_status = atoi((const char*)sState);
        //~{N36H~}
        GetField((unsigned char *)tmp_gga,sLat,2);
        s_latitude = atof((const char*)sLat);
        GetField((unsigned char *)tmp_gga, Lat_NS,3);
        s_south_north_latitude = Lat_NS[0];
        //~{>-6H~}
        GetField((unsigned char *)tmp_gga,sLon,4);
        s_longitude = atof((const char*)sLon);
        GetField((unsigned char *)tmp_gga, Lon_EW,5);
        s_west_east_longitude = Lon_EW[0];
        //~{8_3L~}
        GetField((unsigned char *)tmp_gga,sHigh,9);
        GetField((unsigned char *)tmp_gga, sHigh2,11);
        s_high = atof((const char*)sHigh) + atof((const char*)sHigh2);//~{8_6H2;PhR*W*;;~}
        if (s_sat_count == 0)
		{
			s_longitude = 0; // ????
			s_latitude = 0;  // ~{N3~}??
			s_high = 0;
		}
	}
    logd("s_sat_count:%d,s_fix_status:%d,s_latitude:%f,s_longitude:%f\r\n",s_sat_count,s_fix_status,s_latitude,s_longitude);
   //~{T*J}>]IO4+~}
    encrypt_transmission_set_config encrypt_conf;
    get_encrypt_conf( &encrypt_conf );
    if (encrypt_conf.cer_status && encrypt_conf.transmission_status)
    {
        
        encrypt_flag = 1;
        //produce_sourse_data(&encrypt_conf);
      //  call_encrypt_firware(&encrypt_conf);
    }
}

void progress_gga(char *data, int len)
{
#ifdef DISTANCE_THRESHOLD
	static char last_gga[1024] = { 0 };
#endif
	static unsigned int divl = 0;
    static unsigned int send_count = 0;
	char tmp_gga[1024] = { 0 };
	strncpy(tmp_gga, data, len);
	strcpy(s_gga, tmp_gga);
	int size = 0;
    int freq_temp = 0;
    static int freq_temp1 = 0;
    char close_flag_temp1 = 0;
	static int s_gga_wrong_time = 0;		//gga not valid times for reboot
    static int distance_threshold_30s_count;
    //for test
    char data_tempxx[100];
	//if (strcmp(tmp_gga, "") != 0)
	{
		s_get_gga_flag = 1;
		loge("s_get_gga_flag : %d\r\n",s_get_gga_flag);
		struct sys_info tmp = { { 0 } };
		get_sys_info(&tmp);
        close_flag_temp1 = get_data_close_status();
		if(  close_flag_temp1 == 0)			//only mc100+bd982 need this requirements
		{
		    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_GGA,1);
            logd("-------------gga output:%d\r\n",GNSS_SET_CONFIG.output_type);

            freq_temp = get_Trimble_NMEA_Config(DAT_ID_GGA,2);
            logd("-------------gga f_freq:%d , n_freq:%d\r\n",freq_temp1,freq_temp);
            if (freq_temp != freq_temp1)
            {
                send_count = 0;
                
            }
            freq_temp1 = freq_temp;
            
            switch (freq_temp)
            {
                case DAT_FRQ_10S:
                    logd("-------------gga send_count:%d \r\n",send_count);
                    if (send_count % 10 == 0)
                    {
                        send_count = 0;
                         logd("-------------gga DAT_FRQ_10S output \r\n");
                        nmea_data_output_authority(tmp_gga,strlen(tmp_gga),DAT_ID_GGA);
                    }
                    send_count++;
                    break;
                case DAT_FRQ_5S:
                    logd("-------------gga send_count:%d \r\n",send_count);
                    if (send_count % 5 == 0)
                    {
                        send_count = 0;
                        logd("-------------gga DAT_FRQ_5S output \r\n");
                        nmea_data_output_authority(tmp_gga,strlen(tmp_gga),DAT_ID_GGA);
                    }
                    send_count++;
                    break;
                case DAT_FRQ_2S:
                    logd("-------------gga send_count:%d \r\n",send_count);
                    if (send_count % 2 == 0)
                    {
                        send_count = 0;
                        logd("-------------gga DAT_FRQ_2S output \r\n");
                        nmea_data_output_authority(tmp_gga,strlen(tmp_gga),DAT_ID_GGA);
                    }
                    send_count++;
                    break;
                default:
                    logd("-------------gga default output \r\n");
                    nmea_data_output_authority(tmp_gga,strlen(tmp_gga),DAT_ID_GGA);
                    break;
            }
		}

		

		
		
		if (strncmp(tmp.pn + 4, "06", 2) == 0)		//bd982
		{
            switch(freq_temp)
            {
                case DAT_FRQ_10HZ:
                    if (divl++ % 10 != 0)//~{7VF5TZUb6y~}
    				{
    					return;
    				}
                    break;
                case DAT_FRQ_5HZ:
                    if (divl++ % 5 != 0)//~{7VF5TZUb6y~}
    				{
    					return;
    				}
                    break;
                case DAT_FRQ_2HZ:
                    if (divl++ % 2 != 0)//~{7VF5TZUb6y~}
    				{
    					return;
    				}
                    break;
                case DAT_FRQ_1HZ:
                    
                    break;
                default:
                    
                    break;
            }
		}
		
//~{MxR3OTJ>J}>]5HLaH!~}
		{
            unsigned char sv_num_char[4] = {0};
            unsigned char sLat[16] = {0};
            unsigned char sLon[16] = {0};
            unsigned char Lat_NS[2] = {0};
            unsigned char Lon_EW[2] = {0};
            unsigned char sHigh[16] = {0};
            unsigned char sHigh2[16] = {0};
            unsigned char sState[4] = {0};  //~{6(N;W4L,~}
            
            //~{N@PG?EJ}~}
            GetField((unsigned char *)tmp_gga,sv_num_char,7);
            s_sat_count = atoi((const char*)sv_num_char);
            //~{6(N;W4L,~}
            GetField((unsigned char *)tmp_gga,sState,6);
            s_fix_status = atoi((const char*)sState);
            //~{N36H~}
            GetField((unsigned char *)tmp_gga,sLat,2);
            s_latitude = atof((const char*)sLat);
            GetField((unsigned char *)tmp_gga, Lat_NS,3);
            s_south_north_latitude = Lat_NS[0];
            //~{>-6H~}
            GetField((unsigned char *)tmp_gga,sLon,4);
            s_longitude = atof((const char*)sLon);
            GetField((unsigned char *)tmp_gga, Lon_EW,5);
            s_west_east_longitude = Lon_EW[0];
            //~{8_3L~}
            GetField((unsigned char *)tmp_gga,sHigh,9);
            GetField((unsigned char *)tmp_gga, sHigh2,11);
            s_high = atof((const char*)sHigh) + atof((const char*)sHigh2);//~{8_6H2;PhR*W*;;~}
            if (s_sat_count == 0)
			{
				s_longitude = 0; // ????
				s_latitude = 0;  // ~{N3~}??
				s_high = 0;
			}
		}
        logd("s_sat_count:%d,s_fix_status:%d,s_latitude:%f,s_longitude:%f\r\n",s_sat_count,s_fix_status,s_latitude,s_longitude);
       //~{T*J}>]IO4+~}
        encrypt_transmission_set_config encrypt_conf;
        get_encrypt_conf( &encrypt_conf );
        if (encrypt_conf.cer_status && encrypt_conf.transmission_status)
        {
            
            encrypt_flag = 1;
            //produce_sourse_data(&encrypt_conf);
          //  call_encrypt_firware(&encrypt_conf);
        }
#ifdef DISTANCE_THRESHOLD
		if (strlen(last_gga) != 0)
		{
			double last_lat, last_long,last_h1,last_h2;
			double recent_lat, recent_long,recent_h1,recent_h2;
            char last_ew,last_ns,recent_ew,recent_ns;
			double dist = 0;
			DATA_DISPATCH_STRUCT dis_para;
            
            memset(data_tempxx,0,sizeof(data_tempxx));
            
			// sscanf( last_gga, "$%*[^,],%*f,%2d%lf,%*c,%3d%lf,%*c,%*s",
			// 		&last_lat_d, &last_lat_m, &last_long_d, &last_long_m );
			
			sscanf(last_gga, "$%*[^,],%*f,%lf,%c,%lf,%c,%*d,%*d,%*f,%lf,%*c,%lf,%*c,%*s",&last_lat,&last_ns, &last_long,&last_ew,&last_h1,&last_h2);

			// if ( 4 != sscanf( tmp_gga, "$%*[^,],%*f,%2d%lf,%*c,%3d%lf,%*c,%*s",
			// 				  &recent_lat_d, &recent_lat_m, &recent_long_d, &recent_long_m ) )
			if (6 != sscanf(tmp_gga, "$%*[^,],%*f,%lf,%c,%lf,%c,%*d,%*d,%*f,%lf,%*c,%lf,%*c,%*s",&recent_lat,&recent_ns, &recent_long,&recent_ew,&recent_h1,&recent_h2))
			{
				s_gga_wrong_time++;
				logd("gga is not vaild %d | %s\n", s_gga_wrong_time, tmp_gga);
				if( s_gga_wrong_time > 1200 )
				{
					logd("---------gga is wrong too long, so reboot receiver ! ! !\n");
					system("reboot");
				}
                else
                {
                    s_gga_wrong_time = 0;
                }
				return;
			}

			s_gga_wrong_time = 0;
			gga_utc_time_stick_check( tmp_gga );
            double last_dLat1,last_dLat2,last_dLon1,last_dLon2;
            double recent_dLat1,recent_dLat2,recent_dLon1,recent_dLon2;
            //~{Wn=|R;4N~}GGA~{4&@m~}-------------------------------------------
            //gga~{N36H~}(~{6H7V~})~{W*;/N*6H#,GR8y>]DO11Gx7VJG7q<S8::E~}
            last_dLat1 = (int)last_lat / 100;    //~{U{J}~}
            last_dLat2 = (last_lat - last_dLat1*100) / 60;//~{P!J}~}
            last_lat = last_dLat1 + last_dLat2;
            if(last_ns == 'S')
            {
                last_lat = -last_lat;
            }
            //gga~{>-6H~}(~{6H7V~})~{W*;/N*6H#,GR8y>]6+NwGx7VJG7q<S8::E~}
            last_dLon1 = (int)last_long / 100;    //~{U{J}~}
            last_dLon2 = (last_long - last_dLon1*100) / 60;//~{P!J}~}
            last_long = last_dLon1 + last_dLon2;
            if(last_ew == 'W')
            {
                last_long = -last_long;
            }
            //~{IOR;4N~}GGA~{4&@m~}-------------------------------------------
            //gga~{N36H~}(~{6H7V~})~{W*;/N*6H#,GR8y>]DO11Gx7VJG7q<S8::E~}
            recent_dLat1 = (int)recent_lat / 100;    //~{U{J}~}
            recent_dLat2 = (recent_lat - recent_dLat1*100) / 60;//~{P!J}~}
            recent_lat = recent_dLat1 + recent_dLat2;
            if(recent_ns == 'S')
            {
                recent_lat = -recent_lat;
            }
            //gga~{>-6H~}(~{6H7V~})~{W*;/N*6H#,GR8y>]6+NwGx7VJG7q<S8::E~}
            recent_dLon1 = (int)recent_long / 100;    //~{U{J}~}
            recent_dLon2 = (recent_long - recent_dLon1*100) / 60;//~{P!J}~}
            recent_long = recent_dLon1 + recent_dLon2;
            if(recent_ew == 'W')
            {
                recent_long = -recent_long;
            }
            
			double BLH0[3] = {0};
            double BLH1[3] = {0};
            
            BLH0[0] = last_lat*PAI/180.0;//LAT
            BLH0[1] = last_long*PAI/180.0;//LONG
            BLH0[2] = last_h1 + last_h2;

            BLH1[0] = recent_lat*PAI/180.0;//LAT
            BLH1[1] = recent_long*PAI/180.0;//LONG
            BLH1[2] = recent_h1 + recent_h2;
           // loge("-------------cal ----last_lat : %lf,last_long : %lf,last_h1 : %lf ,last_h2 : %lf\r\n",last_lat,last_long,last_h1,last_h2);
          //  loge("-------------cal ----recent_lat : %lf,recent_long : %lf,recent_h1 : %lf ,recent_h2 : %lf\r\n",recent_lat,recent_long,recent_h1,recent_h2);
           // loge("-------------cal ----BLH0 : %lf, %lf, %lf\r\n",BLH0[0],BLH0[1],BLH0[2]);
          //  loge("-------------cal ----BLH1 : %lf, %lf, %lf\r\n",BLH1[0],BLH1[1],BLH1[2]);
			dist = cal_distance_of_two_point(BLH0,BLH1);
			get_datadispatch_conf(&dis_para);
			if (dist < dis_para.m_threshold)//~{B_<-~}:~{Hg9{J1<d<d8tTZ~}30s~{DZ#,LlO_RF6/5D>`@kP!SZ7'V5#,Tr2;<GB<J}>]#,4sSZ5HSZ7'V5TrIO4+#;~}
			//~{J1<d<d8t4sSZ5HSZ~}30s,~{N^B[JG7q4sSZ5HSZ7'V5>yIO4+~}
			{
			    distance_threshold_30s_count++;
                if (distance_threshold_30s_count %30 == 0)
                {
                    distance_threshold_30s_count = 0;
                }
                else
                {
                    //for test
                 //   logd("unupdata: %lf ,count: %d ,last_lat:%lf ,last_lon:%lf ,last_h1:%lf ,last_h2:%lf ,noww_lat:%lf ,noww_lon:%lf ,noww_h1:%lf ,noww_h2:%lf \r\n",dist,distance_threshold_30s_count,last_lat,last_long,last_h1,last_h2,recent_lat,recent_long,recent_h1,recent_h2);
                 //   logd("last_gga:%s\r\n",last_gga);
                 //   logd("recent_gga:%s\r\n",tmp_gga);
                    
                    //logd("-------------------------------------noupdata: %lf ,count: %d \r\n",dist,distance_threshold_30s_count);
                    return;
                }
			}
            else
            {
                distance_threshold_30s_count = 0;
            }
            

            //for test
            
            //sprintf(data_tempxx,"updata: %lf ,count: %d \r\nlast_lat:%lf ,last_lon:%lf \r\nnoww_lat:%lf ,noww_lon:%lf \r\n",dist,distance_threshold_30s_count,last_lat,last_long,recent_lat,recent_long);
          //  logd("updata: %lf ,count: %d ,last_lat:%lf ,last_lon:%lf ,last_h1:%lf ,last_h2:%lf ,noww_lat:%lf ,noww_lon:%lf ,noww_h1:%lf ,noww_h2:%lf \r\n",dist,distance_threshold_30s_count,last_lat,last_long,last_h1,last_h2,recent_lat,recent_long,recent_h1,recent_h2);
          //  logd("last_gga:%s\r\n",last_gga);
          //  logd("recent_gga:%s\r\n",tmp_gga);
            //logd("-------------------------------------updata: %lf ,count: %d \r\n",dist,distance_threshold_30s_count);
		}
		strcpy(last_gga, tmp_gga);
#endif
		logd("DISTANCE_THRESHOLD cal ok\r\n");
		//get_cmv_data(tmp_cmv);
		//
		{
			char *temp_handle = NULL;
			char *acm_handle = NULL;
            char *PILERESULT_handle = NULL;
            char *sensor_temp_handle = NULL;
            //char acm_handle1[100] = {0};
			char *rfid_handle = NULL;
			int *rfid_len_handle = NULL;
			static int no_refresh_temp_count = 0;
			static int no_refresh_acm_count = 0;
            static int no_refresh_PILERESULT_count = 0;
			static int no_refresh_rfid_count = 0;
            static int no_refresh_sensor_temp_count = 0;
			if (0 == get_temp_buff(&temp_handle))
			{
				no_refresh_temp_count++;
			}
			else
			{
				no_refresh_temp_count = 0;
			}
			
			if (0 == get_acm_buff(&acm_handle))
			{
				no_refresh_acm_count++;
			}
			else
			{
				no_refresh_acm_count = 0;
                //sscanf(acm_handle1,"$ACM,%lf,%lf,%d,%d*",&frequency,&compacrate,&amplitude,&gps_drct);
                //loge("--------------------------acm _ 1:%s\r\n",s_acm);
			}

            if (0 == get_PILERESULT_buff(&PILERESULT_handle))
            {no_refresh_PILERESULT_count++;}
            else
            {no_refresh_PILERESULT_count = 0;}
			if (0 == get_rfid_buff(&rfid_handle, &rfid_len_handle))
			{
				no_refresh_rfid_count++;
			}
			else
			{
				no_refresh_rfid_count = 0;
			}

            if (0 == get_sensor_temp_buff(&sensor_temp_handle))
            {
                no_refresh_sensor_temp_count++;
            }
            else
            {
                no_refresh_sensor_temp_count = 0;
            }
			//strcat( tmp_gga, tmp_cmv );
			size = strlen(tmp_gga);
			loge ("gga_len:%d,gga:%s\r\n",size,tmp_gga);
			if (no_refresh_temp_count <= 2)
			{
			    if ((strlen(temp_handle) + size)<sizeof(tmp_gga))
			    {
                    strcat(tmp_gga, temp_handle);
				    size += strlen(temp_handle);
                }
				else
				{
                    loge("out of buffer\r\n");
                }
			}
			if (no_refresh_acm_count <= 2)
			{
				strcat(tmp_gga, acm_handle);
				size += strlen(acm_handle);
			}
            if (no_refresh_PILERESULT_count <= 2)
            {
                if ((strlen(PILERESULT_handle) + size)<sizeof(tmp_gga))
                {
                    strcat(tmp_gga, PILERESULT_handle);
				    size += strlen(PILERESULT_handle);
                }
                else
                {
                    loge("out of buffer\r\n");
                }
            }
			if (no_refresh_rfid_count <= 2)
			{
				memcpy(tmp_gga + size, rfid_handle, *rfid_len_handle);
				size += *rfid_len_handle;
			}
            if (no_refresh_sensor_temp_count <= 2)
            {
                strcat(tmp_gga + size, sensor_temp_handle);
				size += strlen(sensor_temp_handle);
            }
			//can
			{
				char asd[1024 * 3] = { 0 };
				get_other_buff(asd);
				memcpy(tmp_gga + size, asd, strlen(asd));
				size += strlen(asd);
			}
            if ((size +2)>sizeof(tmp_gga))
            {
                loge("out of buffer\r\n");
                return;
            }
			memcpy(tmp_gga + size, "\r\n", 2);
			size += 2;
			loge ("gga_len1:%d,gga:%s\r\n",size,tmp_gga);
			pub_data(tmp_gga, s_zda_t, size);
		}
		return;
        
	}
}
/*
* Function:~{>2L,7'V5EP1p~}
* Paramer: data:~{51G0J1<d~}gga    len:~{J}>]3$6H~}
* Return:  ~{JG7q7"KM~} 1:~{7"KM~} 0:~{2;7"KM~}
* History: author:zh   date:2017.01.24
*/
int distance_threshold_process(char *data,int len)
{
    #ifdef DISTANCE_THRESHOLD
	    static char last_gga[200] = { 0 };
        char tmp_gga[200] = {0};
        strncpy(tmp_gga, data, len);
        static int s_gga_wrong_time = 0;
        static int distance_threshold_30s_count = 0;
        int send_flag = 0;
		if (strlen(last_gga) != 0)
		{
			double last_lat, last_long,last_h1,last_h2;
			double recent_lat, recent_long,recent_h1,recent_h2;
            char last_ew,last_ns,recent_ew,recent_ns;
			double dist = 0;
			DATA_DISPATCH_STRUCT dis_para;
            

            
			// sscanf( last_gga, "$%*[^,],%*f,%2d%lf,%*c,%3d%lf,%*c,%*s",
			// 		&last_lat_d, &last_lat_m, &last_long_d, &last_long_m );
			
			sscanf(last_gga, "$%*[^,],%*f,%lf,%c,%lf,%c,%*d,%*d,%*f,%lf,%*c,%lf,%*c,%*s",&last_lat,&last_ns, &last_long,&last_ew,&last_h1,&last_h2);

			// if ( 4 != sscanf( tmp_gga, "$%*[^,],%*f,%2d%lf,%*c,%3d%lf,%*c,%*s",
			// 				  &recent_lat_d, &recent_lat_m, &recent_long_d, &recent_long_m ) )
			if (6 != sscanf(tmp_gga, "$%*[^,],%*f,%lf,%c,%lf,%c,%*d,%*d,%*f,%lf,%*c,%lf,%*c,%*s",&recent_lat,&recent_ns, &recent_long,&recent_ew,&recent_h1,&recent_h2))
			{
				s_gga_wrong_time++;
				logd("gga is not vaild %d | %s\n", s_gga_wrong_time, tmp_gga);
				if( s_gga_wrong_time > 1200 )
				{
					logd("---------gga is wrong too long, so reboot receiver ! ! !\n");
					system("reboot");
				}
                else
                {
                    s_gga_wrong_time = 0;
                }
				return send_flag;
			}

			s_gga_wrong_time = 0;
			gga_utc_time_stick_check( tmp_gga );
            double last_dLat1,last_dLat2,last_dLon1,last_dLon2;
            double recent_dLat1,recent_dLat2,recent_dLon1,recent_dLon2;
            //~{Wn=|R;4N~}GGA~{4&@m~}-------------------------------------------
            //gga~{N36H~}(~{6H7V~})~{W*;/N*6H#,GR8y>]DO11Gx7VJG7q<S8::E~}
            last_dLat1 = (int)last_lat / 100;    //~{U{J}~}
            last_dLat2 = (last_lat - last_dLat1*100) / 60;//~{P!J}~}
            last_lat = last_dLat1 + last_dLat2;
            if(last_ns == 'S')
            {
                last_lat = -last_lat;
            }
            //gga~{>-6H~}(~{6H7V~})~{W*;/N*6H#,GR8y>]6+NwGx7VJG7q<S8::E~}
            last_dLon1 = (int)last_long / 100;    //~{U{J}~}
            last_dLon2 = (last_long - last_dLon1*100) / 60;//~{P!J}~}
            last_long = last_dLon1 + last_dLon2;
            if(last_ew == 'W')
            {
                last_long = -last_long;
            }
            //~{IOR;4N~}GGA~{4&@m~}-------------------------------------------
            //gga~{N36H~}(~{6H7V~})~{W*;/N*6H#,GR8y>]DO11Gx7VJG7q<S8::E~}
            recent_dLat1 = (int)recent_lat / 100;    //~{U{J}~}
            recent_dLat2 = (recent_lat - recent_dLat1*100) / 60;//~{P!J}~}
            recent_lat = recent_dLat1 + recent_dLat2;
            if(recent_ns == 'S')
            {
                recent_lat = -recent_lat;
            }
            //gga~{>-6H~}(~{6H7V~})~{W*;/N*6H#,GR8y>]6+NwGx7VJG7q<S8::E~}
            recent_dLon1 = (int)recent_long / 100;    //~{U{J}~}
            recent_dLon2 = (recent_long - recent_dLon1*100) / 60;//~{P!J}~}
            recent_long = recent_dLon1 + recent_dLon2;
            if(recent_ew == 'W')
            {
                recent_long = -recent_long;
            }
            
			double BLH0[3] = {0};
            double BLH1[3] = {0};
            
            BLH0[0] = last_lat*PAI/180.0;//LAT
            BLH0[1] = last_long*PAI/180.0;//LONG
            BLH0[2] = last_h1 + last_h2;

            BLH1[0] = recent_lat*PAI/180.0;//LAT
            BLH1[1] = recent_long*PAI/180.0;//LONG
            BLH1[2] = recent_h1 + recent_h2;
           // loge("-------------cal ----last_lat : %lf,last_long : %lf,last_h1 : %lf ,last_h2 : %lf\r\n",last_lat,last_long,last_h1,last_h2);
          //  loge("-------------cal ----recent_lat : %lf,recent_long : %lf,recent_h1 : %lf ,recent_h2 : %lf\r\n",recent_lat,recent_long,recent_h1,recent_h2);
           // loge("-------------cal ----BLH0 : %lf, %lf, %lf\r\n",BLH0[0],BLH0[1],BLH0[2]);
          //  loge("-------------cal ----BLH1 : %lf, %lf, %lf\r\n",BLH1[0],BLH1[1],BLH1[2]);
			dist = cal_distance_of_two_point(BLH0,BLH1);
			get_datadispatch_conf(&dis_para);
			if (dist < dis_para.m_threshold)//~{B_<-~}:~{Hg9{J1<d<d8tTZ~}30s~{DZ#,LlO_RF6/5D>`@kP!SZ7'V5#,Tr2;<GB<J}>]#,4sSZ5HSZ7'V5TrIO4+#;~}
			//~{J1<d<d8t4sSZ5HSZ~}30s,~{N^B[JG7q4sSZ5HSZ7'V5>yIO4+~}
			{
			    distance_threshold_30s_count++;
                if (distance_threshold_30s_count %30 == 0)
                {
                    distance_threshold_30s_count = 0;
                    send_flag = 1;
                }
                else
                {
                    //for test
                 //   logd("unupdata: %lf ,count: %d ,last_lat:%lf ,last_lon:%lf ,last_h1:%lf ,last_h2:%lf ,noww_lat:%lf ,noww_lon:%lf ,noww_h1:%lf ,noww_h2:%lf \r\n",dist,distance_threshold_30s_count,last_lat,last_long,last_h1,last_h2,recent_lat,recent_long,recent_h1,recent_h2);
                 //   logd("last_gga:%s\r\n",last_gga);
                 //   logd("recent_gga:%s\r\n",tmp_gga);
                    
                    //logd("-------------------------------------noupdata: %lf ,count: %d \r\n",dist,distance_threshold_30s_count);
                    send_flag = 0; 
                }
			}
            else
            {
                distance_threshold_30s_count = 0;
                send_flag = 1;
            }
            

            //for test
            
            //sprintf(data_tempxx,"updata: %lf ,count: %d \r\nlast_lat:%lf ,last_lon:%lf \r\nnoww_lat:%lf ,noww_lon:%lf \r\n",dist,distance_threshold_30s_count,last_lat,last_long,recent_lat,recent_long);
          //  logd("updata: %lf ,count: %d ,last_lat:%lf ,last_lon:%lf ,last_h1:%lf ,last_h2:%lf ,noww_lat:%lf ,noww_lon:%lf ,noww_h1:%lf ,noww_h2:%lf \r\n",dist,distance_threshold_30s_count,last_lat,last_long,last_h1,last_h2,recent_lat,recent_long,recent_h1,recent_h2);
          //  logd("last_gga:%s\r\n",last_gga);
          //  logd("recent_gga:%s\r\n",tmp_gga);
            //logd("-------------------------------------updata: %lf ,count: %d \r\n",dist,distance_threshold_30s_count);
		}
		strcpy(last_gga, tmp_gga);
        return send_flag;
#endif

}

void progress_zda(char *data, int len)
{
    char close_flag_temp2 = 0;
    int hour_t,min_t,sec_t,ret;
    static int hour_t1,min_t1,sec_t1;
	strncpy(s_zda, data, len);
    ret = sscanf( s_zda, "$%*[^,],%2d%2d%2d.%*2d,",&hour_t,&min_t,&sec_t );
    loge("zda0:%s\r\n",s_zda);
    if (ret == 3)
    {
        if ((hour_t != hour_t1)||(min_t != min_t1)||(sec_t != sec_t1) )
        {
            strncpy(s_zda_t, data, len);
            loge("zda:%s\r\n",s_zda_t);
        }
        else
        {
             loge("zda1:%s\r\n",s_zda_t);
        }
        hour_t1 = hour_t;
        min_t1 = min_t;
        sec_t1 = sec_t;
    }
    close_flag_temp2 = get_data_close_status();
    if (close_flag_temp2 == 0)
    {
        GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_ZDA,1);
        nmea_data_output_authority(data, len,DAT_ID_ZDA);
    }

}

void progress_hdt(char *data, int len)
{
	//static int counter = 0;
	//char tmpdata[64] = { 0 };

	//memcpy( tmpdata, data, len );
    /*
	counter++;
	if( 0 == counter%10 )
	{
		sscanf( tmpdata, "$GPHDT,%lf,%*s", &s_azimuth );
	}
	if( counter >10000 )
	{
		counter = 1;
	}
*/
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_HDT,1);
    nmea_data_output_authority(data, len,DAT_ID_HDT);
	
}

//$GPVTG,341.25,T,-658.65,M,0.01,N,0.02,K,D*01
void progress_vtg(char *data, int len)
{
    sscanf(data,"%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],%*[^,],N,%lf,K",&vtg_speed);
    loge("vtg:%s,speed :%lf \r\n",data,vtg_speed);
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_VTG,1);
    nmea_data_output_authority(data, len,DAT_ID_VTG);
}

void progress_gsv(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_GSV,1);
    nmea_data_output_authority(data, len,DAT_ID_GSV);
}

void progress_rmc(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_RMC,1);
    nmea_data_output_authority(data, len,DAT_ID_RMC);
}

void progress_gll(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_GLL,1);
    nmea_data_output_authority(data, len,DAT_ID_GLL);
}

void progress_gst(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_GST,1);
    nmea_data_output_authority(data, len,DAT_ID_GST);
}

void progress_gsa(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_GSA,1);
    nmea_data_output_authority(data, len,DAT_ID_GSA);
}

void progress_rot(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_ROT,1);
    nmea_data_output_authority(data, len,DAT_ID_ROT);
}

void progress_vgk(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_VGK,1);
    nmea_data_output_authority(data, len,DAT_ID_VGK);
}

void progress_vhd(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_VHD,1);
    nmea_data_output_authority(data, len,DAT_ID_VHD);
}

void progress_avr(char *data, int len)
{
    sscanf((const char *)data,"$PTNL,AVR,%*[^,],%lf,Yaw",
                    &s_azimuth);
                    logd("----------------------azimuth:%lf\r\n",s_azimuth);
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_AVR,1);
    nmea_data_output_authority(data, len,DAT_ID_AVR);
}

void progress_ggk(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_GGK,1);
    nmea_data_output_authority(data, len,DAT_ID_GGK);
}

void progress_bpq(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_BPQ,1);
    nmea_data_output_authority(data, len,DAT_ID_BPQ);
}

void progress_pjk(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_PJK,1);
    nmea_data_output_authority(data, len,DAT_ID_PJK);
}

void progress_pjt(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_PJT,1);
    nmea_data_output_authority(data, len,DAT_ID_PJT);
}

void progress_blh(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_BLH,1);
    nmea_data_output_authority(data, len,DAT_ID_BLH);
}

void progress_time(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_TIME,1);
    nmea_data_output_authority(data, len,DAT_ID_TIME);
}

void progress_dop(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_DOP,1);
    nmea_data_output_authority(data, len,DAT_ID_DOP);
}

void progress_grs(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_GRS,1);
    nmea_data_output_authority(data, len,DAT_ID_GRS);
}

void progress_gns(char *data, int len)
{
    GNSS_SET_CONFIG.output_type = get_Trimble_NMEA_Config(DAT_ID_GNS,1);
    nmea_data_output_authority(data, len,DAT_ID_GNS);
}

void send_diff_to_gnss(char *data, int len)
{
	static int diff_count = 0;
	if (s_gnss_fd >= 0)
	{
		write(s_gnss_fd, data, len);
		s_gnss_get_diff_flag = 1;
		diff_count++;
		if (diff_count >= 180 && s_fix_status != 4 && s_fix_status != 5)
		{
			logd("===get diff data but not diff exit    receive rtcm  do exit \n");
			exit(0);
		}
		else
		{
			diff_count = 0;
		}
	}
}

void get_gnss_ui_info(int *sat_count, int *fix_status)
{
	*sat_count = s_sat_count;
	*fix_status = s_fix_status;
}

int get_gnss_get_diff_flag()
{
	int tmp = s_gnss_get_diff_flag;
	s_gnss_get_diff_flag = 0;
	return tmp;
}

int get_gnss_gga(char *out, int *len)
{
	memcpy(out, s_gga, strlen(s_gga));
	*len = strlen(s_gga);
	return 0;
}

int gnss_get_satellite_count(int sockfd, msg_head_t *pMsgHead, char *msg_data)
{
	if (!pMsgHead)
	{
		return ERR;
	}

	if (pMsgHead->msg_type != MSG_TYPE(CATEGORY_GNSS, OP_CODE_GNSS__GET_SATELLITE_COUNT))
	{
		logi("msg_type( %d, %d ) wrong\n", MSG_CATEGORY( pMsgHead->msg_type ), MSG_OPCODE( pMsgHead->msg_type ));
		return ERR;
	}

	logi("gnss_get_satellite_count %d\n", s_sat_count);

	cmd_response_massage(sockfd, pMsgHead, &s_sat_count, sizeof(s_sat_count));

	return OK;
}

int gnss_get_position(int sockfd, msg_head_t *pMsgHead, char *msg_data)
{
	DAT_STRUCT_BLH blh = { 0 };

	if (!pMsgHead)
	{
		return ERR;
	}

	if (pMsgHead->msg_type != MSG_TYPE(CATEGORY_GNSS, OP_CODE_GNSS__GET_POSITION))
	{
		logi("msg_type( %d, %d ) wrong\n", MSG_CATEGORY( pMsgHead->msg_type ), MSG_OPCODE( pMsgHead->msg_type ));
		return ERR;
	}

	logi("gnss_get_position\n");

	blh.h = s_high;
	blh.latitude = s_latitude;
	blh.longitude = s_longitude;
	blh.west_east = s_west_east_longitude;
	blh.south_north = s_south_north_latitude;
	blh.azimuth = s_azimuth;

	cmd_response_massage(sockfd, pMsgHead, &blh, sizeof(DAT_STRUCT_BLH));

	return OK;
}

int gnss_get_fix_status(int sockfd, msg_head_t *pMsgHead, char *msg_data)
{
	if (!pMsgHead)
	{
		return ERR;
	}

	if (pMsgHead->msg_type != MSG_TYPE(CATEGORY_GNSS, OP_CODE_GNSS__GET_FIX_STATUS))
	{
		logi("msg_type( %d, %d ) wrong\n", MSG_CATEGORY( pMsgHead->msg_type ), MSG_OPCODE( pMsgHead->msg_type ));
		return ERR;
	}

	logi("gnss_get_fix_status\n");

	//  fix_status 1: ????   2: ???   4: ???  5:????
	cmd_response_massage(sockfd, pMsgHead, &s_fix_status, sizeof(s_fix_status));

	return OK;
}

void gnss_commands_init()
{
	register_msg_process_fun(MSG_TYPE(CATEGORY_GNSS, OP_CODE_GNSS__GET_SATELLITE_COUNT), gnss_get_satellite_count);
	register_msg_process_fun(MSG_TYPE(CATEGORY_GNSS, OP_CODE_GNSS__GET_POSITION), gnss_get_position);
	register_msg_process_fun(MSG_TYPE(CATEGORY_GNSS, OP_CODE_GNSS__GET_FIX_STATUS), gnss_get_fix_status);
}

