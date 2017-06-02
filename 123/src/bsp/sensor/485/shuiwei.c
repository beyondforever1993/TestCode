/*
文件名称: shuiwei.c
功能:
    1.包含 CHR.WYS-1压力水位计 驱动相关的函数
作者: 杜在连
修改记录:
    2017-5-16 文件初创
备注:   void
注意:   void
 */

/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/

/*****************************************变量定义************************************************/
static const uint8_t aucSwCmd[] = {0x09, 0x03, 0x00, 0x01, 0x00, 0x02, 0x94, 0x83 };//水位485命令定义

/******************************************函数定义***********************************************/
/****************************************static函数定义*********************************************/

/****************************************extern函数定义*********************************************/

/*
名称: SwSendCmd()
功能:
    1.通过485向水位传感器发送数据
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void SwSendCmd(void)
{
    _485Send(&aucSwCmd[0], sizeof(aucSwCmd));
    return;
}

/*
名称: SwDeal()
功能:
    1.解析来自水位传感器的数据
参数:   
    1.pucTmp:   接收自传感器的原始数据
    2.pucData:  指向解析完成的传感器数据缓存的指针
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
static void SwDeal(uint8_t *pucTmp, uint8_t ucLen, uint8_t *pucData)
{
    uint32_t ulCrc = 0;

    //UartPrintBuffer(pucTmp, ucLen);
    
    LPC_CRC->SEED = 0xFFFF;
    
    ulCrc = CRC_CalcBlockChecksum(pucTmp, ucLen - 2, CRC_WR_8BIT);
    if (((ulCrc & 0xff) != pucTmp[ucLen - 2]) || (((ulCrc >> 8) & 0xff) != pucTmp[ucLen - 1]))
    {
        printf("485 Crc Error!\r\n");
        goto Return;
    }
    memcpy(pucData, &pucTmp[3], 4);
Return:
    return;
}
