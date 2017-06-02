#include "include.h"



/*
名称: DelayUs()
功能:
    1.Delay 1us
参数:   
    1.us
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void DelayUs(uint16_t us)                //大约1ms
{
    uint16_t  delay;

    while(us)
    {
        for(delay = 0; delay < 200; delay++)
        {
                ;
        }
        us--;
    }  
    return;
}

/*
名称: DelayMs()
功能:
    1.Delay 1ms
参数:   
    1.ms
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void DelayMs(uint16_t ms)                //大约1ms
{
    uint16_t  delay = 100;

    while(delay--)
    {
        DelayUs(ms);
    }
    return;
}

