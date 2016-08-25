/*------------------------------------------------------------------------------
* ephemeris.c : satellite ephemeris and clock functions
*
*          Copyright (C) 2010-2013 by T.TAKASU, All rights reserved.
*
* references :
*     [1] IS-GPS-200D, Navstar GPS Space Segment/Navigation User Interfaces,
*         7 March, 2006
*     [2] Global Navigation Satellite System GLONASS, Interface Control Document
*         Navigational radiosignal In bands L1, L2, (Edition 5.1), 2008
*     [3] RTCA/DO-229C, Minimum operational performanc standards for global
*         positioning system/wide area augmentation system airborne equipment,
*         RTCA inc, November 28, 2001
*     [4] RTCM Paper, April 12, 2010, Proposed SSR Messages for SV Orbit Clock,
*         Code Biases, URA
*     [5] RTCM Paper 012-2009-SC104-528, January 28, 2009 (previous ver of [4])
*     [6] RTCM Paper 012-2009-SC104-582, February 2, 2010 (previous ver of [4])
*     [7] European GNSS (Galileo) Open Service Signal In Space Interface Control
*         Document, Issue 1, February, 2010
*     [8] Quasi-Zenith Satellite System Navigation Service Interface Control
*         Specification for QZSS (IS-QZSS) V1.1, Japan Aerospace Exploration
*         Agency, July 31, 2009
*     [9] BeiDou navigation satellite system signal in space interface control
*         document open service signal B1I (version 1.0), China Satellite
*         Navigation office, December 2012
*     [10] RTCM Standard 10403.1 - Amendment 5, Differential GNSS (Global
*         Navigation Satellite Systems) Services - version 3, July 1, 2011
*
* version : $Revision:$ $Date:$
* history : 2010/07/28 1.1  moved from rtkcmn.c
*                           added api:
*                               eph2clk(),geph2clk(),seph2clk(),satantoff()
*                               satposs()
*                           changed api:
*                               eph2pos(),geph2pos(),satpos()
*                           deleted api:
*                               satposv(),satposiode()
*           2010/08/26 1.2  add ephemeris option EPHOPT_LEX
*           2010/09/09 1.3  fix problem when precise clock outage
*           2011/01/12 1.4  add api alm2pos()
*                           change api satpos(),satposs()
*                           enable valid unhealthy satellites and output status
*                           fix bug on exception by glonass ephem computation
*           2013/01/10 1.5  support beidou (compass)
*                           use newton's method to solve kepler eq.
*                           update ssr correction algorithm
*           2013/03/20 1.6  fix problem on ssr clock relativitic correction
*-----------------------------------------------------------------------------*/
#include "cmn.h"

static const char rcsid[] = "$Id:$";

//================================================================
//*********************************Cresent*********************************
#define CRESSYNC        "$BIN"      /* crescent bin sync code */
#define ID_CRESPOS      1           /* crescent msg id: bin 1 position/velocity */
#define ID_CRESBDSEPH   35          /* hemis msg id: bin 35 beidou ephemeris */
#define ID_CRESBDSRAW   36          /* hemis msg id: bin 36 beidou observation */
#define ID_CRESGLOEPH   65          /* hemis msg id: bin 65 glonass ephemeris */
#define ID_CRESGLORAW   66          /* hemis msg id: bin 66 glonass L1/L2 phase and code */
#define ID_CRESRAW2     76          /* crescent msg id: bin 76 dual-freq raw */
#define ID_CRESWAAS     80          /* crescent msg id: bin 80 waas messages */
#define ID_CRESIONUTC   94          /* crescent msg id: bin 94 ion/utc parameters */
#define ID_CRESEPH      95          /* crescent msg id: bin 95 raw ephemeris */
#define ID_CRESRAW      96          /* crescent msg id: bin 96 raw phase and code */

#define SNR2CN0_L1      30.0        /* crescent snr to c/n0 offset (db) L1 */
#define SNR2CN0_L2      30.0        /* crescent snr to c/n0 offset (db) L2 */
//zy define
#define CRESHLEN        8           /* crescent message header length (bytes) */

//================================================================

/* constants and macros ------------------------------------------------------*/

#define SQR(x)   ((x)*(x))

#define RE_GLO   6378136.0        /* radius of earth (m)            ref [2] */
#define MU_GPS   3.9860050E14     /* gravitational constant         ref [1] */
#define MU_GLO   3.9860044E14     /* gravitational constant         ref [2] */
#define MU_GAL   3.986004418E14   /* earth gravitational constant   ref [7] */
#define MU_CMP   3.986004418E14   /* earth gravitational constant   ref [9] */
#define J2_GLO   1.0826257E-3     /* 2nd zonal harmonic of geopot   ref [2] */

#define OMGE_GLO 7.292115E-5      /* earth angular velocity (rad/s) ref [2] */
#define OMGE_GAL 7.2921151467E-5  /* earth angular velocity (rad/s) ref [7] */
#define OMGE_CMP 7.292115E-5      /* earth angular velocity (rad/s) ref [9] */

#define SIN_5 -0.0871557427476582 /* sin(-5.0 deg) */
#define COS_5  0.9961946980917456 /* cos(-5.0 deg) */

#define ERREPH_GLO 5.0            /* error of glonass ephemeris (m) */
#define TSTEP    60.0             /* integration step glonass ephemeris (s) */
#define RTOL_KEPLER 1E-14         /* relative tolerance for Kepler equation */

#define DEFURASSR 0.15            /* default accurary of ssr corr (m) */
#define MAXECORSSR 10.0           /* max orbit correction of ssr (m) */
#define MAXCCORSSR (1E-6*CLIGHT)  /* max clock correction of ssr (m) */
#define MAXAGESSR 70.0            /* max age of ssr orbit and clock (s) */
#define MAXAGESSR_HRCLK 10.0      /* max age of ssr high-rate clock (s) */
#define STD_BRDCCLK 30.0          /* error of broadcast clock (m) */

#define BD2GPST_WEEK	1356
#define BD2GPST_SEC		14

//------------------Get Types By Pointer----------------------
#define U1(p)		(*((unsigned char *)(p)))
//#define U2(p)		(*((unsigned short *)(p)))
//#define U4(p)		(*((unsigned int *)(p)))
#define I1(p)		(*((char   *)(p)))
#define I2(p)		(*((short  *)(p)))
//#define I4(p)		(*((int    *)(p)))
#define L4(p)		(*((long *)(p)))
static unsigned short U2( unsigned char *p )
{
    unsigned short u;
    memcpy( &u, p, 2 );
    return u;
}

static unsigned int   U4( unsigned char *p )
{
    unsigned int   u;
    memcpy( &u, p, 4 );
    return u;
}

static int            I4( unsigned char *p )
{
    int            i;
    memcpy( &i, p, 4 );
    return i;
}
static double         R8( unsigned char *p )
{
    double         r;
    memcpy( &r, p, 8 );
    return r;
}

const double lam[] =            /* carrier wave length (m) */
{
    CLIGHT / FREQ1, CLIGHT / FREQ2, CLIGHT / FREQ5, CLIGHT / FREQ7, CLIGHT / FREQ6, CLIGHT / FREQ8
};

const double lam_glo[] =
{
    CLIGHT / ( FREQ1_GLO + DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO - 4 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO + 5 * DFRQ1_GLO ),
    CLIGHT / ( FREQ1_GLO + 6 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO + DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO - 4 * DFRQ1_GLO ),
    CLIGHT / ( FREQ1_GLO + 5 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO - 6 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO - 2 * DFRQ1_GLO ),
    CLIGHT / ( FREQ1_GLO - 7 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO ), CLIGHT / ( FREQ1_GLO - DFRQ1_GLO ),
    CLIGHT / ( FREQ1_GLO - 2 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO - 7 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO ),
    CLIGHT / ( FREQ1_GLO - DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO + 4 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO - 3 * DFRQ1_GLO ),
    CLIGHT / ( FREQ1_GLO + 3 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO + 2 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO + 4 * DFRQ1_GLO ),
    CLIGHT / ( FREQ1_GLO - 3 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO + 3 * DFRQ1_GLO ), CLIGHT / ( FREQ1_GLO + 2 * DFRQ1_GLO ),
    
    CLIGHT / ( FREQ2_GLO + DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO - 4 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO + 5 * DFRQ2_GLO ),
    CLIGHT / ( FREQ2_GLO + 6 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO + DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO - 4 * DFRQ2_GLO ),
    CLIGHT / ( FREQ2_GLO + 5 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO - 6 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO - 2 * DFRQ2_GLO ),
    CLIGHT / ( FREQ2_GLO - 7 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO ), CLIGHT / ( FREQ2_GLO - DFRQ2_GLO ),
    CLIGHT / ( FREQ2_GLO - 2 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO - 7 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO ),
    CLIGHT / ( FREQ2_GLO - DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO + 4 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO - 3 * DFRQ2_GLO ),
    CLIGHT / ( FREQ2_GLO + 3 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO + 2 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO + 4 * DFRQ2_GLO ),
    CLIGHT / ( FREQ2_GLO - 3 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO + 3 * DFRQ2_GLO ), CLIGHT / ( FREQ2_GLO + 2 * DFRQ2_GLO )
};


const double lam_bds[] =
{
    CLIGHT / FREQ2_CMP, CLIGHT / FREQ6_CMP, CLIGHT / FREQ7_CMP //obey to tya
};


typedef struct gpstime
{
    int     year;
    int     month;
    int     day;
    int     hour;
    int     min;
    double  second;
    int     week;
    double  sec;
    int     jday;
    gtime_t SecTime;
} GPST;

GPST ghtime = { 0 };   //existing meaning??
gtime_t g_time = { 0 };
static int     g_n = 0;
static int     b3flg = 0;

typedef struct {		/* navigation data type */
	double ion_gps[8];	/* gps iono model parameters {a0,a1,a2,a3,b0,b1,b2,b3} */
	double utc_gps[4];	/* gps delta-utc parameters {A0,A1,T,W} */
	double ion_gal[4];	/* galileo iono model parameters {ai0,ai1,ai2,0} */
	double ion_qzs[8];	/* qzss iono model parameters {a0,a1,a2,a3,b0,b1,b2,b3} */
	double ion_cmp[8];	/* BeiDou iono model parameters {a0,a1,a2,a3,b0,b1,b2,b3} */
	int leaps;			/* leap seconds (s) */
} NAV_HEAD;

NAV_HEAD nav_head;  //existing meaning??
double ion_r[8];
double utc_r[4];
int leap_r;



/* variance by ura ephemeris (ref [1] 20.3.3.3.1.1) --------------------------*/
static double var_uraeph( int ura )
{
    const double ura_value[] =
    {
        2.4, 3.4, 4.85, 6.85, 9.65, 13.65, 24.0, 48.0, 96.0, 192.0, 384.0, 768.0, 1536.0,
        3072.0, 6144.0
    };
    return ura < 0 || 15 < ura ? 6144.0 : SQR( ura_value[ura] );
}

#if 0
/* variance by ura ssr (ref [4]) ---------------------------------------------*/
static double var_urassr( int ura )
{
    double std;
    if ( ura <= 0 )
    {
        return SQR( DEFURASSR );
    }
    if ( ura >= 63 )
    {
        return SQR( 546.65 );
    }
    std = ( pow( 3.0, ( ura >> 3 ) & 7 ) * ( 1.0 + ( ura & 7 ) / 4.0 ) - 1.0 ) * 1E-3;
    return SQR( std );
}
#endif

