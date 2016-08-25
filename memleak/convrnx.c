#include "cmn.h"

/* global variables ----------------------------------------------------------*/
static const int navsys[] =     /* system codes */
{
    SYS_GPS, SYS_GLO, SYS_GAL, SYS_QZS, SYS_SBS, SYS_CMP, 0
};
//#define NOUTFILE        7       /* number of output files */
/* show status message -------------------------------------------------------*/
extern int showstat( int sess, gtime_t ts, gtime_t te, int *n )
{
    const char type[] = "ONGHQLSE";
    char msg[1024] = "", *p = msg, s[64];
    int i;
    
    if ( sess > 0 )
    {
        p += sprintf( p, "(%d) ", sess );
    }
    if ( ts.time != 0 )
    {
        time2str( ts, s, 0 );
        p += sprintf( p, "%s", s );
    }
    if ( te.time != 0 && timediff( te, ts ) > 0.9 )
    {
        time2str( te, s, 0 );
        p += sprintf( p, "-%s", s + 5 );
    }
    p += sprintf( p, ": " );
    
    for ( i = 0; i < NOUTFILE + 1; i++ )
    {
        if ( n[i] == 0 )
        {
            continue;
        }
        p += sprintf( p, "%c=%d%s", type[i], n[i], i < NOUTFILE ? " " : "" );
    }
    return showmsg( msg );
}
extern void setapppos( raw_t *raw, rnxopt_t *opt )
{
    prcopt_t prcopt = prcopt_default;
    sol_t sol = {{0}};
    char msg[128];
    
    prcopt.navsys = opt->navsys;
    
    /* point positioning with last obs data */
    if ( !pntpos( raw->obs.data, raw->obs.n, &raw->nav, &prcopt, &sol, NULL, NULL,
                  msg ) )
    {
        trace( 2, "point position error (%s)\n", msg );
        return;
    }
    matcpy( opt->apppos, sol.rr, 3, 1 );
}

