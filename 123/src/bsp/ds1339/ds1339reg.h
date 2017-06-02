#ifndef __DS1339_REG_H
#define __DS1339_REG_H

typedef enum{
SecReg,
MinReg,
HourReg,
DayReg,
DateReg,
MonthReg,
YearReg,
ASecReg,
AMinReg,
AHourReg,
ADayReg,
ADateReg,
AMonthReg,
AYearReg,
CtrlReg,
StaReg,
TrickleReg,
}stRegDef;

#define EOSC        (1 << 7)// Enable Oscillator

#endif