/* almanac to satellite position and clock bias --------------------------------
* compute satellite position and clock bias with almanac (gps, galileo, qzss)
* args   : gtime_t time     I   time (gpst)
*          alm_t *alm       I   almanac
*          double *rs       O   satellite position (ecef) {x,y,z} (m)
*          double *dts      O   satellite clock bias (s)
* return : none
* notes  : see ref [1],[7],[8]
*-----------------------------------------------------------------------------*/
extern void alm2pos( gtime_t time, const alm_t *alm, double *rs, double *dts )
{
    double tk, M, E, Ek, sinE, cosE, u, r, i, O, x, y, sinO, cosO, cosi, mu;
    
    trace( 4, "alm2pos : time=%s sat=%2d\n", time_str( time, 3 ), alm->sat );
    
    tk = timediff( time, alm->toa );
    
    if ( alm->A <= 0.0 )
    {
        rs[0] = rs[1] = rs[2] = *dts = 0.0;
        return;
    }
    mu = satsys( alm->sat, NULL ) == SYS_GAL ? MU_GAL : MU_GPS;
    
    M = alm->M0 + sqrt( mu / ( alm->A * alm->A * alm->A ) ) * tk;
    for ( E = M, sinE = Ek = 0.0; fabs( E - Ek ) > 1E-12; )
    {
        Ek = E;
        sinE = sin( Ek );
        E = M + alm->e * sinE;
    }
    cosE = cos( E );
    u = atan2( sqrt( 1.0 - alm->e * alm->e ) * sinE, cosE - alm->e ) + alm->omg;
    r = alm->A * ( 1.0 - alm->e * cosE );
    i = alm->i0;
    O = alm->OMG0 + ( alm->OMGd - OMGE ) * tk - OMGE * alm->toas;
    x = r * cos( u );
    y = r * sin( u );
    sinO = sin( O );
    cosO = cos( O );
    cosi = cos( i );
    rs[0] = x * cosO - y * cosi * sinO;
    rs[1] = x * sinO + y * cosi * cosO;
    rs[2] = y * sin( i );
    *dts = alm->f0 + alm->f1 * tk;
}
/* broadcast ephemeris to satellite clock bias ---------------------------------
* compute satellite clock bias with broadcast ephemeris (gps, galileo, qzss)
* args   : gtime_t time     I   time by satellite clock (gpst)
*          eph_t *eph       I   broadcast ephemeris
* return : satellite clock bias (s) without relativeity correction
* notes  : see ref [1],[7],[8]
*          satellite clock does not include relativity correction and tdg
*-----------------------------------------------------------------------------*/
extern double eph2clk( gtime_t time, const eph_t *eph )
{
    double t;
    int i;
    
    trace( 4, "eph2clk : time=%s sat=%2d\n", time_str( time, 3 ), eph->sat );
    
    t = timediff( time, eph->toc );
    
    for ( i = 0; i < 2; i++ )
    {
        t -= eph->f0 + eph->f1 * t + eph->f2 * t * t;
    }
    return eph->f0 + eph->f1 * t + eph->f2 * t * t;
}
/* broadcast ephemeris to satellite position and clock bias --------------------
* compute satellite position and clock bias with broadcast ephemeris (gps,
* galileo, qzss)
* args   : gtime_t time     I   time (gpst)
*          eph_t *eph       I   broadcast ephemeris
*          double *rs       O   satellite position (ecef) {x,y,z} (m)
*          double *dts      O   satellite clock bias (s)
*          double *var      O   satellite position and clock variance (m^2)
* return : none
* notes  : see ref [1],[7],[8]
*          satellite clock includes relativity correction without code bias
*          (tgd or bgd)
*-----------------------------------------------------------------------------*/
extern void eph2pos( gtime_t time, const eph_t *eph, double *rs, double *dts,
                     double *var )
{
    double tk, M, E, Ek, sinE, cosE, u, r, i, O, sin2u, cos2u, x, y, sinO, cosO, cosi, mu, omge;
    double xg, yg, zg, sino, coso;
    int n, sys, prn;
    
    //double secondceshi = time2gpst( eph->toe, NULL );
    
    trace( 4, "eph2pos : time=%s sat=%2d\n", time_str( time, 3 ), eph->sat );
    
    if ( eph->A <= 0.0 )
    {
        rs[0] = rs[1] = rs[2] = *dts = *var = 0.0;
        return;
    }
    tk = timediff( time, eph->toe );
    
    switch ( ( sys = satsys( eph->sat, &prn ) ) )
    {
        case SYS_GAL:
            mu = MU_GAL;
            omge = OMGE_GAL;
            break;
        case SYS_CMP:
            mu = MU_CMP;
            omge = OMGE_CMP;
            break;
        default:
            mu = MU_GPS;
            omge = OMGE;
            break;
    }
    M = eph->M0 + ( sqrt( mu / ( eph->A * eph->A * eph->A ) ) + eph->deln ) * tk;
    
    for ( n = 0, E = M, Ek = 0.0; fabs( E - Ek ) > RTOL_KEPLER; n++ )
    {
        Ek = E;
        E -= ( E - eph->e * sin( E ) - M ) / ( 1.0 - eph->e * cos( E ) );
    }
    sinE = sin( E );
    cosE = cos( E );
    
    trace( 4, "kepler: sat=%2d e=%8.5f n=%2d del=%10.3e\n", eph->sat, eph->e, n, E - Ek );
    
    u = atan2( sqrt( 1.0 - eph->e * eph->e ) * sinE, cosE - eph->e ) + eph->omg;
    r = eph->A * ( 1.0 - eph->e * cosE );
    i = eph->i0 + eph->idot * tk;
    sin2u = sin( 2.0 * u );
    cos2u = cos( 2.0 * u );
    u += eph->cus * sin2u + eph->cuc * cos2u;
    r += eph->crs * sin2u + eph->crc * cos2u;
    i += eph->cis * sin2u + eph->cic * cos2u;
    x = r * cos( u );
    y = r * sin( u );
    cosi = cos( i );
    
    /* beidou geo satellite (ref [9]) */
    if ( sys == SYS_CMP && prn <= 5 )
    {
        O = eph->OMG0 + eph->OMGd * tk - omge * eph->toes;
        sinO = sin( O );
        cosO = cos( O );
        xg = x * cosO - y * cosi * sinO;
        yg = x * sinO + y * cosi * cosO;
        zg = y * sin( i );
        sino = sin( omge * tk );
        coso = cos( omge * tk );
        rs[0] = xg * coso + yg * sino * COS_5 + zg * sino * SIN_5;
        rs[1] = -xg * sino + yg * coso * COS_5 + zg * coso * SIN_5;
        rs[2] = -yg * SIN_5 + zg * COS_5;
    }
    else
    {
        O = eph->OMG0 + ( eph->OMGd - omge ) * tk - omge * eph->toes;
        sinO = sin( O );
        cosO = cos( O );
        rs[0] = x * cosO - y * cosi * sinO;
        rs[1] = x * sinO + y * cosi * cosO;
        rs[2] = y * sin( i );
    }
    tk = timediff( time, eph->toc );
    *dts = eph->f0 + eph->f1 * tk + eph->f2 * tk * tk;
    
    /* relativity correction */
    *dts -= 2.0 * sqrt( mu * eph->A ) * eph->e * sinE / SQR( CLIGHT );
    
    /* position and clock error variance */
    *var = var_uraeph( eph->sva );
}
/* glonass orbit differential equations --------------------------------------*/
static void deq( const double *x, double *xdot, const double *acc )
{
    double a, b, c, r2 = dot( x, x, 3 ), r3 = r2 * sqrt( r2 ), omg2 = SQR( OMGE_GLO );
    
    if ( r2 <= 0.0 )
    {
        xdot[0] = xdot[1] = xdot[2] = xdot[3] = xdot[4] = xdot[5] = 0.0;
        return;
    }
    /* ref [2] A.3.1.2 with bug fix for xdot[4],xdot[5] */
    a = 1.5 * J2_GLO * MU_GLO * SQR( RE_GLO ) / r2 / r3; /* 3/2*J2*mu*Ae^2/r^5 */
    b = 5.0 * x[2] * x[2] / r2;            /* 5*z^2/r^2 */
    c = -MU_GLO / r3 - a * ( 1.0 - b );    /* -mu/r^3-a(1-b) */
    xdot[0] = x[3];
    xdot[1] = x[4];
    xdot[2] = x[5];
    xdot[3] = ( c + omg2 ) * x[0] + 2.0 * OMGE_GLO * x[4] + acc[0];
    xdot[4] = ( c + omg2 ) * x[1] - 2.0 * OMGE_GLO * x[3] + acc[1];
    xdot[5] = ( c - 2.0 * a ) * x[2] + acc[2];
}
/* glonass position and velocity by numerical integration --------------------*/
static void glorbit( double t, double *x, const double *acc )
{
    double k1[6], k2[6], k3[6], k4[6], w[6];
    int i;
    
    deq( x, k1, acc );
    for ( i = 0; i < 6; i++ )
    {
        w[i] = x[i] + k1[i] * t / 2.0;
    }
    deq( w, k2, acc );
    for ( i = 0; i < 6; i++ )
    {
        w[i] = x[i] + k2[i] * t / 2.0;
    }
    deq( w, k3, acc );
    for ( i = 0; i < 6; i++ )
    {
        w[i] = x[i] + k3[i] * t;
    }
    deq( w, k4, acc );
    for ( i = 0; i < 6; i++ )
    {
        x[i] += ( k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i] ) * t / 6.0;
    }
}
/* glonass ephemeris to satellite clock bias -----------------------------------
* compute satellite clock bias with glonass ephemeris
* args   : gtime_t time     I   time by satellite clock (gpst)
*          geph_t *geph     I   glonass ephemeris
* return : satellite clock bias (s)
* notes  : see ref [2]
*-----------------------------------------------------------------------------*/
extern double geph2clk( gtime_t time, const geph_t *geph )
{
    double t;
    int i;
    
    trace( 4, "geph2clk: time=%s sat=%2d\n", time_str( time, 3 ), geph->sat );
    
    t = timediff( time, geph->toe );
    
    for ( i = 0; i < 2; i++ )
    {
        t -= -geph->taun + geph->gamn * t;
    }
    return -geph->taun + geph->gamn * t;
}
/* glonass ephemeris to satellite position and clock bias ----------------------
* compute satellite position and clock bias with glonass ephemeris
* args   : gtime_t time     I   time (gpst)
*          geph_t *geph     I   glonass ephemeris
*          double *rs       O   satellite position {x,y,z} (ecef) (m)
*          double *dts      O   satellite clock bias (s)
*          double *var      O   satellite position and clock variance (m^2)
* return : none
* notes  : see ref [2]
*-----------------------------------------------------------------------------*/
extern void geph2pos( gtime_t time, const geph_t *geph, double *rs, double *dts,
                      double *var )
{
    double t, tt, x[6];
    int i;
    
    trace( 4, "geph2pos: time=%s sat=%2d\n", time_str( time, 3 ), geph->sat );
    
    t = timediff( time, geph->toe );
    
    *dts = -geph->taun + geph->gamn * t;
    
    for ( i = 0; i < 3; i++ )
    {
        x[i  ] = geph->pos[i];
        x[i + 3] = geph->vel[i];
    }
    for ( tt = t < 0.0 ? -TSTEP : TSTEP; fabs( t ) > 1E-9; t -= tt )
    {
        if ( fabs( t ) < TSTEP )
        {
            tt = t;
        }
        glorbit( tt, x, geph->acc );
    }
    for ( i = 0; i < 3; i++ )
    {
        rs[i] = x[i];
    }
    
    *var = SQR( ERREPH_GLO );
}
/* sbas ephemeris to satellite clock bias --------------------------------------
* compute satellite clock bias with sbas ephemeris
* args   : gtime_t time     I   time by satellite clock (gpst)
*          seph_t *seph     I   sbas ephemeris
* return : satellite clock bias (s)
* notes  : see ref [3]
*-----------------------------------------------------------------------------*/
extern double seph2clk( gtime_t time, const seph_t *seph )
{
    double t;
    int i;
    
    trace( 4, "seph2clk: time=%s sat=%2d\n", time_str( time, 3 ), seph->sat );
    
    t = timediff( time, seph->t0 );
    
    for ( i = 0; i < 2; i++ )
    {
        t -= seph->af0 + seph->af1 * t;
    }
    return seph->af0 + seph->af1 * t;
}
/* sbas ephemeris to satellite position and clock bias -------------------------
* compute satellite position and clock bias with sbas ephemeris
* args   : gtime_t time     I   time (gpst)
*          seph_t  *seph    I   sbas ephemeris
*          double  *rs      O   satellite position {x,y,z} (ecef) (m)
*          double  *dts     O   satellite clock bias (s)
*          double  *var     O   satellite position and clock variance (m^2)
* return : none
* notes  : see ref [3]
*-----------------------------------------------------------------------------*/
extern void seph2pos( gtime_t time, const seph_t *seph, double *rs, double *dts,
                      double *var )
{
    double t;
    int i;
    
    trace( 4, "seph2pos: time=%s sat=%2d\n", time_str( time, 3 ), seph->sat );
    
    t = timediff( time, seph->t0 );
    
    for ( i = 0; i < 3; i++ )
    {
        rs[i] = seph->pos[i] + seph->vel[i] * t + seph->acc[i] * t * t / 2.0;
    }
    *dts = seph->af0 + seph->af1 * t;
    
    *var = var_uraeph( seph->sva );
}
/* select ephememeris --------------------------------------------------------*/
static eph_t *seleph( gtime_t time, int sat, int iode, const nav_t *nav )
{
    double t, tmax, tmin;
    int i, j = -1;
    
    trace( 4, "seleph  : time=%s sat=%2d iode=%d\n", time_str( time, 3 ), sat, iode );
    
    tmax = MAXDTOE + 1.0;
    tmin = tmax + 1.0;
    
    for ( i = 0; i < nav->n; i++ )
    {
        if ( nav->eph[i].sat != sat )
        {
            continue;
        }
        if ( iode >= 0 && nav->eph[i].iode != iode )
        {
            continue;
        }
        if ( ( t = fabs( timediff( nav->eph[i].toe, time ) ) ) > tmax )
        {
            continue;
        }
        if ( iode >= 0 )
        {
            return nav->eph + i;
        }
        if ( t <= tmin )
        {
            j = i;    /* toe closest to time */
            tmin = t;
        }
    }
    if ( iode >= 0 || j < 0 )
    {
        trace( 2, "no broadcast ephemeris: %s sat=%2d iode=%3d\n", time_str( time, 0 ),
               sat, iode );
        return NULL;
    }
    return nav->eph + j;
}
/* select glonass ephememeris ------------------------------------------------*/
static geph_t *selgeph( gtime_t time, int sat, int iode, const nav_t *nav )
{
    double t, tmax = MAXDTOE_GLO, tmin = tmax + 1.0;
    int i, j = -1;
    
    trace( 4, "selgeph : time=%s sat=%2d iode=%2d\n", time_str( time, 3 ), sat, iode );
    
    for ( i = 0; i < nav->ng; i++ )
    {
        if ( nav->geph[i].sat != sat )
        {
            continue;
        }
        if ( iode >= 0 && nav->geph[i].iode != iode )
        {
            continue;
        }
        if ( ( t = fabs( timediff( nav->geph[i].toe, time ) ) ) > tmax )
        {
            continue;
        }
        if ( iode >= 0 )
        {
            return nav->geph + i;
        }
        if ( t <= tmin )
        {
            j = i;    /* toe closest to time */
            tmin = t;
        }
    }
    if ( iode >= 0 || j < 0 )
    {
        trace( 3, "no glonass ephemeris  : %s sat=%2d iode=%2d\n", time_str( time, 0 ),
               sat, iode );
        return NULL;
    }
    return nav->geph + j;
}
/* select sbas ephememeris ---------------------------------------------------*/
static seph_t *selseph( gtime_t time, int sat, const nav_t *nav )
{
    double t, tmax = MAXDTOE_SBS, tmin = tmax + 1.0;
    int i, j = -1;
    
    trace( 4, "selseph : time=%s sat=%2d\n", time_str( time, 3 ), sat );
    
    for ( i = 0; i < nav->ns; i++ )
    {
        if ( nav->seph[i].sat != sat )
        {
            continue;
        }
        if ( ( t = fabs( timediff( nav->seph[i].t0, time ) ) ) > tmax )
        {
            continue;
        }
        if ( t <= tmin )
        {
            j = i;    /* toe closest to time */
            tmin = t;
        }
    }
    if ( j < 0 )
    {
        trace( 3, "no sbas ephemeris     : %s sat=%2d\n", time_str( time, 0 ), sat );
        return NULL;
    }
    return nav->seph + j;
}
/* satellite clock with broadcast ephemeris ----------------------------------*/
static int ephclk( gtime_t time, gtime_t teph, int sat, const nav_t *nav,
                   double *dts )
{
    eph_t  *eph;
    geph_t *geph;
    seph_t *seph;
    int sys;
    
    trace( 4, "ephclk  : time=%s sat=%2d\n", time_str( time, 3 ), sat );
    
    sys = satsys( sat, NULL );
    
    if ( sys == SYS_GPS || sys == SYS_GAL || sys == SYS_QZS || sys == SYS_CMP )
    {
        if ( !( eph = seleph( teph, sat, -1, nav ) ) )
        {
            return 0;
        }
        *dts = eph2clk( time, eph );
    }
    else if ( sys == SYS_GLO )
    {
        if ( !( geph = selgeph( teph, sat, -1, nav ) ) )
        {
            return 0;
        }
        *dts = geph2clk( time, geph );
    }
    else if ( sys == SYS_SBS )
    {
        if ( !( seph = selseph( teph, sat, nav ) ) )
        {
            return 0;
        }
        *dts = seph2clk( time, seph );
    }
    else
    {
        return 0;
    }
    
    return 1;
}
/* satellite position and clock by broadcast ephemeris -----------------------*/
static int ephpos( gtime_t time, gtime_t teph, int sat, const nav_t *nav,
                   int iode, double *rs, double *dts, double *var, int *svh )
{
    eph_t  *eph;
    geph_t *geph;
    seph_t *seph;
    double rst[3], dtst[1], tt = 1E-3;
    int i, sys;
    
    trace( 4, "ephpos  : time=%s sat=%2d iode=%d\n", time_str( time, 3 ), sat, iode );
    
    sys = satsys( sat, NULL );
    
    *svh = -1;
    
    if ( sys == SYS_GPS || sys == SYS_GAL || sys == SYS_QZS || sys == SYS_CMP )
    {
        if ( !( eph = seleph( teph, sat, iode, nav ) ) )
        {
            return 0;
        }
        
        eph2pos( time, eph, rs, dts, var );
        time = timeadd( time, tt );
        eph2pos( time, eph, rst, dtst, var );
        *svh = eph->svh;
    }
    else if ( sys == SYS_GLO )
    {
        if ( !( geph = selgeph( teph, sat, iode, nav ) ) )
        {
            return 0;
        }
        geph2pos( time, geph, rs, dts, var );
        time = timeadd( time, tt );
        geph2pos( time, geph, rst, dtst, var );
        *svh = geph->svh;
    }
    else if ( sys == SYS_SBS )
    {
        if ( !( seph = selseph( teph, sat, nav ) ) )
        {
            return 0;
        }
        
        seph2pos( time, seph, rs, dts, var );
        time = timeadd( time, tt );
        seph2pos( time, seph, rst, dtst, var );
        *svh = seph->svh;
    }
    else
    {
        return 0;
    }
    
    /* satellite velocity and clock drift by differential approx */
    for ( i = 0; i < 3; i++ )
    {
        rs[i + 3] = ( rst[i] - rs[i] ) / tt;
    }
    dts[1] = ( dtst[0] - dts[0] ) / tt;
    
    return 1;
}

