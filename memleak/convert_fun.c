/*
 *
 *
 *
 */

//header
#include <string.h>

#include "convert_fun.h"
//#include "gnss_fun.h"

//global variable



//static variable
static int 		s_stream_fromat = 20;
static raw_t	s_raw_hrc = {{0}};


//extern function declare
extern int  input_raw_zy( raw_t *raw, int format, unsigned char *data, int len, int *read_len );
extern void conv_hrc_obs_buf( hrc_dat_buf_t *dat_buf, const raw_t *raw );
extern void conv_hrc_nav_buf( hrc_dat_buf_t *dat_buf, raw_t *raw );


/*
 *int convert_gnss_oem_type_to_stream_format( )
 *{
 *    OEM_TYPE	oem_type = 0;
 *    
 *    oem_type = device_get_oemtype();
 *    switch( oem_type )
 *    {
 *        case OEM_TYPE_UNICORE:
 *            s_stream_fromat = STRFMT_UB370;
 *        break;
 *        case OEM_TYPE_TRIMBLE:
 *            s_stream_fromat = STRFMT_TRIMBLE;
 *        break;
 *        case OEM_TYPE_NOVATEL:
 *            s_stream_fromat = STRFMT_OEM4;
 *        break;
 *        case OEM_TYPE_HEMISPHERE:
 *            s_stream_fromat = STRFMT_CRES;
 *        break;
 *        default:
 *        break;
 *    }
 *
 *    return HC_OK;
 *}
 *
 */

//local function define
void construct_hrc_buf( char *data, int len, hrc_dat_buf_t *dat_buf )
{
	int 	ret = 0;
	int 	read_len = 0;
	char	*pdata = NULL;
		
	if ( !dat_buf )
	{
		return;
	}

	init_raw( &s_raw_hrc );
	
    /*
	 *convert_gnss_oem_type_to_stream_format();
     */

	for( pdata = data;  pdata < ( data + len ); pdata += read_len )
	{	
		ret = input_raw_zy( &s_raw_hrc, s_stream_fromat, (unsigned char*)pdata, data + len - pdata, &read_len );
		switch ( ret )
		{
		case 1:		//�۲��ļ�
			conv_hrc_obs_buf( dat_buf, &s_raw_hrc );			
			break;		
		case 2:		//�����ļ�
			conv_hrc_nav_buf( dat_buf, &s_raw_hrc );
			break;		
		case 9:
			break;
		default:	//other
			break;		
		}
	}

#if 0
	FILE *pfile;
	pfile = fopen( "/mnt/1234567.HRC", "a+" );
	//logd("666666 hrc data-type: %d , data-len: %d \n", ret, dat_buf->len);
	fwrite( dat_buf->buf, 1, dat_buf->len, pfile );
	fflush( pfile );
	fclose( pfile );
#endif

	free_raw( &s_raw_hrc );
	
}





