#include "include.h"



/*
����: DelayUs()
����:
    1.Delay 1us
����:   
    1.us
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void DelayUs(uint16_t us)                //��Լ1ms
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
����: DelayMs()
����:
    1.Delay 1ms
����:   
    1.ms
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void DelayMs(uint16_t ms)                //��Լ1ms
{
    uint16_t  delay = 100;

    while(delay--)
    {
        DelayUs(ms);
    }
    return;
}

