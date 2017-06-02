#include "shuiwei.c"
#include "qingxie.c"
#include "yewei.c"

/******************************************�궨��*************************************************/
#define _485DATASZ          4

/*******************************************����**************************************************/
#pragma pack(1)
typedef struct{//485���ݶ���
uint8_t aucData[_485DATASZ];
}st485Def;
#pragma pack()
/*****************************************��������************************************************/
st485CfgDef  *const pst485Cfg = &(stRtuCfg.stSenCfg.st485Cfg); 
st485Def     st485Buf;//485���涨��

/******************************************��������***********************************************/

/****************************************static��������*********************************************/
/*
����: _485RestProc()
����:
    1.485��λ����
����:   void
����ֵ: void
����:   void
���:   void
��ע:   
    1.�������ʱ仯ʱִ�и�λ����
ע��:   void
*/
static void _485RestProc(void)
{
    static uint32_t ulBaud = 0;//������

    if (ulBaud != pst485Cfg->baud)
    {
        ulBaud = pst485Cfg->baud;
        _485ChangeBaud(ulBaud);
    }
    return;
}

/*
����: __485Send()
����:
    1.���ݴ��������ͷ���ָ��Э�������
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void __485Send(void)
{
    switch (pst485Cfg->enType)
    {
        case SWJ:
        {
            SwSendCmd();
            break;
        }
        default:
        {
            break;
        }
    }
}

/*
����: _485Deal()
����:
    1.485���ݽ���
����:   
    1.pucTmp:   �����Դ�������ԭʼ����
    2.ucLen:    ���յ������ݳ���
    2.pucData:  ָ�������ɵĴ��������ݻ����ָ��
����ֵ: void
����:   void
���:   void
��ע:   
    1.�������ʱ仯ʱִ�и�λ����
ע��:   void
*/
static void _485Deal(uint8_t *pucTmp, uint8_t ucLen, uint8_t *pucData)
{
    switch (pst485Cfg->enType)
    {
        case SWJ:
        {
            SwDeal(pucTmp, ucLen, pucData);
            break;
        }
        default:
        {
            break;
        }
    }
}


/*
����: _485Collect()
����:
    1.�ɼ�485����
����:   
    1.pucData: ָ�����ݻ����ָ��
����ֵ: void
����:   void
���:   void
��ע:   
    1.�������ʱ仯ʱִ�и�λ����
ע��:   void
*/
static void _485Collect(uint8_t *pucData)
{
    static uint8_t aucTmp[100] = {0};
    uint8_t ucLen = 0;

    __485Send();
    ucLen = _485Recv(&aucTmp[0], sizeof(aucTmp));//��ȡ�����Դ�����������
    if (ucLen)
    {//���յ�����������
        _485Deal(aucTmp, ucLen, pucData);
    }
    return;
}

/*
����: __485Init()
����:
    1.ִ��485���Ӳ����ʼ��
    2.��ʼ��485���ݻ���ָ��
����:   
    1.ppucData: ָ�����ݻ����ָ��
����ֵ: void
����:   void
���:   void
��ע:   
    1.�������ʱ仯ʱִ�и�λ����
ע��:   void
*/
static void __485Init(st485Def **ppstData)
{
    _485Init();//485Ӳ����ʼ��
    ppstData = &st485Buf;
    return;
}

/****************************************extern��������*********************************************/

/*
����: _485Proc()
����:
    1.485���������ݲɼ�����
����:   
    1.pucData: ָ�����ݻ����ָ��
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void _485Proc(void *pvData)
{
    _485RestProc();
    _485Collect((uint8_t *)pvData);
    return;
}

/*
����: _485Default()
����:
    1.485���������� default
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void _485Default(void)
{
    pst485Cfg->baud   = 9600;
    pst485Cfg->enType = SWJ;
    pst485Cfg->para0  = 0;
    pst485Cfg->para1  = 0;
    return;
}