#if 0
/* satellite position and clock with sbas correction -------------------------*/
static int satpos_sbas( gtime_t time, gtime_t teph, int sat, const nav_t *nav,
                        double *rs, double *dts, double *var, int *svh )
{
    const sbssatp_t *sbs;
    int i;
    
    trace( 4, "satpos_sbas: time=%s sat=%2d\n", time_str( time, 3 ), sat );
    
    /* search sbas satellite correciton */
    for ( i = 0; i < nav->sbssat.nsat; i++ )
    {
        sbs = nav->sbssat.sat + i;
        if ( sbs->sat == sat )
        {
            break;
        }
    }
    if ( i >= nav->sbssat.nsat )
    {
        trace( 2, "no sbas correction for orbit: %s sat=%2d\n", time_str( time, 0 ), sat );
        ephpos( time, teph, sat, nav, -1, rs, dts, var, svh );
        *svh = -1;
        return 0;
    }
    /* satellite postion and clock by broadcast ephemeris */
    if ( !ephpos( time, teph, sat, nav, sbs->lcorr.iode, rs, dts, var, svh ) )
    {
        return 0;
    }
    
    /* sbas satellite correction (long term and fast) */
    if ( sbssatcorr( time, sat, nav, rs, dts, var ) )
    {
        return 1;
    }
    *svh = -1;
    return 0;
}
/* satellite position and clock with ssr correction --------------------------*/
static int satpos_ssr( gtime_t time, gtime_t teph, int sat, const nav_t *nav,
                       int opt, double *rs, double *dts, double *var, int *svh )
{
    const ssr_t *ssr;
    eph_t *eph;
    double t1, t2, t3, er[3], ea[3], ec[3], rc[3], deph[3], dclk, dant[3] = {0}, tk;
    int i, sys;
    
    trace( 4, "satpos_ssr: time=%s sat=%2d\n", time_str( time, 3 ), sat );
    
    ssr = nav->ssr + sat - 1;
    
    if ( !ssr->t0[0].time )
    {
        trace( 2, "no ssr orbit correction: %s sat=%2d\n", time_str( time, 0 ), sat );
        return 0;
    }
    if ( !ssr->t0[1].time )
    {
        trace( 2, "no ssr clock correction: %s sat=%2d\n", time_str( time, 0 ), sat );
        return 0;
    }
    /* inconsistency between orbit and clock correction */
    if ( ssr->iod[0] != ssr->iod[1] )
    {
        trace( 2, "inconsist ssr correction: %s sat=%2d iod=%d %d\n",
               time_str( time, 0 ), sat, ssr->iod[0], ssr->iod[1] );
        *svh = -1;
        return 0;
    }
    t1 = timediff( time, ssr->t0[0] );
    t2 = timediff( time, ssr->t0[1] );
    t3 = timediff( time, ssr->t0[2] );
    
    /* ssr orbit and clock correction (ref [4]) */
    if ( fabs( t1 ) > MAXAGESSR || fabs( t2 ) > MAXAGESSR )
    {
        trace( 2, "age of ssr error: %s sat=%2d t=%.0f %.0f\n", time_str( time, 0 ),
               sat, t1, t2 );
        *svh = -1;
        return 0;
    }
    if ( ssr->udi[0] >= 1.0 )
    {
        t1 -= ssr->udi[0] / 2.0;
    }
    if ( ssr->udi[1] >= 1.0 )
    {
        t2 -= ssr->udi[0] / 2.0;
    }
    
    for ( i = 0; i < 3; i++ )
    {
        deph[i] = ssr->deph[i] + ssr->ddeph[i] * t1;
    }
    dclk = ssr->dclk[0] + ssr->dclk[1] * t2 + ssr->dclk[2] * t2 * t2;
    
    /* ssr highrate clock correction (ref [4]) */
    if ( ssr->iod[0] == ssr->iod[2] && ssr->t0[2].time && fabs( t3 ) < MAXAGESSR_HRCLK )
    {
        dclk += ssr->hrclk;
    }
    if ( norm( deph, 3 ) > MAXECORSSR || fabs( dclk ) > MAXCCORSSR )
    {
        trace( 3, "invalid ssr correction: %s deph=%.1f dclk=%.1f\n",
               time_str( time, 0 ), norm( deph, 3 ), dclk );
        *svh = -1;
        return 0;
    }
    /* satellite postion and clock by broadcast ephemeris */
    if ( !ephpos( time, teph, sat, nav, ssr->iode, rs, dts, var, svh ) )
    {
        return 0;
    }
    
    /* satellite clock for gps, galileo and qzss */
    sys = satsys( sat, NULL );
    if ( sys == SYS_GPS || sys == SYS_GAL || sys == SYS_QZS || sys == SYS_CMP )
    {
        if ( !( eph = seleph( teph, sat, ssr->iode, nav ) ) )
        {
            return 0;
        }
        
        /* satellite clock by clock parameters */
        tk = timediff( time, eph->toc );
        dts[0] = eph->f0 + eph->f1 * tk + eph->f2 * tk * tk;
        dts[1] = eph->f1 + 2.0 * eph->f2 * tk;
        
        /* relativity correction */
        dts[0] -= 2.0 * dot( rs, rs + 3, 3 ) / CLIGHT / CLIGHT;
    }
    /* radial-along-cross directions in ecef */
    if ( !normv3( rs + 3, ea ) )
    {
        return 0;
    }
    cross3( rs, rs + 3, rc );
    if ( !normv3( rc, ec ) )
    {
        *svh = -1;
        return 0;
    }
    cross3( ea, ec, er );
    
    /* satellite antenna offset correction */
    if ( opt )
    {
        satantoff( time, rs, nav->pcvs + sat - 1, dant );
    }
    for ( i = 0; i < 3; i++ )
    {
        rs[i] += -( er[i] * deph[0] + ea[i] * deph[1] + ec[i] * deph[2] ) + dant[i];
    }
    /* t_corr = t_sv - (dts(brdc) + dclk(ssr) / CLIGHT) (ref [10] eq.3.12-7) */
    dts[0] += dclk / CLIGHT;
    
    /* variance by ssr ura */
    *var = var_urassr( ssr->ura );
    
    trace( 5, "satpos_ssr: %s sat=%2d deph=%6.3f %6.3f %6.3f er=%6.3f %6.3f %6.3f dclk=%6.3f var=%6.3f\n",
           time_str( time, 2 ), sat, deph[0], deph[1], deph[2], er[0], er[1], er[2], dclk, *var );
           
    return 1;
}

#endif

