/*
 ============================================================================
 Name        : HRCCoder.c
 Author      : Luv
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "cmn.h"
//#include "hc_log.h"
//define global variable

hrc_dat_buf_t m_hrc_buf;

//extern void DoNothing(void* p){};//to get rid of warning
rtcm_t rtcm_send;
int Initialize_rtcm( rtcm_t *rtcm_send )
{
	double cp[ MAXSAT ][ NFREQ + NEXOBS ] = { { 0 } }; /* carrier-phase measurement */
	memcpy( cp, rtcm_send->cp, sizeof( cp ) );
	memset( rtcm_send, 0, sizeof(rtcm_t) );
	memcpy( rtcm_send->cp, cp, sizeof( cp ) );
	return 0;
}
int encodeMSM4( const obs_t *obs, const nav_t *nav, gtime_t time, unsigned char *buff ) //obs
{
	int buffnbyte = 0, i = 0, sys = 0;
	int nGPS = 0, nGlonass = 0, nCompass = 0;

	//memset( &rtcm_send, 0, sizeof( rtcm_t ) );
	Initialize_rtcm( &rtcm_send );
	rtcm_send.obs = *obs;
	rtcm_send.nav = *nav;
	rtcm_send.time = time;
	if( buff == NULL )
	{
		return -1;
	}
	//********start encoding
	for( i = 0; i < obs->n; i++ )
	{
		sys = satsys( obs->data[ i ].sat, NULL );
		if( sys == SYS_GPS )
		{
			nGPS++;
		}
		else if( sys == SYS_GLO )
		{
			nGlonass++;
		}
		else if( sys == SYS_CMP )
		{
			nCompass++;
		}
	}
	//logd("666666 obs->n:%d, ngps:%d, nglo:%d, ncmp:%d \n", obs->n, nGPS, nGlonass, nCompass);
	int sync = 0;
	//GPS Observation
	if( nGlonass > 0 || nCompass > 0 )
	{
		sync = 1;
	}
	else
	{
		sync = 0;
	}
	if( nGPS > 0 && gen_rtcm3( &rtcm_send, 1074, sync ) )
	{
		memcpy( buff + buffnbyte, rtcm_send.buff, rtcm_send.nbyte );
		buffnbyte += rtcm_send.nbyte;
	}

	if( nCompass > 0 )
	{
		sync = 1;
	}
	else
	{
		sync = 0;
	}
	if( nGlonass > 0 && gen_rtcm3( &rtcm_send, 1084, sync ) )
	{
		memcpy( buff + buffnbyte, rtcm_send.buff, rtcm_send.nbyte );
		buffnbyte += rtcm_send.nbyte;
	}
	//Compass Observation
	sync = 0;
	if( nCompass > 0 && gen_rtcm3( &rtcm_send, 1124, sync ) )
	{
		memcpy( buff + buffnbyte, rtcm_send.buff, rtcm_send.nbyte );
		buffnbyte += rtcm_send.nbyte;
	}
	return buffnbyte;
}
int encodeNav( nav_t *nav, int ephsat, unsigned char *buff )
{
	int buffnbyte = 0, sys;
	Initialize_rtcm( &rtcm_send );
	//rtcm_send.obs =;
	rtcm_send.nav = *nav;
	rtcm_send.ephsat = ephsat;
	if( buff == NULL )
	{
		return -1;
	}
	//********start encoding
	int sync = 0;
	sys = satsys( ephsat, NULL );
	if( sys == SYS_GPS )
	{
		if( gen_rtcm3( &rtcm_send, 1019, sync ) )
		{
			memcpy( buff + buffnbyte, rtcm_send.buff, rtcm_send.nbyte );
			buffnbyte += rtcm_send.nbyte;
		}

	}
	else if( sys == SYS_GLO )
	{
		if( gen_rtcm3( &rtcm_send, 1020, sync ) )
		{
			memcpy( buff + buffnbyte, rtcm_send.buff, rtcm_send.nbyte );
			buffnbyte += rtcm_send.nbyte;
		}

	}
	else if( sys == SYS_CMP )
	{
		eph_t *eph = rtcm_send.nav.eph + rtcm_send.ephsat - 1;
		eph->week -= 1356; //to adjust to tya
		if( gen_rtcm3( &rtcm_send, 1047, sync ) )
		{
			memcpy( buff + buffnbyte, rtcm_send.buff, rtcm_send.nbyte );
			buffnbyte += rtcm_send.nbyte;
		}
	}
	else //unsupported system type
	{
		return -1;
	}
	//********end encoding
	return buffnbyte;
}

extern void conv_hrc_obs_buf( hrc_dat_buf_t *dat_buf, const raw_t *raw )
{
	int len;
	gtime_t time;

	if( !dat_buf || raw->obs.n <= 0 )
	{
		fprintf( stderr, "!dat_buf || raw->obs.n <= 0\n" );
		return;
	}
	sortobs( (obs_t *)&raw->obs ); //tya
	time = raw->obs.data[ 0 ].time;

	/* save slips */
	//saveslips(slips,str->obs->data,str->obs->n);/*no use*/
	//if ( !screent( time, opt->ts, opt->te, opt->tint ) )
	//{
	//	fprintf(stderr, "!screent( time, opt->ts, opt->te, opt->tint )" );				
	//    return;
	//}
	//fprintf(stderr, "opt->ts[%s], opt->te[%s], opt->tint[%f]\n", time_str(opt->ts,0), time_str(opt->te,0), opt->tint );
	//outrnxobsb_buf( dat_buf, opt, raw->obs.data, raw->obs.n, 0 );
#if 1    
	len = encodeMSM4( &raw->obs, &raw->nav, time, dat_buf->cur );
	if( len == -1 )
	{
		fprintf( stderr, " encodeMSM4 err! %d \n", len );
		return;
	}
	dat_buf->len = len;
	//fprintf(stderr, " encodeMSM4 ret = %d \n", len);
	//write(conn_fd, dat_buf->cur, ret);
#endif

	//if ( opt->tstart.time == 0 )
	//{
	//    opt->tstart = time;
	//}
	//opt->tend = time;

	//n[0]++;
}

// checked
extern void conv_hrc_nav_buf( hrc_dat_buf_t *dat_buf, raw_t *raw )
{
#if 1 

	int len;

	len = encodeNav( &raw->nav, raw->ephsat, dat_buf->cur );
	if( len == -1 )
	{
		fprintf( stderr, " encodeNav err! %d \n", len );
		return;
	}
	dat_buf->len = len;
	//fprintf(stderr, " encodeNav ret = %d \n", len);

#endif
}