/* convert rinex obs type ver.3 -> ver.2 -------------------------------------*/
static void convcode( int ver, int sys, char *type )
{
    if ( ver ==3 && ( sys == SYS_GPS || sys == SYS_QZS || sys == SYS_SBS || sys == SYS_CMP ) &&
            !strcmp( type + 1, "1C" ) ) /* L1C/A */
    {
        strcpy( type + 1, "A" );
    }
    else if ( ver == 3 && ( sys == SYS_GPS || sys == SYS_QZS ) &&
              ( !strcmp( type + 1, "1S" ) || !strcmp( type + 1, "1L" ) ||
                !strcmp( type + 1, "1X" ) ) ) /* L1C */
    {
        strcpy( type + 1, "B" );
    }
    else if ( ver == 3 && ( sys == SYS_GPS || sys == SYS_QZS || sys == SYS_CMP ) &&
              ( !strcmp( type + 1, "2S" ) || !strcmp( type + 1, "2L" ) ||
                !strcmp( type + 1, "2X" ) ) ) /* L2C */
    {
        strcpy( type + 1, "C" );
    }
    else if ( ver == 3 && sys == SYS_GLO && !strcmp( type + 1, "1C" ) ) /* L1C/A */
    {
        strcpy( type + 1, "A" );
    }
    else if ( ver == 3 && sys == SYS_GLO && !strcmp( type + 1, "2C" ) ) /* L2C/A */
    {
        strcpy( type + 1, "D" );
    }
    else if (  sys == SYS_CMP && ( !strcmp( type + 1, "1I" ) || !strcmp( type + 1, "1Q" ) ||
                             !strcmp( type + 1, "1X" ) ) ) /* B1 */
    {
        strcpy( type + 1, "1" );
    }
    else if (  sys == SYS_CMP && ( !strcmp( type + 1, "7I" ) || !strcmp( type + 1, "7Q" ) ||
                                  !strcmp( type + 1, "7X" ) ) ) /* B2 */
    {
        strcpy( type + 1, "2" );
    }
    else if (  sys == SYS_CMP && ( !strcmp( type + 1, "6I" ) || !strcmp( type + 1, "6Q" ) ||
                                  !strcmp( type + 1, "6X" ) ) ) /* B3 */
    {
        strcpy( type + 1, "5" );
    }
    else if (  sys == SYS_GAL && ( !strcmp( type + 1, "1X" )||!strcmp( type + 1, "1Z" ) )  ) /* E2-L1-E1 */
    {
        strcpy( type + 1, "1" );
    }
    else if (  sys == SYS_GAL && ( !strcmp( type + 1, "5X" ))  ) /* E5a */
    {
        strcpy( type + 1, "5" );
    }
    else if (  sys == SYS_GAL && ( !strcmp( type + 1, "7X" )) ) /* E5b */
    {
        strcpy( type + 1, "7" );
    }
	else if (  sys == SYS_GAL && ( !strcmp( type + 1, "8X" )) ) /* E5a+b */
    {
        strcpy( type + 1, "8" );
    }
    else if (  sys == SYS_GAL && ( !strcmp( type + 1, "6X" )||!strcmp( type + 1, "6Z" )) ) /* E6 */
    {
        strcpy( type + 1, "6" );
    }
    else if ( !strcmp( type, "C1P" ) || !strcmp( type, "C1W" ) || !strcmp( type, "C1Y" ) ||
              !strcmp( type, "C1N" ) ) /* L1P,P(Y) */
    {
        strcpy( type, "P1" );
    }
    else if ( !strcmp( type, "C2P" ) || !strcmp( type, "C2W" ) || !strcmp( type, "C2Y" ) ||
              !strcmp( type, "C2N" ) || !strcmp( type, "C2D" ) ) /* L2P,P(Y) */
    {
        strcpy( type, "P2" );
    }
    else if ( !strcmp( type, "L6I" ) )
    {
        strcpy( type, "L5" );
    }
    else if ( !strcmp( type, "L7C" ) )
    {
        strcpy( type, "L2" );
    }
    else if ( !strcmp( type, "C6I" ) )
    {
        strcpy( type, "P5" );
    }
    else if ( !strcmp( type, "S6I" ) )
    {
        strcpy( type, "S5" );
    }
    else if ( !strcmp( type, "D6I" ) )
    {
        strcpy( type, "D5" );
    }
    else if ( !strcmp( type, "C7C" ) )
    {
        strcpy( type, "P2" );
    }
    else if ( !strcmp( type, "S7C" ) )
    {
        strcpy( type, "S2" );
    }
    else if ( !strcmp( type, "D7C" ) )
    {
        strcpy( type, "D2" );
    }
    else
    {
        type[2] = '\0';
    }
}
static void setopt_obstype( const unsigned char *codes,
                            const unsigned char *types, int sys, rnxopt_t *opt )
{
    const char type_str[] = "CLDS"; /*code;phase;dopler;snr*/
    char type[16], *id;
    int i, j, k, freq;
    
    trace( 3, "setopt_obstype: sys=%d\n", sys );
    
    opt->nobs[sys] = 0;
    
    if ( !( navsys[sys]&opt->navsys ) )
    {
        return;
    }
    
    for ( i = 0; codes[i]; i++ )
    {
    
        if ( !( id = code2obs( codes[i], &freq ) ) )
        {
            continue;
        }
        
        if ( !( opt->freqtype & ( 1 << ( freq - 1 ) ) ) || opt->mask[sys][codes[i] - 1] == '0' )
        {
            continue;
        }
        for ( j = 0; j < 4; j++ ) /*CLDS*/
        {
            if ( !( opt->obstype & ( 1 << j ) ) )
            {
                continue;
            }
            if ( types && !( types[i] & ( 1 << j ) ) )
            {
                continue;
            }
            
            /* observation type in ver.3 */
            sprintf( type, "%c%s", type_str[j], id );
            if ( type[0] == 'C' && type[2] == 'N' )
            {
                continue;    /* codeless */
            }
            
            if ( opt->rnxver == 2 ) /* ver.2 */
            {
            
                /* ver.3 -> ver.2 */
                convcode( opt->rnxver, navsys[sys], type );
                
                /* check duplicated observation type */
                
                for ( k = 0; k < opt->nobs[0]; k++ )
                {
                    if ( !strcmp( opt->tobs[0][k], type ) )
                    {
                        break;
                    }
                }
                if ( k >= opt->nobs[0] && opt->nobs[0] < MAXOBSTYPE )
                {
                    strcpy( opt->tobs[0][opt->nobs[0]++], type );
                }
                
            }
            else if ( opt->nobs[sys] < MAXOBSTYPE ) /* ver.3 */
            {
                strcpy( opt->tobs[sys][opt->nobs[sys]++], type );
            }
        }
    }
}
/*format:数据格式或板卡类型 tya*/
/* set observation types -----------------------------------------------------*/
extern void set_obstype( int format, rnxopt_t *opt )
{
    /* supported codes by rtcm2 */
    const unsigned char codes_rtcm2[6][8] =
    {
        {CODE_L1C, CODE_L1P, CODE_L2C, CODE_L2P},
        {CODE_L1C, CODE_L1P, CODE_L2C, CODE_L2P}
    };
    /* supported codes by rtcm3 */
    const unsigned char codes_rtcm3[6][8] =
    {
        {CODE_L1C, CODE_L1W, CODE_L2W, CODE_L2X, CODE_L5X},
        {CODE_L1C, CODE_L1P, CODE_L2C, CODE_L2P},
        {CODE_L1X, CODE_L5X, CODE_L7X, CODE_L8X},
        {CODE_L1C, CODE_L2X, CODE_L5X},
        {CODE_L1C, CODE_L5X},
        {CODE_L1C, CODE_L2C, CODE_L7I}
    };
    /* supported codes by novatel oem3 */
    const unsigned char codes_oem3[6][8] =
    {
        {CODE_L1C, CODE_L2P}, {0}, {0}, {0}, {CODE_L1C}
    };
    /* supported codes by novatel oem4 zy add bds*/
    const unsigned char codes_oem4[6][8] =
    {
        {CODE_L1C, CODE_L2P, CODE_L2D, CODE_L2X, CODE_L5Q},
        {CODE_L1C, CODE_L2C, CODE_L2P},
        {CODE_L1B, CODE_L1C, CODE_L5Q, CODE_L7Q, CODE_L8Q},
        {CODE_L1C, CODE_L2X, CODE_L5Q},
        {CODE_L1C, CODE_L5I},
        {CODE_L1I, CODE_L7I, CODE_L6I}
    };
    /* supported codes by javad */
    const unsigned char codes_javad[6][8] =
    {
        {CODE_L1C, CODE_L1W, CODE_L1X, CODE_L2X, CODE_L2W, CODE_L5X},
        {CODE_L1C, CODE_L1P, CODE_L2C, CODE_L2P},
        {CODE_L1X, CODE_L5X, CODE_L7X, CODE_L8X},
        {CODE_L1C, CODE_L1X, CODE_L1Z, CODE_L2X, CODE_L5X},
        {CODE_L1C, CODE_L5X},
        {CODE_L1C, CODE_L2C, CODE_L7I}
    };
    /* supported codes by rinex and binex */
    const unsigned char codes_rinex[6][32] =
    {
        {
            CODE_L1C, CODE_L1P, CODE_L1W, CODE_L1Y, CODE_L1M, CODE_L1N, CODE_L1S, CODE_L1L,
            CODE_L2C, CODE_L2D, CODE_L2S, CODE_L2L, CODE_L2X, CODE_L2P, CODE_L2W, CODE_L2Y,
            CODE_L2M, CODE_L2N, CODE_L5I, CODE_L5Q, CODE_L5X
        },
        {CODE_L1C, CODE_L1P, CODE_L2C, CODE_L2P, CODE_L3I, CODE_L3Q, CODE_L3X},
        {
            CODE_L1C, CODE_L1A, CODE_L1B, CODE_L1X, CODE_L1Z, CODE_L5I, CODE_L5Q, CODE_L5X,
            CODE_L6A, CODE_L6B, CODE_L6C, CODE_L6X, CODE_L6Z, CODE_L7I, CODE_L7Q, CODE_L7X,
            CODE_L8I, CODE_L8Q, CODE_L8X
        },
        {
            CODE_L1C, CODE_L1S, CODE_L1L, CODE_L1X, CODE_L1Z, CODE_L2S, CODE_L2L, CODE_L2X,
            CODE_L5I, CODE_L5Q, CODE_L5X, CODE_L6S, CODE_L6L, CODE_L6X
        },
        {CODE_L1C, CODE_L5I, CODE_L5Q, CODE_L5X},
        {
            //CODE_L2I, CODE_L2Q, CODE_L2X, CODE_L7I, CODE_L7Q, CODE_L7X, CODE_L6I, CODE_L6Q,
            CODE_L1I, CODE_L1Q, CODE_L1X, CODE_L7I, CODE_L7Q, CODE_L7X, CODE_L6I, CODE_L6Q,
            CODE_L6X
        }
    };
    /*tya supported codes by ub370,6-navsys*/
    const unsigned char codes_ub370[6][8] =
    {
        {CODE_L1C, CODE_L2P},
        {CODE_L1C, CODE_L2P},
        {CODE_L1C, CODE_L2P},
        {CODE_L1C, CODE_L2P},
        {CODE_L1C, CODE_L2P},
        {CODE_L1I, CODE_L7I, CODE_L6I}
        //{CODE_L1C,CODE_L2P,CODE_L5I},
    };
    /*tya supported codes by TRIMBLE*/
    const unsigned char codes_trimble[6][5] =
    {
        {CODE_L1C, CODE_L2P, CODE_L5X},
        {CODE_L1C, CODE_L2P},
    //    {CODE_L1X, CODE_L5X, CODE_L7X,CODE_L8X,CODE_L6X},
        {CODE_L1X, CODE_L5X, CODE_L7X,CODE_L8X},
        {CODE_L1C, CODE_L2P},
        {CODE_L1C, CODE_L2P},
        {CODE_L1I, CODE_L7I, CODE_L6I},
        //{CODE_L1C,CODE_L2P,CODE_L5I},
    };
    /* supported codes by hemisphere */
    /*
    const unsigned char codes_cres[6][8] =
    {
        {CODE_L1C, CODE_L2P},
        {CODE_L1C, CODE_L2P},
        {0}, {0}, {CODE_L1C},
        {CODE_L1I, CODE_L1Q, CODE_L7I, CODE_L7Q, CODE_L6I, CODE_L6Q }
    };
    */
    const unsigned char codes_cres[6][8] =
    {
        {CODE_L1C, CODE_L2P},
        {CODE_L1C, CODE_L2P},
        {0}, {0}, {CODE_L1C},
        {CODE_L1I, CODE_L7I, CODE_L6I }
    };
    /* supported codes by others */
    const unsigned char codes_other[6][8] =
    {
        {CODE_L1C}, {CODE_L1C}, {0}, {0}, {CODE_L1C}
    };
    const unsigned char *codes;
    int i;
    
    trace( 3, "set_obstype: format=%d\n", format );
    
    for ( i = 0; i < 6; i++ ) /* the num of satellite systems */
    {
        switch ( format )
        {
            case STRFMT_UB370:
                codes = codes_ub370[i];
                break;//tya
            case STRFMT_RTCM2:
                codes = codes_rtcm2[i];
                break;
            case STRFMT_RTCM3:
                codes = codes_rtcm3[i];
                break;
            case STRFMT_OEM4 :
                codes = codes_oem4 [i];
                break;
            case STRFMT_OEM3 :
                codes = codes_oem3 [i];
                break;
            case STRFMT_CRES :
                codes = codes_cres [i];
                break;
            case STRFMT_JAVAD:
                codes = codes_javad[i];
                break;
            case STRFMT_BINEX:
                codes = codes_rinex[i];
                break;
            case STRFMT_RINEX:
                codes = codes_rinex[i];
                break;
            case STRFMT_TRIMBLE:
                codes = codes_trimble[i];
                break;//tya
            default:
                codes = codes_other[i];
                break;
        }
        /* set observation types in rinex option */
        setopt_obstype( codes, NULL, i, opt );
    }
}
extern void outheader( FILE **ofp, const rnxopt_t *opt, nav_t *nav )
{
    int i;
    mete_t mete = { {0} };
    
    trace( 3, "closefile:\n" );
    
    for ( i = 0; i < NOUTFILE; i++ )
    {
    
        if ( !ofp[i] )
        {
            continue;
        }
        
        switch ( i )
        {
            case 0:
                outrnxobsh_buf ( ofp[0], opt, nav );  //zy
                break;
            case 1:
                outrnxnavh ( ofp[1], opt, nav );
                break;
            case 2:
                outrnxgnavh( ofp[2], opt, nav );
                break;
            case 3:
                outrnxhnavh( ofp[3], opt, nav );
                break;
            case 4:
                outrnxqnavh( ofp[4], opt, nav );
                break;
            case 5:
                outrnxlnavh( ofp[5], opt, nav );
                break;
            case 6:
                outrnxcnavh( ofp[6], opt, nav );
                break;
            case 7:
                outrnxmeteh_buf( ofp[7], opt, &mete );   //zy
                break;
            default:
                break;
        }
    }
}


