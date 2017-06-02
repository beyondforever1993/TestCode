#ifndef _VSPRINTF_DEFINED_
#define _VSPRINTF_DEFINED_

//int sprintf( char * buf, const char * fmt, ... );
//char * flt( char * str, double num, int size, int precision, char fmt, int flags );
//int vsprintf( char * buf, const char * fmt, ... );

void cfltcvt( double value, char * buffer, char fmt, int precision );
#endif