/* satellite position and clock ------------------------------------------------
* compute satellite position, velocity and clock
* args   : gtime_t time     I   time (gpst)
*          gtime_t teph     I   time to select ephemeris (gpst)
*          int    sat       I   satellite number
*          nav_t  *nav      I   navigation data
*          int    ephopt    I   ephemeris option (EPHOPT_???)
*          double *rs       O   sat position and velocity (ecef)
*                               {x,y,z,vx,vy,vz} (m|m/s)
*          double *dts      O   sat clock {bias,drift} (s|s/s)
*          double *var      O   sat position and clock error variance (m^2)
*          int    *svh      O   sat health flag (-1:correction not available)
* return : status (1:ok,0:error)
* notes  : satellite position is referenced to antenna phase center
*          satellite clock does not include code bias correction (tgd or bgd)
*-----------------------------------------------------------------------------*/
extern int satpos( gtime_t time, gtime_t teph, int sat, int ephopt,
                   const nav_t *nav, double *rs, double *dts, double *var,
                   int *svh )
{
    trace( 4, "satpos  : time=%s sat=%2d ephopt=%d\n", time_str( time, 3 ), sat, ephopt );
    
    *svh = 0;
    
    switch ( ephopt )
    {
        case EPHOPT_BRDC  :
            return ephpos     ( time, teph, sat, nav, -1, rs, dts, var, svh );
        default:
            break;
    }
    *svh = -1;
    return 0;
}
/* satellite positions and clocks ----------------------------------------------
* compute satellite positions, velocities and clocks
* args   : gtime_t teph     I   time to select ephemeris (gpst)
*          obsd_t *obs      I   observation data
*          int    n         I   number of observation data
*          nav_t  *nav      I   navigation data
*          int    ephopt    I   ephemeris option (EPHOPT_???)
*          double *rs       O   satellite positions and velocities (ecef)
*          double *dts      O   satellite clocks
*          double *var      O   sat position and clock error variances (m^2)
*          int    *svh      O   sat health flag (-1:correction not available)
* return : none
* notes  : rs [(0:2)+i*6]= obs[i] sat position {x,y,z} (m)
*          rs [(3:5)+i*6]= obs[i] sat velocity {vx,vy,vz} (m/s)
*          dts[(0:1)+i*2]= obs[i] sat clock {bias,drift} (s|s/s)
*          var[i]        = obs[i] sat position and clock error variance (m^2)
*          svh[i]        = obs[i] sat health flag
*          if no navigation data, set 0 to rs[], dts[], var[] and svh[]
*          satellite position and clock are values at signal transmission time
*          satellite position is referenced to antenna phase center
*          satellite clock does not include code bias correction (tgd or bgd)
*          any pseudorange and broadcast ephemeris are always needed to get
*          signal transmission time
*-----------------------------------------------------------------------------*/
extern void satposs( gtime_t teph, const obsd_t *obs, int n, const nav_t *nav,
                     int ephopt, double *rs, double *dts, double *var, int *svh )
{
    gtime_t time[MAXOBS] = {{0}}; /*MAXOBS:max sat num at one epoch*/
    double dt, pr;
    int i, j;
    
    trace( 3, "satposs : teph=%s n=%d ephopt=%d\n", time_str( teph, 3 ), n, ephopt );
    
    for ( i = 0; i < n && i < MAXOBS; i++ )
    {
        for ( j = 0; j < 6; j++ )
        {
            rs [j + i * 6] = 0.0;
        }
        for ( j = 0; j < 2; j++ )
        {
            dts[j + i * 2] = 0.0;
        }
        var[i] = 0.0;
        svh[i] = 0;
        
        /* search any psuedorange */
        for ( j = 0, pr = 0.0; j < NFREQ; j++ ) if ( ( pr = obs[i].P[j] ) > 0.0 )
            {
                break;
            }
            
        if ( j >= NFREQ )
        {
            trace( 2, "no pseudorange %s sat=%2d\n", time_str( obs[i].time, 3 ), obs[i].sat );
            continue;
        }
        /* transmission time by satellite clock */
        time[i] = timeadd( obs[i].time, -pr / CLIGHT );
        
        /* satellite clock bias by broadcast ephemeris */
        if ( !ephclk( time[i], teph, obs[i].sat, nav, &dt ) )
        {
            trace( 2, "no broadcast clock %s sat=%2d\n", time_str( time[i], 3 ), obs[i].sat );
            continue;
        }
        time[i] = timeadd( time[i], -dt );
        
        /* satellite position and clock at transmission time */
        if ( !satpos( time[i], teph, obs[i].sat, ephopt, nav, rs + i * 6, dts + i * 2, var + i,
                      svh + i ) )
        {
            trace( 2, "no ephemeris %s sat=%2d\n", time_str( time[i], 3 ), obs[i].sat );
            continue;
        }
        /* if no precise clock unavailable, use broadcast clock instead */
        if ( dts[i * 2] == 0.0 )
        {
            if ( !ephclk( time[i], teph, obs[i].sat, nav, dts + i * 2 ) )
            {
                continue;
            }
            dts[1 + i * 2] = 0.0;
            *var = SQR( STD_BRDCCLK );
        }
    }
    for ( i = 0; i < n && i < MAXOBS; i++ )
    {
        trace( 4, "%s sat=%2d rs=%13.3f %13.3f %13.3f dts=%12.3f var=%7.3f svh=%02X\n",
               time_str( time[i], 6 ), obs[i].sat, rs[i * 6], rs[1 + i * 6], rs[2 + i * 6],
               dts[i * 2] * 1E9, var[i], svh[i] );
    }
}

/* decode glonass ephemeris strings --------------------------------------------
* decode glonass ephemeris string (ref [2])
* args   : unsigned char *buff I glonass navigation data string bits in frames
*                                (without hamming and time mark)
*                                  buff[ 0- 9]: string #1 (77 bits)
*                                  buff[10-19]: string #2
*                                  buff[20-29]: string #3
*                                  buff[30-39]: string #4
*          geph_t *geph  IO     glonass ephemeris message
* return : status (1:ok,0:error)
* notes  : geph->tof should be set to frame time witin 1/2 day before calling
*          geph->frq is set to 0
*-----------------------------------------------------------------------------*/
int decode_glostr(const unsigned char *buff, geph_t *geph)
{
	double tow,tod,tof,toe;
	int P,P1,P2,P3,P4,tk_h,tk_m,tk_s,tb,ln,NT,slot,M,week;
	int i=1,frn1,frn2,frn3,frn4;

	//trace(3,"decode_glostr:\n");

	/* frame 1 */
	frn1        =getbitu(buff,i, 4);           i+= 4+2;
	P1          =getbitu(buff,i, 2);           i+= 2;
	tk_h        =getbitu(buff,i, 5);           i+= 5;
	tk_m        =getbitu(buff,i, 6);           i+= 6;
	tk_s        =getbitu(buff,i, 1)*30;        i+= 1;
	geph->vel[0]=getbitg(buff,i,24)*P2_20*1E3; i+=24;
	geph->acc[0]=getbitg(buff,i, 5)*P2_30*1E3; i+= 5;
	geph->pos[0]=getbitg(buff,i,27)*P2_11*1E3; i+=27+4;

	/* frame 2 */
	frn2        =getbitu(buff,i, 4);           i+= 4;
	geph->svh   =getbitu(buff,i, 3);           i+= 3;
	P2          =getbitu(buff,i, 1);           i+= 1;
	tb          =getbitu(buff,i, 7);           i+= 7+5;
	geph->vel[1]=getbitg(buff,i,24)*P2_20*1E3; i+=24;
	geph->acc[1]=getbitg(buff,i, 5)*P2_30*1E3; i+= 5;
	geph->pos[1]=getbitg(buff,i,27)*P2_11*1E3; i+=27+4;

	/* frame 3 */
	frn3        =getbitu(buff,i, 4);           i+= 4;
	P3          =getbitu(buff,i, 1);           i+= 1;
	geph->gamn  =getbitg(buff,i,11)*P2_40;     i+=11+1;
	P           =getbitu(buff,i, 2);           i+= 2;
	ln          =getbitu(buff,i, 1);           i+= 1;
	geph->vel[2]=getbitg(buff,i,24)*P2_20*1E3; i+=24;
	geph->acc[2]=getbitg(buff,i, 5)*P2_30*1E3; i+= 5;
	geph->pos[2]=getbitg(buff,i,27)*P2_11*1E3; i+=27+4;

	/* frame 4 */
	frn4        =getbitu(buff,i, 4);           i+= 4;
	geph->taun  =getbitg(buff,i,22)*P2_30;     i+=22;
	geph->dtaun =getbitg(buff,i, 5)*P2_30;     i+= 5;
	//double dtaun =getbitg(buff,i, 5)*P2_30;  i+= 5;
	geph->age   =getbitu(buff,i, 5);           i+= 5+14;
	P4          =getbitu(buff,i, 1);           i+= 1;
	geph->sva   =getbitu(buff,i, 4);           i+= 4+3;
	//int sva   =getbitu(buff,i, 4);           i+= 4+3;
	NT          =getbitu(buff,i,11);           i+=11;
	slot        =getbitu(buff,i, 5);           i+= 5;
	M           =getbitu(buff,i, 2);

	if (frn1!=1||frn2!=2||frn3!=3||frn4!=4) {
		//trace(3,"decode_glostr error: frn=%d %d %d %d %d\n",frn1,frn2,frn3,frn4);
		return 0;
	}
	if (!(geph->sat=satno(SYS_GLO,slot))) {
		//trace(2,"decode_glostr error: slot=%d\n",slot);
		return 0;
	}
	geph->frq=0;
	geph->iode=tb;
	tow=time2gpst(gpst2utc(geph->tof),&week);
	tod=fmod(tow,86400.0); tow-=tod;
	tof=tk_h*3600.0+tk_m*60.0+tk_s-10800.0; /* lt->utc */
	if      (tof<tod-43200.0) tof+=86400.0;
	else if (tof>tod+43200.0) tof-=86400.0;
	geph->tof=utc2gpst(gpst2time(week,tow+tof));
	toe=tb*900.0-10800.0; /* lt->utc */
	if      (toe<tod-43200.0) toe+=86400.0;
	else if (toe>tod+43200.0) toe-=86400.0;
	geph->toe=utc2gpst(gpst2time(week,tow+toe)); /* utc->gpst */
	return 1;
}

/*--------------------------------------*/
/*
void decode_crespos( unsigned char *raw )
{
}
*/

static int decode_cresraw( raw_t *raw )
{
    gtime_t         time;
    double          tows, toff, cp, pr, dop, snr;
    int             i, j, n = 0, prn, sat, word2, lli = 0;
    unsigned int    word1, sn, sc;
    //unsigned char   *p = raw + 8;
    unsigned char *p = raw->buff + 8;
    double          ep[6] = {0};
    //Obs_Data      obs_d = { 0 };//[MAXSAT];
    //obs_t           obs_d = { 0 };//[MAXSAT];
    //GPST ghtime = { 0 };
    //double lam[MAXSAT][NFREQ] = { {0} }; /* carrier wave lengths (m) */
    
    
    ghtime.week = U2( p + 2 );
    ghtime.sec = R8( p + 4 );
    ghtime.sec = *( ( double * )( p + 4 ) );
    tows = floor( ghtime.sec * 1000.0 + 0.5 ) / 1000.0;
    toff = CLIGHT * ( tows - ghtime.sec );
    time = gpst2time( ghtime.week, ghtime.sec );
    time2epoch( time, ep );
    ghtime.SecTime = time;
    ghtime.year = ( int )ep[0];
    ghtime.month = ( int )ep[1];
    ghtime.day = ( int )ep[2];
    ghtime.hour = ( int )ep[3];
    ghtime.min = ( int )ep[4];
    ghtime.second = ep[5];
    
    for ( i = n = 0, p += 12; i < 12 && n < MAXOBS; i++, p += 24 )
    {
        word1 = U4( p  );
        word2 = I4( p + 4 );
        if ( ( prn = word1 & 0xFF ) == 0 )
        {
            continue;
        }
        if ( !( sat = satno( prn <= MAXPRNGPS ? SYS_GPS : SYS_SBS, prn ) ) )
        {
            continue;
        }
        pr = R8( p + 8 ) - toff;
        cp = R8( p + 16 ) - toff;
        if ( !( word2 & 1 ) )
        {
            cp = 0.0;
        }
        sn = ( word1 >> 8 ) & 0xFF;
        
        snr = sn == 0 ? 0.0 : 10.0 * log10( 0.8192 * sn ) + SNR2CN0_L1;
        sc = ( unsigned int )( word1 >> 24 ); //counter
        
        dop = word2 / 16 / 4096.0;
        
        //raw->obs.data.time = ghtime;
        raw->obs.data[n].time = time;
        raw->obs.data[n].sat = sat;
        raw->obs.data[n].P[0] = pr;
        raw->obs.data[n].L[0] = -cp / lam[0];
        raw->obs.data[n].D[0] = -( float )( dop / lam[0] );
        raw->obs.data[n].SNR[0] = ( unsigned char )( snr + 0.5 );
        raw->obs.data[n].LLI[0] = ( unsigned char )lli;
        raw->obs.data[n].code[0] = CODE_L1C;
        
        for ( j = 1; j < NFREQ; j++ )
        {
            raw->obs.data[n].L[j] = raw->obs.data[n].P[j] = 0.0;
            raw->obs.data[n].D[j] = 0.0;
            raw->obs.data[n].SNR[j] = raw->obs.data[n].LLI[j] = 0;
            raw->obs.data[n].code[j] = CODE_NONE;
        }
        n++;
    }
    
    //raw->obs.n = n;
    fprintf( stderr, "gps1 %02d %2.0f %2.0f %2.0f %2.0f%11.7f  \n",  ( int )ep[0] % 100, ep[1], ep[2], ep[3], ep[4], ep[5] );
    if( time.time != g_time.time )
    {
		fprintf( stderr," %d, %d \n", (int)time.time, (int)g_time.time);
		g_time = time;
		g_n = n;
    }
    else //equal
    {
		g_n += n;
    }

#if 0   //排序处理等，暂时先不用管  
    if ( obs_d.n > 0 )
    {
        //pObsData.Add(obs_d);
        //obsEpochNum++;
        //if(timediff(obs_d.time.SecTime, gt1) <= 0)
        //  return;
        qsort( obs_d.data, obs_d.n, sizeof( OBSDATA ), cmpobs );
        fwrite( ( char * )( &obs_d ), sizeof( Obs_Data ), 1, fobsTmp );
        //      pObsData.Add(obs_d);
        obsEpochNum++;
        gt1 = obs_d.time;
    }
#endif
    return 1;
    
}