extern void closefile( FILE **ofp, const rnxopt_t *opt, nav_t *nav )
{
    int i;
    
    for ( i = 0; i < NOUTFILE; i++ )
    {
    
        if ( !ofp[i] )
        {
            continue;
        }
        
        /* rewrite header to file */
        rewind( ofp[i] );
        switch ( i )
        {
            case 0:
                // outrnxobsh ( ofp[0], opt, nav );
                break;
            case 1:
                outrnxnavh ( ofp[1], opt, nav );
                break;
            case 2:
                outrnxgnavh( ofp[2], opt, nav );
                break;
            case 3:
                outrnxhnavh( ofp[3], opt, nav );
                break;
            case 4:
                outrnxqnavh( ofp[4], opt, nav );
                break;
            case 5:
                outrnxlnavh( ofp[5], opt, nav );
                break;
            case 6:
                outrnxcnavh( ofp[6], opt, nav );
                break;
            default:
                break;
        }
        
        fclose( ofp[i] );
        ofp[i] = NULL;
    }
}

extern void closelinkfile( FILE **ofp )
{
	int i;

	for ( i = 0; i < NOUTFILE; i++ )
	{
		if ( !ofp[i] )
        {
            continue;
        }
        rewind( ofp[i] );
        fclose( ofp[i] );
        ofp[i] = NULL;
	}

}

