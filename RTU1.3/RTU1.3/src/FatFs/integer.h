/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _FF_INTEGER
#define _FF_INTEGER
#ifdef _WIN32
#include <windows.h>
#else

/* These types must be 16-bit, 32-bit or larger integer */
#ifndef INT
typedef int				INT;
#endif
typedef unsigned int	UINT;

/* These types must be 8-bit integer */
//typedef signed char		CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types must be 16-bit integer */
typedef short			SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;

/* These types must be 32-bit integer */
typedef long			LONG;
typedef unsigned long	ULONG;
#ifndef DWORD
typedef unsigned long	DWORD;
#endif

/* Boolean type */
//typedef enum { FALSE = 0, TRUE } BOOL;
typedef unsigned int BOOL;

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

#endif

#endif
/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _FF_INTEGER
#define _FF_INTEGER
#ifdef _WIN32
#include <windows.h>
#else

/* These types must be 16-bit, 32-bit or larger integer */
#ifndef INT
typedef int				INT;
#endif
typedef unsigned int	UINT;

/* These types must be 8-bit integer */
//typedef signed char		CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types must be 16-bit integer */
typedef short			SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;

/* These types must be 32-bit integer */
typedef long			LONG;
typedef unsigned long	ULONG;
#ifndef DWORD
typedef unsigned long	DWORD;
#endif

/* Boolean type */
//typedef enum { FALSE = 0, TRUE } BOOL;
typedef unsigned int BOOL;

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

#endif

#endif
