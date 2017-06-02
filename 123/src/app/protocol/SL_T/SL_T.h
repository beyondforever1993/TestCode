#ifndef __SL_T_H
#define __SL_T_H

/************************************控制字符定义 ***************************************************/
#define SOH  0x7E7E  //帧起始 
#define STX  0x02  //传输正文起始 
#define SYN  0x16  多包传输正文起始 
#define ETX  03  //报文结束，后续无报文 
#define ETB  17  //报文结束，后续有报文 
#define ENQ  05  //询问 
#define EOT  04  //传输结束，退出 
#define ACK  06  //肯定确认，继续发送 
#define NAK  15  //否定应答，反馈重发 
#define ESC  1B  //传输结束，终端保持在线 
 
/**************************************功能码定义****************************************************/
#define LLWC    0x2F  //链路维持报 
#define TEST    0x30  //测试报 
#define JYSD    0x31  //均匀时段水文信息报 
#define DINGSH  0x32  //遥测站定时报 
#define JIAB    0x33  //遥测站加报报 
#define XIAOS   0x34  //遥测站小时报 
#define RGZS    0x35  //遥测站人工置数报 
#define PIC     0x36  //遥测站图片报或中心站查询遥测站图片信息 
#define CSS     0x37  //中心站查询遥测站实时数据 
#define CSD     0x38  //中心站查询遥测站时段数据 
#define CRGZS   0x39  //中心站查询遥测站人工置数 
#define CZDYS   0x3A  //中心站查询遥测站指定要素数据 
#define XPZ     0x40  //中心站修改遥测站基本配置表 
#define CPZ     0x41  //中心站读取遥测站基本配置表/遥测站基本配置表 
#define XYX     0x42  //中心站修改遥测站运行参数配置表 
#define DYX     0x43  //中心站读取遥测站运行参数配置表/遥自报运行参数配置表 
#define CSB     0x44  //查询水泵电机实时工作数据 
#define CVER    0x45  //查询遥测终端软件版本 
#define CSTA    0x46  //查询遥测站状态和报警信息 
#define DDATA   0x47  //初始化固态存储数据 
#define DPAR    0x48  //恢复终端出厂设置 
#define XPASSWD 0x49  //修改密码 
#define SCLK    0x4A  //设置遥测站时钟 
#define SIC     0x4B  //设置遥测终端 IC 卡状态 
//#define 0x4C  //控制水泵开关命令/水泵状态信息自报 
//#define 0x4D  //控制阀门开关命令/阀门状态信息自报 
//#define 0x4E  //控制闸门开关命令/闸门状态信息自报 
//#define 0x4F  //水量定值控制命令 
#define CREC    0x50  //中心站查询遥测站事件记录 
#define CCLK    0x51  //中心站查询遥测站时钟

/**************************************功能码定义****************************************************/

#define TT      0xF0    //观测时间引导符
#define ST      0xF1    //测站编码引导符
#define RGZS    0xF2    //人工置数
#define PIC     0xF3    //图片信息
#define DRP     0xF4    //1小时内每 5 分钟时段雨量(每组雨量占 1 字节EX，最大值 25.4 毫米，数据中不含小数点；FF表示非法数据.)
#define DRZ1    0xF5    //1小时内5分钟间隔相对水位1(每组水位占2字节HEX，分辨力是为厘米，最大值为 655.34 米，数据中不含小数点； FFFF 表示非法数据)；对于河道、闸坝（泵）站分别表示河道水位、闸（站）上水位。  
#define DRZ2    0xF6    //1小时内 5 分钟间隔相对水位 2；对于闸坝（泵）站表示闸（站）下水位。 
#define DRZ3    0xF7    //1小时内 5 分钟间隔相对水位 3 
#define DRZ4    0xF8    //1小时内 5 分钟间隔相对水位 4 
#define DRZ5    0xF9    //1小时内 5 分钟间隔相对水位 5 
#define DRZ6    0xFA    //1小时内 5 分钟间隔相对水位 6 
#define DRZ7    0xFB    //1小时内 5 分钟间隔相对水位 7 
#define DRZ8    0xFC    //1小时内 5 分钟间隔相对水位 8 
#define DATA    0xFD    //流速批量数据传输
#define AC      0x01    //断面面积 
#define AI      0x02    //瞬时气温 
#define C       0x03    //瞬时水温 
#define DRxnn   0x04    //时间步长码 g
#define DT      0x05    //时段长,降水、引排水、抽水历时 
#define ED      0x06    //日蒸发量 
#define EJ      0x07    //当前蒸发 
#define FL      0x08    //气压 
#define G       0x09    //闸坝、水库闸门开启高度 
#define GN      0x0A    //输水设备、闸门(组)编号 
#define GS      0x0B    //输水设备类别 
#define GT      0x0C    //水库、闸坝闸门开启孔数 
#define GTP     0x0D    //地温 
#define H       0x0E    //地下水瞬时埋深 
#define W       0x0F    //波浪高度 
#define M10     0x10    //10 厘米处土壤含水量 
#define M20     0x11    //20 厘米处土壤含水量 
#define M30     0x12    //30 厘米处土壤含水量 
#define M40     0x13    //40 厘米处土壤含水量 
#define M50     0x14    //50 厘米处土壤含水量 
#define M60     0x15    //60 厘米处土壤含水量 
#define M80     0x16    //80 厘米处土壤含水量 
#define M100    0x17    //100 厘米处土壤含水量 
#define MST     0x18    //湿度 
#define NS      0x19    //开机台数 
#define P1      0x1A    //1小时时段降水量 
#define P2      0x1B    //2小时时段降水量 
#define P3      0x1C    //3小时时段降水量 
#define P6      0x1D    //6小时时段降水量 
#define P12     0x1E    //12 小时时段降水量 
#define PD      0x1F    //日降水量 
#define PJ      0x20    //当前降水量 
#define PN01    0x21    //1分钟时段降水量 
#define PN05    0x22    //5分钟时段降水量 
#define PN10    0x23    //10 分钟时段降水量 
#define PN30    0x24    //30 分钟时段降水量 

#endif