// checked
extern void convobs_buf( rnx_dat_buf_t *dat_buf, rnxopt_t *opt, const raw_t *raw, int *n  )
{
    gtime_t time;
    
    if ( !dat_buf || raw->obs.n <= 0 )
    {
        fprintf( stderr, "!dat_buf || raw->obs.n <= 0   raw->obs.n= %d\n", raw->obs.n );
        return;
    }
    sortobs( ( obs_t * ) &raw->obs ); //tya
    time = raw->obs.data[0].time;
    
    /* save slips */
    //saveslips(slips,str->obs->data,str->obs->n);/*no use*/
    
    if ( !screent( time, opt->ts, opt->te, opt->tint ) )
    {
    //    fprintf( stderr, "!screent( time, opt->ts, opt->te, opt->tint )" );
        return;
    }
    
    /* restore slips */
    //restslips(slips,str->obs->data,str->obs->n);/*no use*/
    
    /* output rinex obs */
    dat_buf->dat_type = RNX_CONV_DAT_TYPE__OBS;
    outrnxobsb_buf( dat_buf, opt, raw->obs.data, raw->obs.n, 0 );
    
    if ( opt->tstart.time == 0 )
    {
        opt->tstart = time;
    }
    opt->tend = time;
    
    n[0]++;
}

