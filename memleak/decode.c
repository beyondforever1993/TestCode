#include "cmn.h"
//#include "hc_log.h"

extern int input_raw( raw_t *raw, int format, unsigned char data )
{
	trace( 5, "input_raw_ex: format=%d data=0x%02x\n", format, data );

	switch( format )
	{
		case STRFMT_UB370:
			return input_ub370( raw, data );
		case STRFMT_TRIMBLE:
			return input_rt17( raw, data );
		default:
			break;
	}
	return 0;
}

extern int input_raw_zy( raw_t *raw, int format, unsigned char *data, int len, int *read_len )
{
	switch( format )
	{
		case STRFMT_UB370:
			return input_ub370_zy( raw, data, len, read_len );
		case STRFMT_TRIMBLE:
			return input_rt17_zy( raw, data, len, read_len );
		case STRFMT_OEM4:
			return input_oem4_zy( raw, data, len, read_len );
		case STRFMT_CRES:
			return input_cres_zy( raw, data, len, read_len );
			break;        //zy add
		default:
			break;
	}
	return 0;
}

