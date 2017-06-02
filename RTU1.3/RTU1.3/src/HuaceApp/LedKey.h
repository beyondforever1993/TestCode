#ifndef  LEDKEY_H
#define  LEDKEY_H

#include "Global.h"

#define  APP_CFG_TASK_KEY_PRIO                        10
#define  APP_CFG_TASK_LED_DATA_PRIO                   12
#define  APP_CFG_TASK_LED_SATA_PRIO                   11
#define  APP_CFG_TASK_LED_RADIO_PRIO                  13
#define  APP_CFG_TASK_LED_POWER_PRIO                  14
#define  APP_CFG_TASK_KEY_STK_SIZE                        96
#define  APP_CFG_TASK_LED_DATA_STK_SIZE                   64
#define  APP_CFG_TASK_LED_SATA_STK_SIZE                   64
#define  APP_CFG_TASK_LED_RADIO_STK_SIZE                  64
#define  APP_CFG_TASK_LED_POWER_STK_SIZE                  64
extern OS_STK    App_TaskKeyStk[APP_CFG_TASK_KEY_STK_SIZE];
extern OS_STK    App_TaskLedDataStk[APP_CFG_TASK_LED_DATA_STK_SIZE];
extern OS_STK    App_TaskLedSataStk[APP_CFG_TASK_LED_SATA_STK_SIZE];
extern OS_STK    App_TaskLedRadioStk[APP_CFG_TASK_LED_RADIO_STK_SIZE];
extern OS_STK    App_TaskLedPowerStk[APP_CFG_TASK_LED_POWER_STK_SIZE];
void  App_Task_Key                     (void       *p_arg);
void  App_Task_Led_Data                (void       *p_arg);
void  App_Task_Led_Sata                (void       *p_arg);
void  App_Task_Led_Radio               (void       *p_arg);
void  App_Task_Led_Power               (void       *p_arg);

#ifdef X701
#define  APP_CFG_TASK_LED_RADIOR_PRIO                 15
#define  APP_CFG_TASK_LED_NET_PRIO                    16
#define  APP_CFG_TASK_LED_NET_STK_SIZE                    64
#define  APP_CFG_TASK_LED_RADIOR_STK_SIZE                 64
extern OS_STK    App_TaskLedNetStk[APP_CFG_TASK_LED_NET_STK_SIZE];
extern OS_STK    App_TaskLedRadioRStk[APP_CFG_TASK_LED_RADIOR_STK_SIZE];
void  App_Task_Led_RadioR               (void       *p_arg);
void  App_Task_Led_Net                  (void       *p_arg);
#endif

#ifdef X701
struct LED_MOD{
    UINT8 Data;
    UINT8 Sate;
    UINT8 Power;
    UINT8 Net;
    UINT8 Radio;
    UINT8 RadioR;
};
#else
struct LED_MOD{
    UINT8 Data;
    UINT8 Sate;
    UINT8 Radio;
    UINT8 Power;
};
#endif
extern struct LED_MOD g_LedMod;
extern UINT8 g_KeyPower;
extern UINT8 g_KeyStatic;

#ifdef X701
void    SetDataLed();
void    SetSateLed();
void    SetPowerLed();
void    SetNetLed();
void    SetRadioHLed();
void    SetRadioLLed();
void    SetRadioRHLed();
void    SetRadioRLLed();
void    ClearDataLed();
void    ClearSateLed();
void    ClearPowerLed();
void    ClearNetLed();
void    ClearRadioHLed();
void    ClearRadioLLed();
void    ClearRadioRHLed();
void    ClearRadioRLLed();
#else
void ClearDataLed();
void ClearSateLed();
void ClearRadioLed();
void ClearPowerLed();

void SetDataLed();
void SetSateLed();
void SetRadioLed();
void SetPowerLed();
#endif
void PowerOffSys();
void PowerOnSys();
void LedCtrlNetOk();//not for x701
#endif