// checked
extern void convnav_buf( rnx_dat_buf_t *dat_buf, rnxopt_t *opt, raw_t *raw, int *n )
{
    gtime_t ts1, te1, ts2, te2;
    int sys, prn;
    
    ts1 = opt->ts;
    if ( ts1.time != 0 )
    {
        ts1 = timeadd( ts1, -MAXDTOE );
    }
    te1 = opt->te;
    if ( te1.time != 0 )
    {
        te1 = timeadd( te1, MAXDTOE );
    }
    ts2 = opt->ts;
    if ( ts2.time != 0 )
    {
        ts2 = timeadd( ts2, -MAXDTOE_GLO );
    }
    te2 = opt->te;
    if ( te2.time != 0 )
    {
        te2 = timeadd( te2, MAXDTOE_GLO );
    }
    
    sys = satsys( raw->ephsat, &prn )&opt->navsys;

    // int y = 0;
    //  for( y = 0; y < 30; y++ )
    //  {
    //  fprintf( stderr, " raw->nav.eph sat[%d] \n", raw->nav.eph[y].sat );
    //  }
    
    if ( sys == SYS_GPS )
    {
        //星历写入不受采样率影响
         if ( opt->exsats[raw->ephsat - 1] == 1 || !screent( raw->time, ts1, te1, 0.0 ) )
         {
             fprintf( stderr, "sys == SYS_GPS judge err!" );
             return;
         }
        
        dat_buf->dat_type = RNX_CONV_DAT_TYPE__SYS;
        /* output rinex nav */
        outrnxnavb_buf( dat_buf, opt, raw->nav.eph + raw->ephsat - 1 );
    }
    else if ( sys == SYS_GLO )
    {
        if ( opt->exsats[raw->ephsat - 1] == 1 || !screent( raw->time, ts2, te2, 0.0 ) )
        {
            return;
        }
        
        if ( opt->rnxver ==3 )
        {
            /* output rinex nav */
            dat_buf->dat_type = RNX_CONV_DAT_TYPE__SYS;
            outrnxgnavb_buf( dat_buf, opt, raw->nav.geph + prn - 1 );
        }
        if ( opt->rnxver ==2 )
        {
            /* output rinex gnav */
            dat_buf->dat_type = RNX_CONV_DAT_TYPE__GLO;
            outrnxgnavb_buf( dat_buf, opt, raw->nav.geph + prn - 1 );
        }
    }
    else if ( sys == SYS_SBS )
    {
        if ( opt->exsats[raw->ephsat - 1] == 1 || !screent( raw->time, ts1, te1, 0.0 ) )
        {
            return;
        }
        
        if ( opt->rnxver ==3 )
        {
            /* output rinex nav */
            dat_buf->dat_type = RNX_CONV_DAT_TYPE__SYS;
            outrnxhnavb_buf( dat_buf, opt, raw->nav.seph + prn - MINPRNSBS );
        }
        if ( opt->rnxver ==2 )
        {
            /* output rinex hnav */
            dat_buf->dat_type = RNX_CONV_DAT_TYPE__SBS;
            outrnxhnavb_buf( dat_buf, opt, raw->nav.seph + prn - MINPRNSBS );
        }
    }
    else if ( sys == SYS_QZS )
    {
    
        if ( opt->exsats[raw->ephsat - 1] == 1 || !screent( raw->time, ts1, te1, 0.0 ) )
        {
            return;
        }
        
        if ( opt->rnxver ==3 )
        {
        
            /* output rinex nav */
            dat_buf->dat_type = RNX_CONV_DAT_TYPE__SYS;
            outrnxnavb_buf( dat_buf, opt, raw->nav.eph + raw->ephsat - 1 );
        }
        if ( opt->rnxver == 2 )
        {
        
            /* output rinex qnav */
            dat_buf->dat_type = RNX_CONV_DAT_TYPE__QZS;
            outrnxnavb_buf( dat_buf, opt, raw->nav.eph + raw->ephsat - 1 );
        }
    }
    else if ( sys == SYS_GAL )
    {
        if ( opt->exsats[raw->ephsat - 1] == 1 || !screent( raw->time, ts1, te1, 0.0 ) )
        {
            return;
        }
        
        if ( opt->rnxver ==3 )
        {
            /* output rinex nav */
            dat_buf->dat_type = RNX_CONV_DAT_TYPE__SYS;
            outrnxnavb_buf( dat_buf, opt, raw->nav.eph + raw->ephsat - 1 );
        }
        if ( opt->rnxver ==2 )
        {
            /* output rinex lnav */
            dat_buf->dat_type = RNX_CONV_DAT_TYPE__GAL;
            outrnxnavb_buf( dat_buf, opt, raw->nav.eph + raw->ephsat - 1 );
        }
    }
    else if ( sys == SYS_CMP )
    {
        if ( opt->exsats[raw->ephsat - 1] == 1 || !screent( raw->time, ts1, te1, 0.0 ) )
        {
            return;
        }
        
        if ( opt->rnxver ==3 )
        {
            /* output rinex nav */
            dat_buf->dat_type = RNX_CONV_DAT_TYPE__SYS;
            outrnxnavb_buf( dat_buf, opt, raw->nav.eph + raw->ephsat - 1 );
        }
        //tya
        if ( opt->rnxver ==2 )
        {
        
            /* output rinex gnav */
            dat_buf->dat_type = RNX_CONV_DAT_TYPE__CMP;
            outrnxnavb_buf( dat_buf, opt, raw->nav.eph + raw->ephsat - 1 );
        }
    }
}

extern void convmete_buf( rnx_dat_buf_t *dat_buf, rnxopt_t *opt, raw_t *raw, int *n )
{
    if ( !dat_buf )
    {
        return;
    }
    
    /* output rinex mete */
    dat_buf->dat_type = RNX_CONV_DAT_TYPE__METE;
    outrnxmeteb_buf( dat_buf, opt, &raw->mete, raw->mete.n, 0 );
    
}