/*
void decode_cresraw(unsigned char *raw)
{
    gtime_t         time;
    double          tows,toff,cp,pr,dop,snr;
    int             i,j,n,prn,sat,word2,lli=0;
    unsigned int    word1,sn,sc;
    unsigned char   *p=raw+8;
    double          ep[6] = {0};
    Obs_Data        obs_d = { 0 };//[MAXSAT];

    ghtime.week=U2(p+2);
    ghtime.sec = R8(p+4);
    ghtime.sec = *((double*)(p+4));
    tows=floor(ghtime.sec*1000.0+0.5)/1000.0;
    toff=CLIGHT*(tows-ghtime.sec);
    time = gpst2time(ghtime.week,ghtime.sec);
    time2epoch(time,ep);
    ghtime.SecTime = time;
    ghtime.year = (int)ep[0];
    ghtime.month = (int)ep[1];
    ghtime.day = (int)ep[2];
    ghtime.hour = (int)ep[3];
    ghtime.min = (int)ep[4];
    ghtime.second = ep[5];
    for (i=n=0,p+=12;i<12&&n<MAXOBS;i++,p+=24)
    {
        word1=U4(p  );
        word2=I4(p+4);
        if ((prn=word1&0xFF)==0) continue;
        if (!(sat=satno(prn<=MAXPRNGPS?SYS_GPS:SYS_SBS,prn)))
        {
            continue;
        }
        pr=R8(p+ 8)-toff;
        cp=R8(p+16)-toff;
        if (!(word2&1))
            cp=0.0;
        sn =(word1>>8)&0xFF;

        snr=sn==0?0.0:10.0*log10(0.8192*sn)+SNR2CN0_L1;
        sc =(unsigned int)(word1>>24);//counter

        dop=word2/16/4096.0;

        obs_d.time=ghtime;
        obs_d.data[n].sat = sat;
        obs_d.data[n].P[0]= pr;
        obs_d.data[n].L[0]= -cp/lam[0];
        obs_d.data[n].D[0]= -(float)(dop/lam[0]);
        obs_d.data[n].SNR[0]=(unsigned char)(snr+0.5);
        obs_d.data[n].LLI[0]=(unsigned char)lli;
        obs_d.data[n].code[0]=CODE_L1C;

        for (j=1;j<NFREQ;j++)
        {
            obs_d.data[n].L[j]=obs_d.data[n].P[j]=0.0;
            obs_d.data[n].D[j]=0.0;
            obs_d.data[n].SNR[j]=obs_d.data[n].LLI[j]=0;
            obs_d.data[n].code[j]=CODE_NONE;
        }
        n++;
    }
    obs_d.n = n;

#if 0   //排序处理等，暂时先不用管
    if(obs_d.n > 0)
    {
        //pObsData.Add(obs_d);
        //obsEpochNum++;
        //if(timediff(obs_d.time.SecTime, gt1) <= 0)
        //  return;
        qsort(obs_d.data,obs_d.n,sizeof(OBSDATA),cmpobs);
        fwrite((char*)(&obs_d), sizeof(Obs_Data), 1, fobsTmp);
        //      pObsData.Add(obs_d);
        obsEpochNum++;
        gt1 = obs_d.time;
    }
#endif
}
*/

static int decode_cresraw2( raw_t *raw/*, int satsys*/ )
{
    gtime_t time;
    double tow, tows, toff = 0.0, cp[2] = {0}, pr1, pr[2] = {0}, dop[2] = {0}, snr[2] = {0};
    int i, j, n = 0, prn, sat, week, lli[2] = {0};
    unsigned int word1, word2, word3, sc, sn, validphase;
    //unsigned char *p = raw + 8;
    unsigned char *p = raw->buff + 8;
    
    //unsigned short len = U2( raw + 6 );
    unsigned short len = U2( raw->buff + 6 );
    //Obs_Data        obs_d = { 0 };//[MAXSAT];
    double ep[6] = {0.0};
    //trace(4,"decode_cresraw2: len=%d\n",raw->len);
    
    
    if ( len != 460 - 12 )
    {
        //trace(2,"crescent bin 76 message length error: len=%d\n",raw->len);
        //return -1;
    }
    tow = R8( p );
    week = U2( p + 8 );
    tows = floor( tow * 1000.0 + 0.5 ) / 1000.0; /* round by 1ms */
    time = gpst2time( week, tows );
    ghtime.week = week;
    ghtime.sec = tow;
    ghtime.SecTime = gpst2time( ghtime.week, ghtime.sec );
    time2epoch( ghtime.SecTime, ep );
    ghtime.SecTime = time;
    ghtime.year = ( int )ep[0];
    ghtime.month = ( int )ep[1];
    ghtime.day = ( int )ep[2];
    ghtime.hour = ( int )ep[3];
    ghtime.min = ( int )ep[4];
    ghtime.second = ep[5];
        
    //obs_d.time = ghtime;
    raw->obs.data[n].time = time;
    // fprintf( stderr, " time[%d][%f] \n", ( int )time.time, time.sec );
    
    /* time tag offset correction */
    //if (strstr(raw->opt,"-TTCORR"))
    {
        //toff=CLIGHT*(tows-tow);
    }
    //if (fabs(timediff(time,raw->time))<1e-9) {
    //  n=obs_d.n;
    //}
    for ( i = 0, p += 16; i < 15 && n < MAXOBS; i++ )
    {
        word1 = U4( p + 324 + 4 * i ); /* L1CACodeMSBsPRN */
        if ( ( prn = word1 & 0xFF ) == 0 )
        {
            continue;    /* if 0, no data */
        }
        if ( !( sat = satno( /*prn<=MAXPRNGPS?*/SYS_GPS/*:SYS_SBS*/, prn ) ) )
        {
            //trace(2,"creasent bin 76 satellite number error: prn=%d\n",prn);
            continue;
        }
        pr1 = ( word1 >> 13 ) * 256.0; /* upper 19bit of L1CA pseudorange */
        
        word1 = U4( p + 144 + 12 * i ); /* L1CASatObs */
        word2 = U4( p + 148 + 12 * i );
        word3 = U4( p + 152 + 12 * i );
        sn = word1 & 0xFFF;
        snr[0] = sn == 0 ? 0.0 : 10.0 * log10( 0.1024 * sn ) + SNR2CN0_L1;
        sc = ( unsigned int )( word1 >> 24 );
        //if (raw->time.time!=0) {
        //  lli[0]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][0])>0;
        //}
        //else {
        lli[0] = 0;
        //}
        lli[0] |= ( ( word1 >> 12 ) & 7 ) ? 2 : 0;
        //raw->lockt[sat-1][0]=(unsigned char)sc;
        validphase = word2 & 1;
        if ( !validphase )
        {
            continue;
        }
        dop[0] = ( ( word2 >> 1 ) & 0x7FFFFF ) / 512.0;
        if ( ( word2 >> 24 ) & 1 )
        {
            dop[0] = -dop[0];
        }
        pr[0] = pr1 + ( word3 & 0xFFFF ) / 256.0;
        cp[0] = floor( pr[0] / lam[0] / 8192.0 ) * 8192.0;
        cp[0] += ( ( word2 & 0xFE000000 ) + ( ( word3 & 0xFFFF0000 ) >> 7 ) ) / 524288.0;
        if      ( cp[0] - pr[0] / lam[0] < -4096.0 )
        {
            cp[0] += 8192.0;
        }
        else if ( cp[0] - pr[0] / lam[0] > 4096.0 )
        {
            cp[0] -= 8192.0;
        }
        
        if ( i < 12 )
        {
            word1 = U4( p  + 12 * i ); /* L2PSatObs */
            word2 = U4( p + 4 + 12 * i );
            word3 = U4( p + 8 + 12 * i );
            sn = word1 & 0xFFF;
            snr[1] = sn == 0 ? 0.0 : 10.0 * log10( 0.1164 * sn ) + SNR2CN0_L2;
            sc = ( unsigned int )( word1 >> 24 );
            //if (raw->time.time==0) {
            //  lli[1]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][1])>0;
            //}
            //else {
            lli[1] = 0;
            //}
            lli[1] |= ( ( word1 >> 12 ) & 7 ) ? 2 : 0;
            //raw->lockt[sat-1][1]=(unsigned char)sc;
            if ( ( word2 & 1 ) == 0 && !validphase )
            {
                //continue;
            }
            dop[1] = ( ( word2 >> 1 ) & 0x7FFFFF ) / 512.0;
            if ( ( word2 >> 24 ) & 1 )
            {
                dop[1] = -dop[1];
            }
            pr[1] = ( word3 & 0xFFFF ) / 256.0;
            if ( pr[1] != 0.0 )
            {
                pr[1] += pr1;
                if      ( pr[1] - pr[0] < -128.0 )
                {
                    pr[1] += 256.0;
                }
                else if ( pr[1] - pr[0] > 128.0 )
                {
                    pr[1] -= 256.0;
                }
                cp[1] = floor( pr[1] / lam[1] / 8192.0 ) * 8192.0;
                cp[1] += ( ( word2 & 0xFE000000 ) + ( ( word3 & 0xFFFF0000 ) >> 7 ) ) / 524288.0;
                if      ( cp[1] - pr[1] / lam[1] < -4096.0 )
                {
                    cp[1] += 8192.0;
                }
                else if ( cp[1] - pr[1] / lam[1] > 4096.0 )
                {
                    cp[1] -= 8192.0;
                }
            }
            else
            {
                cp[1] = 0.0;
            }
        }
        
        raw->obs.data[n].time = time;
        raw->obs.data[n].sat  = sat;
        //raw->obs.data[n].sys = SYS_GPS;
        for ( j = 0; j < NFREQ; j++ )
        {
            if ( j == 0 || ( j == 1 && i < 12 ) )
            {
                raw->obs.data[n].P[j] = pr[j] == 0.0 ? 0.0 : pr[j] - toff;
                raw->obs.data[n].L[j] = -( cp[j] == 0.0 ? 0.0 : cp[j] - toff / lam[j] );
                raw->obs.data[n].D[j] = -( float )dop[j];
                raw->obs.data[n].SNR[j] = ( unsigned char )( snr[j]/**4.0*/ + 0.5 );
                raw->obs.data[n].LLI[j] = ( unsigned char )lli[j];
                raw->obs.data[n].code[j] = j == 0 ? CODE_L1C : CODE_L2P;
            }
            else
            {
                raw->obs.data[n].L[j] = raw->obs.data[n].P[j] = 0.0;
                raw->obs.data[n].D[j] = 0.0;
                raw->obs.data[n].SNR[j] = raw->obs.data[n].LLI[j] = 0;
                raw->obs.data[n].code[j] = CODE_NONE;
            }
        }
        n++;
    }
    //raw->time=time;    
    //raw->obs.n = n;
    
    if( time.time != g_time.time )
    {
		//fprintf( stderr," %d, %d \n", (int)time.time, (int)g_time.time);
		g_time = time;
		g_n = n;
    }
    else //equal
    {
		g_n = 0;
    }
    
    //if (strstr(raw->opt,"-ENAGLO")) return 0; /* glonass follows */
    
#if 0   //排序处理等，暂时先不用管
    if ( obs_d.n > 0 )
    {
        qsort( obs_d.data, obs_d.n, sizeof( OBSDATA ), cmpobs );
        fwrite( &obs_d, sizeof( Obs_Data ), 1, fobsTmp );
        if ( obsEpochNum == 0 )
        {
            ts = obs_d.time;
            te = ts;
        }
        double dt = timediff( obs_d.time.SecTime, te.SecTime );
        if ( dt > 0.0 )
        {
            te = obs_d.time;
            if ( interval > 0.0 && interval > dt && dt > 0.0 )
            {
                interval = dt;
            }
        }
        //  pObsData.Add(obs_d);
        obsEpochNum++;
    }
#endif
    
    return 1;
}


static int decode_creseph( raw_t *raw )
{
    eph_t eph = {0};
    //unsigned char   *p = raw + 8, buff[90];
    unsigned char   *p = raw->buff + 8;
    unsigned char buff[90] = { 0 };
    unsigned short  prn;
    int sat, k, j, i;
    unsigned int word;
    //gtime_t tzereo = {0};
    
    prn = U2( p );
    if ( !( sat = satno( SYS_GPS, prn ) ) )
    {
        return 0;
    }
    for ( i = 0; i < 3; i++ )
        for ( j = 0; j < 10; j++ )
        {
            word = U4( p + 8 + i * 40 + j * 4 ) >> 6;
            for ( k = 0; k < 3; k++ )
            {
                buff[i * 30 + j * 3 + k] = ( unsigned char )( ( word >> ( 8 * ( 2 - k ) ) ) & 0xFF );
            }
        }
        
    if ( decode_frame( buff, &eph, NULL, NULL, NULL, NULL ) != 1 ||
            decode_frame( buff + 30, &eph, NULL, NULL, NULL, NULL ) != 2 ||
            decode_frame( buff + 60, &eph, NULL, NULL, NULL, NULL ) != 3 )   //zy change 5arg -> 6arg
    {
        return 0;
    }

	eph.sat = sat;
    raw->nav.eph[sat - 1] = eph;
    //raw->nav.eph[prn - 1] = eph;
    raw->ephsat = sat;

    //zy mask 2016.1.11
#if 0
    for ( int i = 0; i < pEphData.GetSize(); i++ )
    {
        eph_t &eph = pEphData.GetAt( i );
        if ( eph.sat == sat )
        {
            if ( eph.iode == eph.iode )
            {
                return 0;    /* unchanged */
            }
        }
    }
    eph.sat = sat;
    bool isFound = false;
    for ( int i = 0; i < pEphData.GetCount(); i++ )
    {
        eph_t &tmpeph = pEphData.GetAt( i );
        if ( tmpeph.sat == eph.sat )
        {
            if ( timediff( eph.toe, tzereo ) > 0.0 && ( fabs( timediff( tmpeph.toe, eph.toe ) ) < 0.05 ) )
            {
                isFound = true;
                break;
            }
            if ( timediff( eph.toc, tzereo ) > 0.0 && ( fabs( timediff( tmpeph.toe, eph.toe ) ) < 0.05 ) )
            {
                isFound = true;
                break;
            }
            
        }
    }
    if ( !isFound )
    {
        pEphData.Add( eph );
    }
#endif
    return 2;
}


