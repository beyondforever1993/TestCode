/*
�ļ�����: app.c
����:
    1.����app ��صĺ���
����: ������
�޸ļ�¼:
    2017-5-11 �ļ�����
��ע:   void
ע��:   void
 */
#include "include.h"



/******************************************�궨��*************************************************/

/*******************************************����**************************************************/

/*****************************************��������************************************************/

/******************************************��������***********************************************/
/****************************************static��������*********************************************/

/****************************************extern��������*********************************************/

/*
����: AppInit()
����:
    1.App Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void AppInit(void)
{
    AppLedInit();
    E2ReadCfg(&stRtuCfg);
    return;
}
