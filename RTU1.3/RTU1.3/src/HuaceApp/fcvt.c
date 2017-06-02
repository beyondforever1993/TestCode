/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: fcvt.c
**创   建   人:
**最后修改日期: 2014年08月12日
**描        述:
********************************************************************************************************/

#include "includes.h"

#define ZEROPAD 1               // Pad with zero
#define SIGN    2               // Unsigned/signed long
#define PLUS    4               // Show plus
#define SPACE   8               // Space if plus
#define LEFT    16              // Left justified
#define SPECIAL 32              // 0x
#define LARGE   64              // Use 'ABCDEF' instead of 'abcdef'

#define is_digit(c) ((c) >= '0' && (c) <= '9')

static char * cvt( double arg, int ndigits, int * decpt, int * sign, char * buf, int eflag )
{
    int r2;
    double fi, fj;
    char * p, * p1;

    if ( ndigits < 0 ) ndigits = 0;
    if ( ndigits >= CVTBUFSIZE - 1 ) ndigits = CVTBUFSIZE - 2;
    r2 = 0;
    * sign = 0;
    p = & buf[0];
    if ( arg < 0 )
    {
        * sign = 1;
        arg = - arg;
    }
    arg = modf( arg, & fi );
    p1 = & buf[CVTBUFSIZE];

    if ( fi != 0 )
    {
        p1 = & buf[CVTBUFSIZE];
        while ( fi != 0 )
        {
            fj = modf( fi / 10, & fi );
            *-- p1 = ( int )(( fj + .03 ) * 10 ) + '0';
            r2 ++;
        }
        while ( p1 < & buf[CVTBUFSIZE] ) * p ++ = * p1 ++;
    }
    else if ( arg > 0 )
    {
        while (( fj = arg * 10 ) < 1 )
        {
            arg = fj;
            r2 --;
        }
    }
    p1 = & buf[ndigits];
    if ( eflag == 0 ) p1 += r2;
    * decpt = r2;
    if ( p1 < & buf[0] )
    {
        buf[0] = '\0';
        return buf;
    }
    while ( p <= p1 && p < & buf[CVTBUFSIZE] )
    {
        arg *= 10;
        arg = modf( arg, & fj );
        * p ++ = ( int ) fj + '0';
    }
    if ( p1 >= & buf[CVTBUFSIZE] )
    {
        buf[CVTBUFSIZE - 1] = '\0';
        return buf;
    }
    p = p1;
    * p1 += 5;
    while (* p1 > '9' )
    {
        * p1 = '0';
        if ( p1 > buf )
            ++*-- p1;
        else
        {
            * p1 = '1';
            (* decpt )++;
            if ( eflag == 0 )
            {
                if ( p > buf ) * p = '0';
                p ++;
            }
        }
    }
    * p = '\0';
    return buf;
}

/*char *ecvt(double arg, int ndigits, int *decpt, int *sign)
{
return cvt(arg, ndigits, decpt, sign, gettib()->cvtbuf, 1);
}*/

char * ecvtbuf( double arg, int ndigits, int * decpt, int * sign, char * buf )
{
    return cvt( arg, ndigits, decpt, sign, buf, 1 );
}

/*char *fcvt(double arg, int ndigits, int *decpt, int *sign)
{
return cvt(arg, ndigits, decpt, sign, gettib()->cvtbuf, 0);
}*/

char * fcvtbuf( double arg, int ndigits, int * decpt, int * sign, char * buf )
{
    return cvt( arg, ndigits, decpt, sign, buf, 0 );
}
void cfltcvt( double value, char * buffer, char fmt, int precision )
{
    int decpt, sign, exp, pos;
    char * digits = NULL;
    char cvtbuf[CVTBUFSIZE + 1];
    int capexp = 0;
    int magnitude;
    if ( fmt == 'G' || fmt == 'E' )
    {
        capexp = 1;
        fmt += 'a' - 'A';
    }
    if ( fmt == 'g' )
    {
        digits = ecvtbuf( value, precision, & decpt, & sign, cvtbuf );
        magnitude = decpt - 1;
        if ( magnitude < - 4  ||  magnitude > precision - 1 )
        {
            fmt = 'e';
            precision -= 1;
        }
        else
        {
            fmt = 'f';
            precision -= decpt;
        }
    }
    if ( fmt == 'e' )
    {
        digits = ecvtbuf( value, precision + 1, & decpt, & sign, cvtbuf );

        if ( sign ) * buffer ++ = '-';
        * buffer ++ = * digits;
        if ( precision > 0 ) * buffer ++ = '.';
        memcpy( buffer, digits + 1, precision );
        buffer += precision;
        * buffer ++ = capexp ? 'E' : 'e';
        if ( decpt == 0 )
        {
            if ( value == 0.0 )
                exp = 0;
            else
                exp = - 1;
        }
        else
            exp = decpt - 1;

        if ( exp < 0 )
        {
            * buffer ++ = '-';
            exp = - exp;
        }
        else
            * buffer ++ = '+';
        buffer[2] = ( exp % 10 ) + '0';
        exp = exp / 10;
        buffer[1] = ( exp % 10 ) + '0';
        exp = exp / 10;
        buffer[0] = ( exp % 10 ) + '0';
        buffer += 3;
    }
    else if ( fmt == 'f' )
    {
        digits = fcvtbuf( value, precision, & decpt, & sign, cvtbuf );
        if ( sign ) * buffer ++ = '-';
        if (* digits )
        {
            if ( decpt <= 0 )
            {
                * buffer ++ = '0';
                * buffer ++ = '.';
                for ( pos = 0; pos < - decpt; pos ++) * buffer ++ = '0';
                while (* digits ) * buffer ++ = * digits ++;
            }
            else
            {
                pos = 0;
                while (* digits )
                {
                    if ( pos ++ == decpt ) * buffer ++ = '.';
                    * buffer ++ = * digits ++;
                }
            }
        }
        else
        {
            * buffer ++ = '0';
            if ( precision > 0 )
            {
                * buffer ++ = '.';
                for ( pos = 0; pos < precision; pos ++) * buffer ++ = '0';
            }
        }
    }
    * buffer = '\0';
}