/* decode bin 94 ion/utc parameters ------------------------------------------*/
static int decode_cresionutc( raw_t *raw )
{
    int i;
    unsigned char *p = raw->buff + 8;
    
    for ( i = 0; i < 8; i++ )
    {
        ion_r[i] = R8( p + i * 8 );
    }
    utc_r[0] = R8( p + 64 );
    utc_r[1] = R8( p + 72 );
    utc_r[2] = ( double )U4( p + 80 );
    utc_r[3] = ( double )U2( p + 84 );
    leap_r = I2( p + 90 );
    return 9;
}

/* decode bin 66 glonass L1/L2 code and carrier phase ------------------------*/
static int decode_cresgloraw( raw_t *raw )
{
    gtime_t time;
    double tow, tows, toff = 0.0, cp[2] = {0}, pr1, pr[2] = {0}, dop[2] = {0}, snr[2] = {0};
    int i, j, n = 0, prn, sat, week, lli[2] = {0};
    unsigned int word1, word2, word3, sc, sn, validphase;
    //unsigned char *p = raw + 8;
    unsigned char *p = raw->buff + 8;
    //unsigned short len = U2( raw + 6 );
    unsigned short len = U2( raw->buff + 6 );
    double ep[6] = {0.0};
    //Obs_Data obs_d = {0};
    
    //trace(4,"decode_cregloraw: len=%d\n",raw->len);
    
    //if (!strstr(raw->opt,"-ENAGLO")) return 0;
    
    if ( len != 364 - 12 )
    {
        //trace(2,"crescent bin 66 message length error: len=%d\n",raw->len);
        return -1;
    }
    tow = R8( p );
    week = U2( p + 8 );
    tows = floor( tow * 1000.0 + 0.5 ) / 1000.0; /* round by 1ms */
    time = gpst2time( week, tows );
    ghtime.week = week;
    ghtime.sec = tow;
    ghtime.SecTime = gpst2time( ghtime.week, ghtime.sec );
    time2epoch( ghtime.SecTime, ep );
    ghtime.SecTime = time;
    ghtime.year = ( int )ep[0];
    ghtime.month = ( int )ep[1];
    ghtime.day = ( int )ep[2];
    ghtime.hour = ( int )ep[3];
    ghtime.min = ( int )ep[4];
    ghtime.second = ep[5];

    /* time tag offset correction */
    //if (strstr(raw->opt,"-TTCORR"))
    {
        //toff=CLIGHT*(tows-tow);
    }
    //if (fabs(timediff(time,raw->time))<1e-9) {
    //  n=obs_d.n;
    //}
    for ( i = 0, p += 16; i < 12 && n < MAXOBS; i++ )
    {
        word1 = U4( p + 288 + 4 * i ); /* L1CACodeMSBsSlot */
        if ( ( prn = word1 & 0xFF ) == 0 )
        {
            continue;    /* if 0, no data */
        }
        if ( !( sat = satno( SYS_GLO, prn ) ) )
        {
            //trace(2,"creasent bin 66 satellite number error: prn=%d\n",prn);
            continue;
        }
        pr1 = ( word1 >> 13 ) * 256.0; /* upper 19bit of L1CA pseudorange */
        
        /* L1Obs */
        word1 = U4( p  + 12 * i );
        word2 = U4( p + 4 + 12 * i );
        word3 = U4( p + 8 + 12 * i );
        sn = word1 & 0xFFF;
        snr[0] = sn == 0 ? 0.0 : 10.0 * log10( 0.1024 * sn ) + SNR2CN0_L1;
        sc = ( unsigned int )( word1 >> 24 );
        //if (raw->time.time!=0) {
        //  lli[0]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][0])>0;
        //}
        //else {
        lli[0] = 0;
        //}
        lli[0] |= ( ( word1 >> 12 ) & 7 ) ? 2 : 0;
        //raw->lockt[sat-1][0]=(unsigned char)sc;
        dop[0] = ( ( word2 >> 1 ) & 0x7FFFFF ) / 512.0;
        validphase = word2 & 1;
        if ( !validphase )
        {
            continue;
        }
        if ( ( word2 >> 24 ) & 1 )
        {
            dop[0] = -dop[0];
        }
        pr[0] = pr1 + ( word3 & 0xFFFF ) / 256.0;
        cp[0] = floor( pr[0] / lam_glo[prn - 1] / 8192.0 ) * 8192.0;
        cp[0] += ( ( word2 & 0xFE000000 ) + ( ( word3 & 0xFFFF0000 ) >> 7 ) ) / 524288.0;
        if      ( cp[0] - pr[0] / lam_glo[prn - 1] < -4096.0 )
        {
            cp[0] += 8192.0;
        }
        else if ( cp[0] - pr[0] / lam_glo[prn - 1] > 4096.0 )
        {
            cp[0] -= 8192.0;
        }
        
        /* L2Obs */
        word1 = U4( p + 144 + 12 * i );
        word2 = U4( p + 148 + 12 * i );
        word3 = U4( p + 152 + 12 * i );
        sn = word1 & 0xFFF;
        snr[1] = sn == 0 ? 0.0 : 10.0 * log10( 0.1164 * sn ) + SNR2CN0_L2;
        sc = ( unsigned int )( word1 >> 24 );
        //if (raw->time.time==0) {
        //  lli[1]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][1])>0;
        //}
        //else {
        lli[1] = 0;
        //}
        lli[1] |= ( ( word1 >> 12 ) & 7 ) ? 2 : 0;
        //raw->lockt[sat-1][1]=(unsigned char)sc;
        dop[1] = ( ( word2 >> 1 ) & 0x7FFFFF ) / 512.0;
        if ( ( word2 >> 24 ) & 1 )
        {
            dop[1] = -dop[1];
        }
        pr[1] = ( word3 & 0xFFFF ) / 256.0;
        if ( pr[1] != 0.0 )
        {
            pr[1] += pr1;
            if      ( pr[1] - pr[0] < -128.0 )
            {
                pr[1] += 256.0;
            }
            else if ( pr[1] - pr[0] > 128.0 )
            {
                pr[1] -= 256.0;
            }
            cp[1] = floor( pr[1] / lam_glo[24 + prn - 1] / 8192.0 ) * 8192.0;
            cp[1] += ( ( word2 & 0xFE000000 ) + ( ( word3 & 0xFFFF0000 ) >> 7 ) ) / 524288.0;
            if      ( cp[1] - pr[1] / lam_glo[24 + prn - 1] < -4096.0 )
            {
                cp[1] += 8192.0;
            }
            else if ( cp[1] - pr[1] / lam_glo[24 + prn - 1] > 4096.0 )
            {
                cp[1] -= 8192.0;
            }
        }
        else
        {
            cp[1] = 0.0;
        }
        
        raw->obs.data[n + g_n].time = time;
        raw->obs.data[n + g_n].sat  = sat;

        //obs_d.data[n].sys = SYS_GLO;
        for ( j = 0; j < NFREQ; j++ )
        {
            if ( j == 0 || ( j == 1 && i < 12 ) )
            {
                raw->obs.data[n + g_n].P[j] = pr[j] == 0.0 ? 0.0 : pr[j] - toff;
                raw->obs.data[n + g_n].L[j] = -( cp[j] == 0.0 ? 0.0 : cp[j] - toff / lam_glo[j * 24 + prn - 1] );
                raw->obs.data[n + g_n].D[j] = -( float )dop[j];
                raw->obs.data[n + g_n].SNR[j] = ( unsigned char )( snr[j]/**4.0*/ + 0.5 );
                raw->obs.data[n + g_n].LLI[j] = ( unsigned char )lli[j];
                raw->obs.data[n + g_n].code[j] = j == 0 ? CODE_L1C : CODE_L2P;
            }
            else
            {
                raw->obs.data[n + g_n].L[j] = raw->obs.data[n + g_n].P[j] = 0.0;
                raw->obs.data[n + g_n].D[j] = 0.0;
                raw->obs.data[n + g_n].SNR[j] = raw->obs.data[n + g_n].LLI[j] = 0;
                raw->obs.data[n + g_n].code[j] = CODE_NONE;
            }
        }
        n++;
    }
    //raw->time=time;

    fprintf( stderr, "glo: %02d %2.0f %2.0f %2.0f %2.0f%11.7f  \n",  ( int )ep[0] % 100, ep[1], ep[2], ep[3], ep[4], ep[5] );

    if( time.time != g_time.time )
    {
		fprintf( stderr," %d, %d \n", (int)time.time, (int)g_time.time);

		g_time = time;
		g_n = n;
    }
    else //equal
    {
		g_n += n;
    }
    
#if 0   //排序处理等，暂时先不用管
    
    if ( obs_d.n > 0 )
    {
        qsort( obs_d.data, obs_d.n, sizeof( OBSDATA ), cmpobs );
        fseek( fobsTmp, -sizeof( Obs_Data ), SEEK_CUR );
        long pos = ftell( fobsTmp );
        if ( pos != -1 )
        {
            Obs_Data obs_d_ = {0};
            if ( fread( &obs_d_, sizeof( Obs_Data ), 1, fobsTmp ) )
            {
                if ( fabs( timediff( obs_d_.time.SecTime, obs_d.time.SecTime ) ) < DTTOL( 1e-6 ) )
                {
                    for ( int i = 0; i < n; i++ )
                    {
                        obs_d_.data[obs_d_.n++] = obs_d.data[i];
                    }
                    obs_d = obs_d_;
                    fseek( fobsTmp, -sizeof( Obs_Data ), SEEK_CUR );
                    pos = ftell( fobsTmp );
                    obsEpochNum--;
                }
            }
        }
        pos = ftell( fobsTmp );
        fseek( fobsTmp, pos, SEEK_SET );
        fwrite( &obs_d, sizeof( Obs_Data ), 1, fobsTmp );
        pos = ftell( fobsTmp );
        if ( obsEpochNum == 0 )
        {
            ts = obs_d.time;
            te = ts;
        }
        double dt = timediff( obs_d.time.SecTime, te.SecTime );
        if ( dt > 0.0 )
        {
            te = obs_d.time;
            if ( interval > 0.0 && interval > dt && dt > 0.0 )
            {
                interval = dt;
            }
        }
        //  pObsData.Add(obs_d);
        obsEpochNum++;
    }
#endif
    return 1;
}

#if 1
/* decode bin 65 glonass ephemeris -------------------------------------------*/
static int decode_cresgloeph( raw_t *raw )
{
    geph_t geph = {0};
    unsigned char *p = raw->buff + 8, str[12];
    int i, j, k, sat, prn, frq, time, no;
    unsigned char subfrm[MAXSAT][380];
    //gtime_t tzereo = {0};
    
    //trace(4,"decode_cregloeph: len=%d\n",raw->len);
    
    //if (!strstr(raw->opt,"-ENAGLO")) return 0;
    
    prn = U1( p );
    p += 1;
    frq = U1( p ) - 8;
    p += 1 + 2;
    time = U4( p );
    p += 4;
    
    if ( !( sat = satno( SYS_GLO, prn ) ) )
    {
        //trace(2,"creasent bin 65 satellite number error: prn=%d\n",prn);
        return -1;
    }
    for ( i = 0; i < 5; i++ )
    {
        for ( j = 0; j < 3; j++ ) for ( k = 3; k >= 0; k-- )
            {
                str[k + j * 4] = U1( p++ );
            }
        if ( ( no = getbitu( str, 1, 4 ) ) != i + 1 )
        {
            //trace(2,"creasent bin 65 string no error: sat=%2d no=%d %d\n",sat,
            //i+1,no);
            return -1;
        }
        memcpy( subfrm[sat - 1] + 10 * i, str, 10 );
    }
    /* decode glonass ephemeris strings */
    geph.tof = ghtime.SecTime;
    if ( !decode_glostr( subfrm[sat - 1], &geph ) || geph.sat != sat )
    {
        return -1;
    }
    geph.frq = frq;
    
    //if (!strstr(raw->opt,"-EPHALL")) {
    //  if (geph.iode==raw->nav.geph[prn-1].iode) return 0; /* unchanged */
    //}
    raw->nav.geph[prn-1] = geph;
    raw->ephsat=sat;

/*    
    for ( int i = 0; i < pGephData.GetSize(); i++ )
    {
        geph_t &gloeph = pGephData.GetAt( i );
        if ( gloeph.sat == sat )
        {
            if ( fabs( timediff( geph.toe, gloeph.toe ) ) < 1.0 && geph.svh == gloeph.svh )
            {
                return 0; // unchanged 
            }
        }
    }
*/    
    //geph.sat = sat;
    
#if 0   //排序处理等，暂时先不用管

    bool isFound = false;
    for ( int i = 0; i < pGephData.GetCount(); i++ )
    {
        geph_t &tmpgeph = pGephData.GetAt( i );
        if ( tmpgeph.sat == geph.sat )
        {
            if ( timediff( geph.toe, tzereo ) > 0.0 && ( fabs( timediff( tmpgeph.toe, geph.toe ) ) < 0.05 ) )
            {
                isFound = true;
                break;
            }
            
        }
    }
    if ( !isFound )
    {
        pGephData.Add( geph );
    }
#endif

    return 2;
}
#endif


/*------------------------------------------*/
static int decode_cresbdseph( raw_t *raw )
{
    unsigned char *p = raw->buff + 8;
    //unsigned short len = U2( raw + 6 );
    unsigned short len = U2( raw->buff + 6 );
    int prn = 0, sat = 0;
    long week, tow = 0, bds_tow = 0;
    long tgd, ion, svh_iodc_ura_iode;
    double toc = 0.0, toe = 0.0;
    eph_t eph = {0};
    //gtime_t tzereo = {0};
    const double secperweek = 604800.0;
    if ( len != 140 - 12 )
    {
        return -1;
    }
    
    prn = U2( p );
    p += 4;
    if ( prn <= 0 )
    {
        return -1;
    }
    sat = satno( prn < MAXPRNCMP ? SYS_CMP : SYS_NONE, prn );
    tow = U4( p );
    p += 4;
    bds_tow = U4( p );
    p += 4;
    toc = 8 * U4( p )/*+BD2GPST_SEC*/;
    p += 4;
    //eph.toc = toc;
    //if(toc > secperweek)
    //{
    //  toc -= secperweek;
    //}
    eph.tocs = toc;
    eph.f0 = P2_33 * I4( p );
    p += 4;
    eph.f1 = P2_50 * I4( p );
    p += 4;
    eph.f2 = P2_66 * I4( p );
    p += 4;
    toe = 8 * U4( p )/*+BD2GPST_SEC*/;
    p += 4;
    //eph.toe = toe;
    //if(toe > secperweek)
    //{
    //  toe -= secperweek;
    //}
    eph.toes = toe;
    eph.sqrtA = P2_19 * U4( p );
    p += 4;
    eph.A = eph.sqrtA * eph.sqrtA;
    eph.e = P2_33 * U4( p );
    p += 4;
    eph.omg = P2_31 * L4( p ) * PI;
    p += 4;
    eph.deln = P2_43 * L4( p ) * PI;
    p += 4;
    eph.M0 = P2_31 * L4( p ) * PI;
    p += 4;
    eph.OMG0 = P2_31 * L4( p ) * PI;
    p += 4;
    eph.OMGd = P2_43 * L4( p ) * PI;
    p += 4;
    eph.i0 = P2_31 * L4( p ) * PI;
    p += 4;
    eph.idot = P2_43 * L4( p ) * PI;
    p += 4;
    eph.cuc = P2_31 * L4( p );
    p += 4;
    eph.cus = P2_31 * L4( p );
    p += 4;
    eph.crc = P2_6 * L4( p );
    p += 4;
    eph.crs = P2_6 * L4( p );
    p += 4;
    eph.cic = P2_31 * L4( p );
    p += 4;
    eph.cis = P2_31 * L4( p );
    p += 4;
    tgd = U4( p );
    p += 4;
    eph.tgd[0] = 0.1 * 1e-9 * ( tgd & 0x3FF );
    eph.tgd[1] = 0.1 * 1e-9 * ( ( tgd >> 10 ) & 0x3FF );
    week = U4( p );
    p += 4;
    eph.week = week + BD2GPST_WEEK;
    eph.toe = gpst2time( eph.week, toe );
    eph.toc = gpst2time( eph.week, toc );
    eph.ttr = gpst2time( eph.week, tow );
    ion = U4( p );
    p += 4;
    nav_head.ion_cmp[0] = P2_30 * ( ( ion >> 24 ) & 0xFF );
    nav_head.ion_cmp[1] = P2_27 * ( ( ion >> 16 ) & 0xFF );
    nav_head.ion_cmp[2] = P2_24 * ( ( ion >> 8 ) & 0xFF );
    nav_head.ion_cmp[3] = P2_24 * ( ion & 0xFF );
    ion = U4( p );
    p += 4;
    nav_head.ion_cmp[4] = 2048.0 * ( ( ion >> 24 ) & 0xFF );
    nav_head.ion_cmp[5] = 16384.0 * ( ( ion >> 16 ) & 0xFF );
    nav_head.ion_cmp[6] = 65536.0 * ( ( ion >> 8 ) & 0xFF );
    nav_head.ion_cmp[7] = 65536.0 * ( ion & 0xFF );
    svh_iodc_ura_iode = U4( p );
    p += 4;
    eph.iode = svh_iodc_ura_iode & 0x1F;
    eph.sva = ( svh_iodc_ura_iode >> 5 ) & 0xF;
    eph.iodc = ( svh_iodc_ura_iode >> 9 ) & 0x1F;
    eph.svh = ( svh_iodc_ura_iode >> 14 ) & 1;
    
    
    if ( eph.toes < tow - 302400.0 )
    {
        eph.week++;
        tow -= secperweek;
    }
    else if ( eph.toes > tow + 302400.0 )
    {
        eph.week--;
        tow += secperweek;
    }
    
    eph.toe = gpst2time( eph.week, eph.toes );
    eph.toe = timeadd( eph.toe, 14 );
    eph.toc = gpst2time( eph.week, toc );
    eph.ttr = gpst2time( eph.week, tow );

    eph.sat = sat/*prn*/;

    //raw->nav.eph[prn-1] = eph;
	raw->nav.eph[sat - 1] = eph;
    raw->ephsat = sat;

/*    
    for ( int i = 0; i < pBephData.GetSize(); i++ )
    {
        eph_t &bdeph = pBephData.GetAt( i );
        if ( bdeph.sat == prn )
        {
            if ( eph.tocs != eph.toes || eph.tocs == bdeph.tocs )
            {
                return 0;
            }
        }
    }  

    bool isFound = false;
    for ( int i = 0; i < pBephData.GetCount(); i++ )
    {
        eph_t &tmpeph = pBephData.GetAt( i );
        if ( tmpeph.sat == eph.sat )
        {
            if ( timediff( eph.toe, tzereo ) > 0.0 && ( fabs( timediff( tmpeph.toe, eph.toe ) ) < 0.05 ) )
            {
                isFound = true;
                break;
            }
            if ( timediff( eph.toc, tzereo ) > 0.0 && ( fabs( timediff( tmpeph.toe, eph.toe ) ) < 0.05 ) )
            {
                isFound = true;
                break;
            }
            
        }
    }
    if ( !isFound )
    {
        pBephData.Add( eph );
    }
*/
	if ( !strstr( raw->opt, "-EPHALL" ) )
	{
		if (eph.iode == raw->nav.eph[sat-1].iode)
		{
			 return (0); /* unchanged */
		}
		if ( fabs( timediff( eph.toe, raw->nav.eph[sat - 1].toe ) ) < 1E-6 )
		{
			return ( 0 );	 
		}
		if ( fabs( timediff( eph.toc, raw->nav.eph[sat - 1].toc ) ) < 1E-6 )
		{
			return ( 0 );	 //zy modified
		}
	}


    //return 1;
    return 2;
}



/*------------------------------------------*/
static int decode_cresbdsraw( raw_t *raw )
{
    gtime_t time;
    double tow, tows, toff = 0.0, cp[NFREQ] = {0}, pr1, pr[NFREQ] = {0}, dop[NFREQ] = {0}, snr[NFREQ] = {0};
    int i, j, n = 0, prn, sat, week, lli[NFREQ] = {0};
    unsigned int word1, word2, word3, sc, sn;
    //unsigned char *p = raw + 8;
    unsigned char *p = raw->buff + 8;
    //unsigned short len = U2( raw + 6 );
    unsigned short len = U2( raw->buff + 6 );
    int pages = 0, page = 0, signalid = 0, tracktime = 0, validphase = 0;
    double ep[6] = {0.0}, ttrack = 0.0;
    //static Obs_Data obs_d = {0};            //全部页结束后再存储
    //bool isfound = false;
    unsigned int isfound = 0;
    static int g_bd_n = 0;
    
    if ( len != 348 - 12 )
    {
        return -1;
    }
    
    tow = R8( p );
    week = U2( p + 8 );
    tows = floor( tow * 1000.0 + 0.5 ) / 1000.0; /* round by 1ms */
    time = gpst2time( week, tows );
    ghtime.week = week;
    ghtime.sec = tow;
    ghtime.SecTime = gpst2time( ghtime.week, ghtime.sec );
    time2epoch( ghtime.SecTime, ep );
    ghtime.SecTime = time;
    ghtime.year = ( int )ep[0];
    ghtime.month = ( int )ep[1];
    ghtime.day = ( int )ep[2];
    ghtime.hour = ( int )ep[3];
    ghtime.min = ( int )ep[4];
    ghtime.second = ep[5];
    
    //obs_d.time = ghtime;
    /* time tag offset correction */
    //if (strstr(raw->opt,"-TTCORR"))
    //{
    //toff=CLIGHT*(tows-tow);
    //}
    word1 = U4( p + 12 );
    pages = ( word1 >> 20 ) & 0xF;
    page = ( word1 >> 24 ) & 0xF;
    signalid = ( word1 >> 28 ) & 0xF;
    if ( signalid < 0 || signalid >= NFREQ )
    {
        return -1;
    }
    
    for ( i = 0, p += 16; i < 20 && n < MAXOBS; i++ )
    {
        word1 = U4( p + 240 + i * 4 ); //L1CodeMSBsPRN
        if ( ( prn = word1 & 0xFF ) == 0 )
        {
            continue;    /* if 0, no data */
        }
        if ( !( sat = satno( SYS_CMP, prn ) ) )
        {
            //trace(2,"creasent bin 66 satellite number error: prn=%d\n",prn);
            continue;
        }
        pr1 = ( word1 >> 13 ) * 256.0; /* upper 19bit of L1CA pseudorange */
        if ( pr1 == 0.0 )
        {
            continue;
        }
        
        if ( signalid == 0 )
        {
            /* L1Obs */
            word1 = U4( p  + 12 * i );
            word2 = U4( p + 4 + 12 * i );
            word3 = U4( p + 8 + 12 * i );
            sn = word1 & 0xFFF;
            snr[0] = sn == 0 ? 0.0 : 10.0 * log10( 0.1024 * sn ) + SNR2CN0_L1;
            sc = ( unsigned int )( word1 >> 24 );
            //if (raw->time.time!=0) {
            //  lli[0]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][0])>0;
            //}
            //else {
            lli[0] = 0;
            //}
            tracktime = ( word1 >> 15 ) & 1;
            ttrack = 0.1 * ( ( word1 >> 16 ) & 7 );
            if ( ttrack < 0.0 || ttrack > 25.5 )
            {
                continue;
            }
            lli[0] |= ( ( word1 >> 12 ) & 7 ) ? 2 : 0;
            //raw->lockt[sat-1][0]=(unsigned char)sc;
            validphase = ( word2 & 1 );
            if ( !validphase )
            {
                continue;
            }
            dop[0] = ( ( word2 >> 1 ) & 0x7FFFFF ) / 512.0;
            if ( ( word2 >> 24 ) & 1 )
            {
                dop[0] = -dop[0];
            }
            pr[0] = pr1 + ( word3 & 0xFFFF ) / 256.0;
            cp[0] = floor( pr[0] / lam_bds[0] / 8192.0 ) * 8192.0;
            cp[0] += ( ( word2 & 0xFE000000 ) + ( ( word3 & 0xFFFF0000 ) >> 7 ) ) / 524288.0;
            if      ( cp[0] - pr[0] / lam_bds[0] < -4096.0 )
            {
                cp[0] += 8192.0;
            }
            else if ( cp[0] - pr[0] / lam_bds[0] > 4096.0 )
            {
                cp[0] -= 8192.0;
            }
        }
        else if ( signalid == 1 )
        {
            /* L2Obs */
            word1 = U4( p  + 12 * i );
            word2 = U4( p + 4 + 12 * i );
            word3 = U4( p + 8 + 12 * i );
            sn = word1 & 0xFFF;
            snr[1] = sn == 0 ? 0.0 : 10.0 * log10( 0.1164 * sn ) + SNR2CN0_L2;
            sc = ( unsigned int )( word1 >> 24 );
            //if (raw->time.time==0) {
            //  lli[1]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][1])>0;
            //}
            //else {
            lli[1] = 0;
            //}
            lli[1] |= ( ( word1 >> 12 ) & 7 ) ? 2 : 0;
            //raw->lockt[sat-1][1]=(unsigned char)sc;
            dop[1] = ( ( word2 >> 1 ) & 0x7FFFFF ) / 512.0;
            if ( ( word2 >> 24 ) & 1 )
            {
                dop[1] = -dop[1];
            }
            pr[1] = ( word3 & 0xFFFF ) / 256.0;
            if ( pr[1] != 0.0 )
            {
                pr[1] += pr1;
                if      ( pr[1] - pr[0] < -128.0 )
                {
                    pr[1] += 256.0;
                }
                else if ( pr[1] - pr[0] > 128.0 )
                {
                    pr[1] -= 256.0;
                }
                cp[1] = floor( pr[1] / lam_bds[1] / 8192.0 ) * 8192.0;
                cp[1] += ( ( word2 & 0xFE000000 ) + ( ( word3 & 0xFFFF0000 ) >> 7 ) ) / 524288.0;
                if      ( cp[1] - pr[1] / lam_bds[1] < -4096.0 )
                {
                    cp[1] += 8192.0;
                }
                else if ( cp[1] - pr[1] / lam_bds[1] > 4096.0 )
                {
                    cp[1] -= 8192.0;
                }
            }
        }
        else if ( signalid == 2 )
        {
            /* L3Obs */
            word1 = U4( p  + 12 * i );
            word2 = U4( p + 4 + 12 * i );
            word3 = U4( p + 8 + 12 * i );
            sn = word1 & 0xFFF;
            snr[2] = sn == 0 ? 0.0 : 10.0 * log10( 0.1164 * sn ) + SNR2CN0_L2;
            sc = ( unsigned int )( word1 >> 24 );
            //if (raw->time.time==0) {
            //  lli[1]=(int)((unsigned char)sc-(unsigned char)raw->lockt[sat-1][1])>0;
            //}
            //else {
            lli[1] = 0;
            //}
            lli[2] |= ( ( word1 >> 12 ) & 7 ) ? 2 : 0;
            //raw->lockt[sat-1][2]=(unsigned char)sc;
            dop[2] = ( ( word2 >> 1 ) & 0x7FFFFF ) / 512.0;
            if ( ( word2 >> 24 ) & 1 )
            {
                dop[2] = -dop[2];
            }
            pr[2] = ( word3 & 0xFFFF ) / 256.0;
            if ( pr[2] != 0.0 )
            {
                pr[2] += pr1;
                if      ( pr[2] - pr[0] < -128.0 )
                {
                    pr[2] += 256.0;
                }
                else if ( pr[2] - pr[0] > 128.0 )
                {
                    pr[2] -= 256.0;
                }
                cp[2] = floor( pr[2] / lam_bds[2] / 8192.0 ) * 8192.0;
                cp[2] += ( ( word2 & 0xFE000000 ) + ( ( word3 & 0xFFFF0000 ) >> 7 ) ) / 524288.0;
                if      ( cp[2] - pr[2] / lam_bds[2] < -4096.0 )
                {
                    cp[2] += 8192.0;
                }
                else if ( cp[2] - pr[2] / lam_bds[2] > 4096.0 )
                {
                    cp[2] -= 8192.0;
                }
            }
        }
        
        //B1 Obs
        if ( signalid == 0 )
        {
            //isfound = false;
            isfound = 0;
			
            for ( j = 0; j < n; j++ )
            {
                if ( raw->obs.data[j + g_n].sat == sat )
                {
                    //isfound = true;
                    isfound = 1;
                    raw->obs.data[j + g_n].time = time;
                    raw->obs.data[j + g_n].sat = sat;
                    //raw->obs.data[j].sys = SYS_CMP;
                    raw->obs.data[j + g_n].P[0] = pr[0] == 0.0 ? 0.0 : pr[0] - toff;
                    raw->obs.data[j + g_n].L[0] = -( cp[0] == 0.0 ? 0.0 : cp[0] - toff / lam_bds[0] );
                    raw->obs.data[j + g_n].D[0] = -( float )dop[0];
                    raw->obs.data[j + g_n].SNR[0] = ( unsigned char )( snr[0]/**4.0*/ + 0.5 );
                    raw->obs.data[j + g_n].LLI[0] = ( unsigned char )lli[0];
                    raw->obs.data[j + g_n].code[0] = CODE_L1I; //CODE_L1C;
                    break;
                }
            }
            if ( !isfound )
            {
                raw->obs.data[n + g_n].time = time;
                raw->obs.data[n + g_n].sat = sat;
                //raw->obs.data[n].sys = SYS_CMP;
                raw->obs.data[n + g_n].P[0] = pr[0] == 0.0 ? 0.0 : pr[0] - toff;
                raw->obs.data[n + g_n].L[0] = -( cp[0] == 0.0 ? 0.0 : cp[0] - toff / lam_bds[0] );
                raw->obs.data[n + g_n].D[0] = -( float )dop[0];
                raw->obs.data[n + g_n].SNR[0] = ( unsigned char )( snr[0]/**4.0*/ + 0.5 );
                raw->obs.data[n + g_n].LLI[0] = ( unsigned char )lli[0];
                raw->obs.data[n + g_n].code[0] = CODE_L1I; //CODE_L1C;
                n++;
            }
        }
        //BD2 Obs
        else if ( signalid == 1 )
        {
            for ( j = 0; j < raw->obs.n; j++ )
            {
                if ( raw->obs.data[j + g_n].sat == sat )
                {
                    raw->obs.data[j + g_n].time = time;
                    raw->obs.data[j + g_n].sat = sat;
                    //raw->obs.data[j].sys = SYS_CMP;
                    raw->obs.data[j + g_n].P[1] = pr[1] == 0.0 ? 0.0 : pr[1] - toff;
                    raw->obs.data[j + g_n].L[1] = -( cp[1] == 0.0 ? 0.0 : cp[1] - toff / lam_bds[1] );
                    raw->obs.data[j + g_n].D[1] = -( float )dop[1];
                    raw->obs.data[j + g_n].SNR[1] = ( unsigned char )( snr[1]/**4.0*/ + 0.5 );
                    raw->obs.data[j + g_n].LLI[1] = ( unsigned char )lli[1];
                    raw->obs.data[j + g_n].code[1] = CODE_L7I; //CODE_L2P;
                    break;
                }
            }
        }
        //BD3 Obs
        else if ( signalid == 2 )
        {
            for ( j = 0; j < raw->obs.n; j++ )
            {
                if ( raw->obs.data[j + g_n].sat == sat )
                {
                    raw->obs.data[j + g_n].time = time;
                    raw->obs.data[j + g_n].sat = sat;
                    //raw->obs.data[j].sys = SYS_CMP;
                    raw->obs.data[j + g_n].P[2] = pr[2] == 0.0 ? 0.0 : pr[2] - toff;
                    raw->obs.data[j + g_n].L[2] = -( cp[2] == 0.0 ? 0.0 : cp[2] - toff / lam_bds[2] );
                    raw->obs.data[j + g_n].D[2] = -( float )dop[2];
                    raw->obs.data[j + g_n].SNR[2] = ( unsigned char )( snr[2]/** 4.0*/ + 0.5 );
                    raw->obs.data[j + g_n].LLI[2] = ( unsigned char )lli[2];
                    raw->obs.data[j + g_n].code[2] = CODE_L6I;
                    break;
                }
            }
        }
    }
    
    if ( signalid == 0 )  //BD1
    {       
	    if( time.time != g_time.time )
	    {
			g_time = time;
			g_n = n;
	    }
	    else //equal
	    {
			g_bd_n += n;   //g_bd_n: temp n, from BD1 to BD3
	    }    
    }
    if ( signalid == 2 )
    {	
		g_n += g_bd_n; 
    	raw->obs.n = g_n;
        b3flg = 1;
        g_bd_n = 0;
	}
	
    if ( page != pages - 1 )
    {
        return 1;
    }
    
#if 0   //排序处理等，暂时先不用管
    
    if ( obs_d.n > 0 )
    {
        qsort( obs_d.data, obs_d.n, sizeof( OBSDATA ), cmpobs );
        fseek( fobsTmp, -sizeof( Obs_Data ), SEEK_CUR );
        long pos = ftell( fobsTmp );
        if ( pos != -1 )
        {
            Obs_Data obs_d_ = {0};
            if ( fread( &obs_d_, sizeof( Obs_Data ), 1, fobsTmp ) )
            {
                if ( fabs( timediff( obs_d_.time.SecTime, obs_d.time.SecTime ) ) < DTTOL( 1e-6 ) )
                {
                    for ( int i = 0; i < obs_d.n; i++ )
                    {
                        obs_d_.data[obs_d_.n++] = obs_d.data[i];
                    }
                    obs_d = obs_d_;
                    fseek( fobsTmp, -sizeof( Obs_Data ), SEEK_CUR );
                    pos = ftell( fobsTmp );
                    obsEpochNum--;
                }
            }
        }
        pos = ftell( fobsTmp );
        fseek( fobsTmp, pos, SEEK_SET );
        fwrite( &obs_d, sizeof( Obs_Data ), 1, fobsTmp );
        pos = ftell( fobsTmp );
        if ( obsEpochNum == 0 )
        {
            ts = obs_d.time;
            te = ts;
        }
        double dt = timediff( obs_d.time.SecTime, te.SecTime );
        if ( dt > 0.0 )
        {
            te = obs_d.time;
            if ( interval > 0.0 && interval > dt && dt > 0.0 )
            {
                interval = dt;
            }
        }
        //  pObsData.Add(obs_d);
        obsEpochNum++;
        Obs_Data obsd = {0};
        obs_d = obsd;
    }
#endif

    return 1;
}


/*------------------------------------------------------------*/
static int decode_p307( raw_t *raw )
{
    //double tow, bd2tow;
    //int msg, week;
    int  type = U2( raw->buff + 4 );   //4 char
    int iReturn = 0;
    //unsigned short bd2leapsec;
    
    //    if ( crc32( raw->buff, raw->len ) != U4( raw->buff + raw->len ) )
    //    {
    //        return -1;
    //    }
    
    //msg = ( U1( raw->buff + 6 ) >> 4 ) & 0x3; //wrong
    //week = adjgpsweek( U2( raw->buff + 14 ) );
    //tow = U4( raw->buff + 16 ) * 0.001;
    //bd2leapsec = U2( raw->buff + 20 );
    //bd2tow = tow - bd2leapsec;
    
    //raw->time = gpst2time( week, tow );
	//raw->obs.data[0].time = gpst2time( week, tow );

    //fprintf( stderr, "===== decode_p307 type = [%d] week[%d] tow[%f] time_t[%d] sec[%f]\n", 
    //					type, week, tow, (int)raw->time.time, raw->time.sec );
        
    /*
    if( ( week == 2048 ) || ( tow == 0.0 ) )
    {
		fprintf( stderr," err week[%d] tow[%f]\n", week, tow );
		//return 0;
    }
    

    if ( msg != 0 )
    {
		fprintf( stderr," err msg[%d] \n", msg );
        return 0;    // message type: 0=binary,1=ascii 
    }
	if ( raw->outtype )
	{
		sprintf( raw->msgtype, "p307 %4d (%4d):  %s", type, raw->len,
				 time_str( gpst2time( week, tow ), 2 ) );
	}
	*/
	
	b3flg = 0;

    switch ( type )
    {
        case ID_CRESPOS   :
            //iReturn =  decode_crespos( raw );
            break;
        case ID_CRESRAW   :
        if(1){
            iReturn = decode_cresraw( raw );
            }
            break;
            
        case ID_CRESRAW2  :
        if(1){
            iReturn = decode_cresraw2( raw );
            }
            break;
            
        case ID_CRESEPH   :
            return decode_creseph( raw );
            break;
            
        case ID_CRESWAAS  :
            break;
            
        case ID_CRESIONUTC:
            //if(wlen == 108)
            //{
               iReturn =  decode_cresionutc( raw );
            //}
            break;
        case ID_CRESGLOEPH:
        if(1){
            return decode_cresgloeph( raw );
            }
            break;
        case ID_CRESGLORAW:
        if(1){
            iReturn = decode_cresgloraw( raw );
            }
            break;
        case ID_CRESBDSEPH:
        if(1){
            return decode_cresbdseph( raw );
         	}
            break;
        case ID_CRESBDSRAW:
            iReturn = decode_cresbdsraw( raw );
            break;
            
        default:
            //continue;
            return 0;
            break;
    }  
    
	if( b3flg == 1 )
	{
		return iReturn;
	}
	else
	{
		return -1;
	} 
    
}


/* zy add ------------------------------------------------------------------------
 * p307 board rinex data conversion
 *
 */
extern int input_cres_zy( raw_t *raw, unsigned char *data, int len, int *read_len )
{
    //fprintf(stderr, "=====input_cres_zy \n");
    data[0] = CRESSYNC[0];
    data[1] = CRESSYNC[1];
    data[2] = CRESSYNC[2];
    data[3] = CRESSYNC[3];
    
    *read_len = len;
    
    if ( len < 8 )
    {
        return -1;
    }
    
    raw->len = U2( data + 6 ) + CRESHLEN;  //data length: offset+6 ?
    if ( raw->len > ( MAXRAWLEN - 4 ) )  //defuault CRCLEN = 4
    {
        return -1;
    }
    
    //fprintf(stderr, "=====input_cres_zy raw->len[%d] \n", raw->len );
    //if ( len < 10 || len < raw->len + UB370CRCLEN )
    if ( len < 10 || len < raw->len + 4 )
    {
        fprintf( stderr, "len < 10 || len < raw->len + 4\n" );
        return 0;
    }
    
    raw->nbyte = 0;
    
    memcpy( raw->buff,  data, len );

    /* decode ub370 message */
    return decode_p307( raw );
}



