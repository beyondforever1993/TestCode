#include "cmn.h"

#define UB370SYNC1   0xAA        /* ub370 message start sync code 1 */
#define UB370SYNC2   0x44        /* ub370 message start sync code 2 */
#define UB370SYNC3   0x12        /* ub370 message start sync code 3 */
#define UB370HLEN    28          /* ub370 message header length (bytes) */
#define UB370CRCLEN  4           /* ub370 crc msg length (bytes) */
#define POLYCRC32   0xEDB88320u /* CRC32 polynomial */
#define ID_BD2EPHEMERIS 1047
#define ID_GPSEPHEMERIS 7
#define ID_GLOEPHEMERIS 723
#define ID_RANGE 43
#define ID_IONUTC 8
#define ID_RAWEPHEM 41
#define OFF_FRQNO   -7
const static double gpst0[] = {1980, 1, 6, 0, 0, 0}; /* gps time reference */
const static double leaps[][7] = /* leap seconds {y,m,d,h,m,s,utc-gpst,...} */
{
    {2012, 7, 1, 0, 0, 0, -16},
    {2009, 1, 1, 0, 0, 0, -15},
    {2006, 1, 1, 0, 0, 0, -14},
    {1999, 1, 1, 0, 0, 0, -13},
    {1997, 7, 1, 0, 0, 0, -12},
    {1996, 1, 1, 0, 0, 0, -11},
    {1994, 7, 1, 0, 0, 0, -10},
    {1993, 7, 1, 0, 0, 0, -9},
    {1992, 7, 1, 0, 0, 0, -8},
    {1991, 1, 1, 0, 0, 0, -7},
    {1990, 1, 1, 0, 0, 0, -6},
    {1988, 1, 1, 0, 0, 0, -5},
    {1985, 7, 1, 0, 0, 0, -4},
    {1983, 7, 1, 0, 0, 0, -3},
    {1982, 7, 1, 0, 0, 0, -2},
    {1981, 7, 1, 0, 0, 0, -1}
};

//static double timeoffset_ = 0.0;      /* time offset (s) */

#define U1(p) (*((unsigned char *)(p)))
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
static float          R4( unsigned char *p )
{
    float          r;
    memcpy( &r, p, 4 );
    return r;
}
static double         R8( unsigned char *p )
{
    double         r;
    memcpy( &r, p, 8 );
    return r;
}

#if 0
static int satno( int sys, int prn )
{
    if ( prn <= 0 )
    {
        return 0;
    }
    switch ( sys )
    {
        case SYS_GPS:
            if ( prn < MINPRNGPS || MAXPRNGPS < prn )
            {
                return 0;
            }
            return prn - MINPRNGPS + 1;
        case SYS_GLO:
            if ( prn < MINPRNGLO || MAXPRNGLO < prn )
            {
                return 0;
            }
            return NSATGPS + prn - MINPRNGLO + 1;
        case SYS_GAL:
            if ( prn < MINPRNGAL || MAXPRNGAL < prn )
            {
                return 0;
            }
            return NSATGPS + NSATGLO + prn - MINPRNGAL + 1;
        case SYS_QZS:
            if ( prn < MINPRNQZS || MAXPRNQZS < prn )
            {
                return 0;
            }
            return NSATGPS + NSATGLO + NSATGAL + prn - MINPRNQZS + 1;
        case SYS_CMP:
            if ( prn < MINPRNCMP || MAXPRNCMP < prn )
            {
                return 0;
            }
            return NSATGPS + NSATGLO + NSATGAL + NSATQZS + prn - MINPRNCMP + 1;
        case SYS_SBS:
            if ( prn < MINPRNSBS || MAXPRNSBS < prn )
            {
                return 0;
            }
            return NSATGPS + NSATGLO + NSATGAL + NSATQZS + NSATCMP + prn - MINPRNSBS + 1;
    }
    return 0;
}
#endif

static int decode_trackstat( unsigned int stat, int *sys, int *code, int *track,
                             int *plock, int *clock, int *parity, int *halfc )
{
    int satsys, sigtype, freq = 0;
    
    *track = stat & 0x1F;
    *plock = ( stat >> 10 ) & 1;
    *parity = ( stat >> 11 ) & 1;
    *clock = ( stat >> 12 ) & 1;
    satsys = ( stat >> 16 ) & 7;
    *halfc = ( stat >> 28 ) & 1;
    sigtype = ( stat >> 21 ) & 0x1F;
    
    switch ( satsys )
    {
        case 0:
            *sys = SYS_GPS;
            break;
        case 1:
            *sys = SYS_GLO;
            break;
        case 4:
            *sys = SYS_CMP;
            break;
        default:
            return -1;
    }
    if ( *sys == SYS_GPS )
    {
        switch ( *track )
        {
            case  4:
                freq = 0;
                *code = CODE_L1C;
                break; /* L1 */
            case  11:
                freq = 1;
                *code = CODE_L2P;
                break; /* L2 */
            default:
                freq = -1;
                break;
        }
    }
    else if ( *sys == SYS_GLO )
    {
        switch ( *track )
        {
            case  4:
                freq = 0;
                *code = CODE_L1C;
                break; /* L1*/
            case  11:
                freq = 1;
                *code = CODE_L2P;
                break; /* L2*/
            default:
                freq = -1;
                break;
        }
    }
    else if ( *sys == SYS_CMP )
    {
        switch ( sigtype )
        {
            case  0:
                freq = 0;
                *code = CODE_L1I;
                break; /* B1 */
            case  17:
                freq = 1;
                *code = CODE_L7I;
                break; /* B2 */
            case 21:
                freq = 2;
                *code = CODE_L6I;
                break;//*code=CODE_L5I; break; /* B5 mark:use L5I,just to compatible for rtklib code*/
            default:
                freq = -1;
                break;
        }
    }
    
    if ( freq < 0 )
    {
        return -1;
    }
    return freq;
}

#if 0
/* check code priority and return obs position -------------------------------*/
static int checkpri( const char *opt, int sys, int code, int freq )
{
    int nex = NEXOBS; /* number of extended obs data */
    
    if ( sys == SYS_GPS )
    {
        if ( strstr( opt, "-GL1P" ) )
        {
            return code == CODE_L1P ? 0 : -1;
        }
        if ( strstr( opt, "-GL2X" ) )
        {
            return code == CODE_L2X ? 1 : -1;
        }
        if ( code == CODE_L1P )
        {
            return nex < 1 ? -1 : NFREQ;
        }
        if ( code == CODE_L2X )
        {
            return nex < 2 ? -1 : NFREQ + 1;
        }
    }
    else if ( sys == SYS_GLO )
    {
        if ( strstr( opt, "-RL2C" ) )
        {
            return code == CODE_L2C ? 1 : -1;
        }
        if ( code == CODE_L2C )
        {
            return nex < 1 ? -1 : NFREQ;
        }
    }
    else if ( sys == SYS_GAL )
    {
        if ( strstr( opt, "-EL1B" ) )
        {
            return code == CODE_L1B ? 0 : -1;
        }
        if ( code == CODE_L1B )
        {
            return nex < 1 ? -1 : NFREQ;
        }
        if ( code == CODE_L7Q )
        {
            return nex < 2 ? -1 : NFREQ + 1;
        }
        if ( code == CODE_L8Q )
        {
            return nex < 3 ? -1 : NFREQ + 2;
        }
    }
    return freq < NFREQ ? freq : -1;
}
#endif

static int sync_ub370( unsigned char *buff, unsigned char data )
{
    buff[0] = buff[1];
    buff[1] = buff[2];
    buff[2] = data;
    return buff[0] == UB370SYNC1 && buff[1] == UB370SYNC2 && buff[2] == UB370SYNC3;
}

static int obsindex( obs_t *obs, gtime_t time, int sat )
{
    int i, j;
    
    if ( obs->n >= MAXOBS )
    {
        return -1;
    }
    for ( i = 0; i < obs->n; i++ )
    {
        if ( obs->data[i].sat == sat )
        {
            return i;
        }
    }
    obs->data[i].time = time;
    obs->data[i].sat = sat;
    for ( j = 0; j < NFREQ + NEXOBS; j++ )
    {
        obs->data[i].L[j] = obs->data[i].P[j] = 0.0;
        obs->data[i].D[j] = 0.0;
        obs->data[i].SNR[j] = obs->data[i].LLI[j] = 0;
        obs->data[i].code[j] = CODE_NONE;
    }
    obs->n++;
    return i;
}
static int decode_rangeb( raw_t *raw )
{
    double psr, adr, dop, snr, lockt, tt;
    char *msg;
    int i, index, nobs, prn, sat, sys, code, freq, pos;
    int track, plock, clock, parity, halfc, lli, gfrq;
    unsigned char *p = raw->buff + UB370HLEN;
    
    nobs = U4( p );
    
    if ( raw->outtype )
    {
        msg = raw->msgtype + strlen( raw->msgtype );
        sprintf( msg, " nobs=%2d", nobs );
    }
    
    if ( raw->len < UB370HLEN + 4 + nobs * 44 )
    {
        return -1;
    }
    for ( i = 0, p += 4; i < nobs; i++, p += 44 )
    {
    
        /* decode tracking status */
        if ( ( freq = decode_trackstat( U4( p + 40 ), &sys, &code, &track, &plock, &clock,
                                        &parity, &halfc ) ) < 0 )
        {
            continue;
        }
        if ( freq >= NFREQ )
        {
            continue;
        }
        pos = freq;
        /* obs position */
        //if ((pos=checkpri(raw->opt,sys,code,freq))<0) continue;
        prn = U2( p );
        if ( sys == SYS_GPS )
        {
            if ( prn > 32 )
            {
                continue;
            }
        }
        else if ( sys == SYS_GLO )
        {
            if ( prn <= 37 || prn >= 62 )
            {
                continue;
            }
            prn -= 37;
        }
        else if ( sys == SYS_CMP )
        {
            if ( prn <= 160 || prn >= 198 )
            {
                continue;
            }
            prn -= 160;
        }
        else
        {
            continue;
        }
        
        if ( !( sat = satno( sys, prn ) ) )
        {
            continue;
        }
        
        gfrq = U2( p + 2 );
        psr  = R8( p + 4 );
        adr  = R8( p + 16 );
        dop  = R4( p + 28 );
        snr  = R4( p + 32 );
        lockt = R4( p + 36 );
        
        /* set glonass frequency channel number */
        if ( sys == SYS_GLO && raw->nav.geph[prn - 1].sat != sat )
        {
            raw->nav.geph[prn - 1].frq = gfrq - 7;
        }
        tt = timediff( raw->time, raw->tobs );
        if ( raw->tobs.time != 0 )
        {
            lli = lockt - raw->lockt[sat - 1][pos] + 0.05 <= tt ||
                  halfc != raw->halfc[sat - 1][pos];
        }
        else
        {
            lli = 0;
        }
        if ( !parity )
        {
            lli |= 2;
        }
        raw->lockt[sat - 1][pos] = lockt;
        raw->halfc[sat - 1][pos] = halfc;
        if ( !clock )
        {
            psr = 0.0;    /* code unlock */
        }
        if ( !plock )
        {
            adr = dop = 0.0;    /* phase unlock */
        }
        
        if ( fabs( timediff( raw->obs.data[0].time, raw->time ) ) > 1E-9 )
        {
            raw->obs.n = 0;
        }
        if ( ( index = obsindex( &raw->obs, raw->time, sat ) ) >= 0 )
        {
            raw->obs.data[index].L  [pos] = -adr;
            raw->obs.data[index].P  [pos] = psr;
            raw->obs.data[index].D  [pos] = ( float )dop;
            raw->obs.data[index].SNR[pos] =
                0.0 <= snr && snr < 255.0 ? ( unsigned char )( snr * 4.0 + 0.5 ) : 0;
            raw->obs.data[index].LLI[pos] = ( unsigned char )lli;
            raw->obs.data[index].code[pos] = code;
        }
    }
    raw->tobs = raw->time;
    return 1;
}
static int decode_bd2ephemerisb( raw_t *raw )
{
    unsigned char *p = raw->buff + UB370HLEN;
    eph_t eph = {0};
    int prn, sat, i = 0, iode2;
    double tow, toc, AS, N;
    
    int zweek;
    if ( raw->len < UB370HLEN + 232 )
    {
        return -1;
    }
    prn = U4( p );
    i += 4;
    prn -= 160;
    if ( !( sat = satno( SYS_CMP, prn ) ) )
    {
        return -1;
    }
    tow = R8( p + i );
    i += 8;
    eph.svh = ( int )U4( p + i );
    i += 4;
    eph.iode = ( int )U4( p + i );
    i += 4;
    iode2 = ( int )U4( p + i );
    i += 4;
    eph.week = ( int )U4( p + i );
    i += 4;
    zweek = U4( p + i );
    i += 4;
    eph.toes = R8( p + i );
    i += 8;
    eph.toe = gpst2time( eph.week, eph.toes );
    eph.ttr = gpst2time( eph.week, eph.toes );
    eph.A = R8( p + i );
    i += 8;
    eph.deln = R8( p + i );
    i += 8;
    eph.M0 = R8( p + i );
    i += 8;
    eph.e = R8( p + i );
    i += 8;
    eph.omg = R8( p + i );
    i += 8;
    eph.cuc = R8( p + i );
    i += 8;
    eph.cus = R8( p + i );
    i += 8;
    eph.crc = R8( p + i );
    i += 8;
    eph.crs = R8( p + i );
    i += 8;
    eph.cic = R8( p + i );
    i += 8;
    eph.cis = R8( p + i );
    i += 8;
    eph.i0 = R8( p + i );
    i += 8;
    eph.idot = R8( p + i );
    i += 8;
    eph.OMG0 = R8( p + i );
    i += 8;
    eph.OMGd = R8( p + i );
    i += 8;
    eph.iodc = U4( p + i );
    i += 4;
    toc = R8( p + i );
    i += 8;
    eph.toc = gpst2time( eph.week, toc );
    eph.tgd[0] = R8( p + i );
    i += 8;
    eph.tgd[1] = R8( p + i );
    i += 8;
    eph.f0 = R8( p + i );
    i += 8;
    eph.f1 = R8( p + i );
    i += 8;
    eph.f2 = R8( p + i );
    i += 8;
    AS = U4( p + i );
    i += 4;
    N = R8( p + i );
    i += 8;
    eph.sva = R8( p + i );
    i += 8;
    
    //eph.toc=timeadd(eph.toc,14);//已经是北斗时间,不用加上14秒
    //eph.toe=timeadd(eph.toe,14);
    
    eph.sat = sat;
    //tya add:避免写文件时星历重复
    if ( fabs( timediff( raw->nav.eph[sat - 1].toe, eph.toe ) ) <= 1E-6 )
    {
        return -1;
    }
    raw->nav.eph[sat - 1] = eph;
    raw->ephsat = sat;
    return 2;
}
static int decode_gpsephemerisb( raw_t *raw )
{
    unsigned char *p = raw->buff + UB370HLEN;
    eph_t eph = {0};
    int prn, sat, i = 0, iode2;
    double tow, toc, AS, N;
    
    int zweek;
    
    if ( raw->len < UB370HLEN + 224 )
    {
        return -1;
    }
    prn = U4( p );
    i += 4;
    if ( !( sat = satno( SYS_GPS, prn ) ) )
    {
        return -1;
    }
    tow = R8( p + i );
    i += 8;
    eph.svh = ( int )U4( p + i );
    i += 4;
    eph.iode = ( int )U4( p + i );
    i += 4;
    iode2 = ( int )U4( p + i );
    i += 4;
    eph.week = ( int )U4( p + i );
    i += 4;
    zweek = U4( p + i );
    i += 4;
    eph.toes = R8( p + i );
    i += 8;
    eph.toe = gpst2time( eph.week, eph.toes );
    eph.ttr = gpst2time( eph.week, eph.toes );
    eph.A = R8( p + i );
    i += 8;
    eph.deln = R8( p + i );
    i += 8;
    eph.M0 = R8( p + i );
    i += 8;
    eph.e = R8( p + i );
    i += 8;
    eph.omg = R8( p + i );
    i += 8;
    eph.cuc = R8( p + i );
    i += 8;
    eph.cus = R8( p + i );
    i += 8;
    eph.crc = R8( p + i );
    i += 8;
    eph.crs = R8( p + i );
    i += 8;
    eph.cic = R8( p + i );
    i += 8;
    eph.cis = R8( p + i );
    i += 8;
    eph.i0 = R8( p + i );
    i += 8;
    eph.idot = R8( p + i );
    i += 8;
    eph.OMG0 = R8( p + i );
    i += 8;
    eph.OMGd = R8( p + i );
    i += 8;
    eph.iodc = U4( p + i );
    i += 4;
    toc = R8( p + i );
    i += 8;
    eph.toc = gpst2time( eph.week, toc );
    eph.tgd[0] = R8( p + i );
    i += 8;
    eph.f0 = R8( p + i );
    i += 8;
    eph.f1 = R8( p + i );
    i += 8;
    eph.f2 = R8( p + i );
    i += 8;
    AS = U4( p + i );
    i += 4;
    N = R8( p + i );
    i += 8;
    eph.sva = R8( p + i );
    i += 8;
    
    eph.sat = sat;
    //tya add:避免写文件时星历重复
    if ( fabs( timediff( raw->nav.eph[sat - 1].toe, eph.toe ) ) <= 1E-6 )
    {
        return -1;
    }
    raw->nav.eph[sat - 1] = eph;
    raw->ephsat = sat;
    return 2;
}
static int decode_gloephemerisb( raw_t *raw )
{
    unsigned char *p = raw->buff + UB370HLEN;
    geph_t geph = {0};
    int prn, sat, i = 0;
    double tow, tof;
    unsigned char sattyp;
    unsigned int iweek, ileap, issue;
    
    if ( raw->len < UB370HLEN + 144 )
    {
        return -1;
    }
    prn = U2( p );
    i += 2;
    prn -= 37;
    if ( !( sat = satno( SYS_GLO, prn ) ) )
    {
        return -1;
    }
    geph.frq = U2( p + i ) + OFF_FRQNO;
    i += 2;
    sattyp = p[i];
    i++;
    //reserved
    i++;
    //
    iweek = U2( p + i );
    i += 2;
    tow = floor( U4( p + i ) * 0.001 + 0.5 );
    i += 4;
    ileap = U4( p + i );
    i += 4;
    ileap = 3600 * 3 - ileap;//算3个时区
    
    //Nt
    i += 2;
    //reserved
    i++;
    //reserved
    i++;
    //
    issue = U4( p + i );
    i += 4;
    //
    geph.svh = U4( p + i );
    i += 4;
    geph.pos[0] = R8( p + i );
    i += 8;
    geph.pos[1] = R8( p + i );
    i += 8;
    geph.pos[2] = R8( p + i );
    i += 8;
    geph.vel[0] = R8( p + i );
    i += 8;
    geph.vel[1] = R8( p + i );
    i += 8;
    geph.vel[2] = R8( p + i );
    i += 8;
    geph.acc[0] = R8( p + i );
    i += 8;
    geph.acc[1] = R8( p + i );
    i += 8;
    geph.acc[2] = R8( p + i );
    i += 8;
    geph.taun = R8( p + i );
    i += 8; /*在写rinex时取负*/
    geph.dtaun = R8( p + i );
    i += 8;
    geph.gamn = R8( p + i );
    i += 8;
    tof = U4( p + i );
    i += 4; //tk(UTC)
    
    i += 4; //P
    i += 4; //Ft
    geph.age = U4( p + i );
    i += 4;
    geph.toe = gpst2time( iweek, tow );
    
    tof += floor( tow / 86400.0 ) * 86400.0;
    if      ( tof < tow - 43200.0 )
    {
        tof += 86400.0;
    }
    else if ( tof > tow + 43200.0 )
    {
        tof -= 86400.0;
    }
    geph.tof = gpst2time( iweek, tof );
    
    geph.sat = sat;
    //tya add:避免写文件时星历重复
    if ( fabs( timediff( raw->nav.geph[prn - 1].tof, geph.tof ) ) <= 1E-6 )
    {
        return -1;
    }
    raw->nav.leaps = ileap; //tya add
    raw->nav.geph[prn - 1] = geph;
    raw->ephsat = sat;
    return 2;
}
/*---------------------decode frame--------------------*/
/* decode navigation data subframe 1 -----------------------------------------*/
static int decode_subfrm1( const unsigned char *buff, eph_t *eph )
{
    double tow, toc;
    int i = 48, week, iodc0, iodc1;
    tow        = getbitu( buff, 24, 17 ) * 6.0;    /* transmission time */
    week       = getbitu( buff, i, 10 );
    i += 10;
    eph->code  = getbitu( buff, i, 2 );
    i += 2;
    eph->sva   = getbitu( buff, i, 4 );
    i += 4;  /* ura index */
    eph->svh   = getbitu( buff, i, 6 );
    i += 6;
    iodc0      = getbitu( buff, i, 2 );
    i += 2;
    eph->flag  = getbitu( buff, i, 1 );
    i += 1 + 87;
    eph->tgd[0] = getbits( buff, i, 8 ) * P2_31;
    i += 8;
    iodc1      = getbitu( buff, i, 8 );
    i += 8;
    toc        = getbitu( buff, i, 16 ) * 16.0;
    i += 16;
    eph->f2    = getbits( buff, i, 8 ) * P2_55;
    i += 8;
    eph->f1    = getbits( buff, i, 16 ) * P2_43;
    i += 16;
    eph->f0    = getbits( buff, i, 22 ) * P2_31;
    
    eph->iodc = ( iodc0 << 8 ) + iodc1;
    eph->week = adjgpsweek( week ); /* week of tow */
    eph->ttr = gpst2time( eph->week, tow );
    eph->toc = gpst2time( eph->week, toc );
    
    return 1;
}
/* decode navigation data subframe 2 -----------------------------------------*/
static int decode_subfrm2( const unsigned char *buff, eph_t *eph )
{
    double sqrtA;
    int i = 48;
    
    eph->iode = getbitu( buff, i, 8 );
    i += 8;
    eph->crs = getbits( buff, i, 16 ) * P2_5;
    i += 16;
    eph->deln = getbits( buff, i, 16 ) * P2_43 * SC2RAD;
    i += 16;
    eph->M0  = getbits( buff, i, 32 ) * P2_31 * SC2RAD;
    i += 32;
    eph->cuc = getbits( buff, i, 16 ) * P2_29;
    i += 16;
    eph->e   = getbitu( buff, i, 32 ) * P2_33;
    i += 32;
    eph->cus = getbits( buff, i, 16 ) * P2_29;
    i += 16;
    sqrtA    = getbitu( buff, i, 32 ) * P2_19;
    i += 32;
    eph->toes = getbitu( buff, i, 16 ) * 16.0;
    i += 16;
    eph->fit = getbitu( buff, i, 1 ) ? 0.0 : 4.0; /* 0:4hr,1:>4hr */
    
    eph->A = sqrtA * sqrtA;
    
    return 2;
}
/* decode navigation data subframe 3 -----------------------------------------*/
static int decode_subfrm3( const unsigned char *buff, eph_t *eph )
{
    double tow, toc;
    int i = 48, iode;
    
    eph->cic = getbits( buff, i, 16 ) * P2_29;
    i += 16;
    eph->OMG0 = getbits( buff, i, 32 ) * P2_31 * SC2RAD;
    i += 32;
    eph->cis = getbits( buff, i, 16 ) * P2_29;
    i += 16;
    eph->i0  = getbits( buff, i, 32 ) * P2_31 * SC2RAD;
    i += 32;
    eph->crc = getbits( buff, i, 16 ) * P2_5;
    i += 16;
    eph->omg = getbits( buff, i, 32 ) * P2_31 * SC2RAD;
    i += 32;
    eph->OMGd = getbits( buff, i, 24 ) * P2_43 * SC2RAD;
    i += 24;
    iode     = getbitu( buff, i, 8 );
    i += 8;
    eph->idot = getbits( buff, i, 14 ) * P2_43 * SC2RAD;
    
    /* check iode and iodc consistency */
    if ( iode != eph->iode || iode != ( eph->iodc & 0xFF ) )
    {
        return 0;
    }
    
    /* adjustment for week handover */
    tow = time2gpst( eph->ttr, &eph->week );
    toc = time2gpst( eph->toc, NULL );
    if      ( eph->toes < tow - 302400.0 )
    {
        eph->week++;
        tow -= 604800.0;
    }
    else if ( eph->toes > tow + 302400.0 )
    {
        eph->week--;
        tow += 604800.0;
    }
    eph->toe = gpst2time( eph->week, eph->toes );
    eph->toc = gpst2time( eph->week, toc );
    eph->ttr = gpst2time( eph->week, tow );
    
    return 3;
}
/* decode almanac ------------------------------------------------------------*/
static void decode_almanac( const unsigned char *buff, alm_t *alm )
{
    gtime_t toa;
    double deltai, sqrtA, tt;
    int i = 50, f0, sat = getbitu( buff, 50, 6 );
    
    if ( !alm || sat < 1 || 32 < sat || alm[sat - 1].week == 0 )
    {
        return;
    }
    
    alm[sat - 1].sat = sat;
    alm[sat - 1].e   = getbits( buff, i, 16 ) * P2_21;
    i += 16;
    alm[sat - 1].toas = getbitu( buff, i, 8 ) * 4096.0;
    i += 8;
    deltai         = getbits( buff, i, 16 ) * P2_19 * SC2RAD;
    i += 16;
    alm[sat - 1].OMGd = getbits( buff, i, 16 ) * P2_38 * SC2RAD;
    i += 16;
    alm[sat - 1].svh = getbitu( buff, i, 8 );
    i += 8;
    sqrtA          = getbitu( buff, i, 24 ) * P2_11;
    i += 24;
    alm[sat - 1].OMG0 = getbits( buff, i, 24 ) * P2_23 * SC2RAD;
    i += 24;
    alm[sat - 1].omg = getbits( buff, i, 24 ) * P2_23 * SC2RAD;
    i += 24;
    alm[sat - 1].M0  = getbits( buff, i, 24 ) * P2_23 * SC2RAD;
    i += 24;
    f0             = getbitu( buff, i, 8 );
    i += 8;
    alm[sat - 1].f1  = getbits( buff, i, 11 ) * P2_38;
    i += 11;
    alm[sat - 1].f0  = getbits( buff, i, 3 ) * P2_17 + f0 * P2_20;
    alm[sat - 1].A   = sqrtA * sqrtA;
    alm[sat - 1].i0  = 0.3 * SC2RAD + deltai;
    
    toa = gpst2time( alm[sat - 1].week, alm[sat - 1].toas );
    tt = timediff( toa, alm[sat - 1].toa );
    if      ( tt < 302400.0 )
    {
        alm[sat - 1].week--;
    }
    else if ( tt > 302400.0 )
    {
        alm[sat - 1].week++;
    }
    alm[sat - 1].toa = gpst2time( alm[sat - 1].week, alm[sat - 1].toas );
}
/* decode navigation data subframe 4 -----------------------------------------*/
static int decode_subfrm4( const unsigned char *buff, alm_t *alm, double *ion,
                           double *utc, int *leaps )
{
    int i, sat, svid = getbitu( buff, 50, 6 );
    
    if ( 25 <= svid && svid <= 32 ) /* page 2,3,4,5,7,8,9,10 */
    {
    
        /* decode almanac */
        decode_almanac( buff, alm );
    }
    else if ( svid == 63 ) /* page 25 */
    {
    
        /* decode as and sv config */
        i = 56;
        for ( sat = 1; sat <= 32; sat++ )
        {
            if ( alm )
            {
                alm[sat - 1].svconf = getbitu( buff, i, 4 );
            }
            i += 4;
        }
        /* decode sv health */
        i = 186;
        for ( sat = 25; sat <= 32; sat++ )
        {
            if ( alm )
            {
                alm[sat - 1].svh   = getbitu( buff, i, 6 );
            }
            i += 6;
        }
    }
    else if ( svid == 56 ) /* page 18 */
    {
    
        /* decode ion/utc parameters */
        if ( ion )
        {
            i = 56;
            ion[0] = getbits( buff, i, 8 ) * P2_30;
            i += 8;
            ion[1] = getbits( buff, i, 8 ) * P2_27;
            i += 8;
            ion[2] = getbits( buff, i, 8 ) * P2_24;
            i += 8;
            ion[3] = getbits( buff, i, 8 ) * P2_24;
            i += 8;
            ion[4] = getbits( buff, i, 8 ) * pow( 2, 11 );
            i += 8;
            ion[5] = getbits( buff, i, 8 ) * pow( 2, 14 );
            i += 8;
            ion[6] = getbits( buff, i, 8 ) * pow( 2, 16 );
            i += 8;
            ion[7] = getbits( buff, i, 8 ) * pow( 2, 16 );
        }
        if ( utc )
        {
            i = 120;
            utc[1] = getbits( buff, i, 24 ) * P2_50;
            i += 24;
            utc[0] = getbits( buff, i, 32 ) * P2_30;
            i += 32;
            utc[2] = getbits( buff, i, 8 ) * pow( 2, 12 );
            i += 8;
            utc[3] = getbitu( buff, i, 8 );
        }
        if ( leaps )
        {
            i = 192;
            *leaps = getbits( buff, i, 8 );
        }
    }
    return 4;
}
/* decode navigation data subframe 5 -----------------------------------------*/
static int decode_subfrm5( const unsigned char *buff, alm_t *alm )
{
    double toas;
    int i, sat, week, svid = getbitu( buff, 50, 6 );
    
    if ( 1 <= svid && svid <= 24 ) /* page 1-24 */
    {
    
        /* decode almanac */
        decode_almanac( buff, alm );
    }
    else if ( svid == 51 ) /* page 25 */
    {
    
        if ( alm )
        {
            i = 56;
            toas = getbitu( buff, i, 8 ) * 4096;
            i += 8;
            week = getbitu( buff, i, 8 );
            i += 8;
            week = adjgpsweek( week );
            
            /* decode sv health */
            for ( sat = 1; sat <= 24; sat++ )
            {
                alm[sat - 1].svh = getbitu( buff, i, 6 );
                i += 6;
            }
            for ( sat = 1; sat <= 32; sat++ )
            {
                alm[sat - 1].toas = toas;
                alm[sat - 1].week = week;
                alm[sat - 1].toa = gpst2time( week, toas );
            }
        }
    }
    return 5;
}

int decode_frame( const unsigned char *buff, eph_t *eph, alm_t *alm,
                         double *ion, double *utc, int *leaps )
{
    int id = getbitu( buff, 43, 3 ); /* subframe id */
    
    switch ( id )
    {
        case 1:
            return decode_subfrm1( buff, eph );
        case 2:
            return decode_subfrm2( buff, eph );
        case 3:
            return decode_subfrm3( buff, eph );
        case 4:
            return decode_subfrm4( buff, alm, ion, utc, leaps );
        case 5:
            return decode_subfrm5( buff, alm );
    }
    return 0;
}

/*-----------------------------------------------------*/
static int decode_rawephemb( raw_t *raw )
{
    unsigned char *p = raw->buff + UB370HLEN;
    eph_t eph = {0};
    int prn, sat;
    
    if ( raw->len < UB370HLEN + 102 )
    {
        return -1;
    }
    prn = U4( p );
    if ( !( sat = satno( SYS_GPS, prn ) ) )
    {
        return -1;
    }
    if ( decode_frame( p + 12, &eph, NULL, NULL, NULL, NULL ) != 1 ||
            decode_frame( p + 42, &eph, NULL, NULL, NULL, NULL ) != 2 ||
            decode_frame( p + 72, &eph, NULL, NULL, NULL, NULL ) != 3 )
    {
        return -1;
    }
    
    eph.sat = sat;
    
    //tya add:避免写文件时星历重复
    if ( fabs( timediff( raw->nav.eph[sat - 1].toe, eph.toe ) ) <= 1E-6 )
    {
        return -1;
    }
    raw->nav.eph[sat - 1] = eph;
    raw->ephsat = sat;
    return 2;
}
static int decode_ionutcb( raw_t *raw )
{
    unsigned char *p = raw->buff + UB370HLEN;
    int i;
    
    if ( raw->len < UB370HLEN + 108 )
    {
        return -1;
    }
    for ( i = 0; i < 8; i++ )
    {
        raw->nav.ion_gps[i] = R8( p + i * 8 );
    }
    raw->nav.utc_gps[0] = R8( p + 72 );
    raw->nav.utc_gps[1] = R8( p + 80 );
    raw->nav.utc_gps[2] = U4( p + 68 );
    raw->nav.utc_gps[3] = U4( p + 64 );
    raw->nav.leaps = I4( p + 96 );
    return 9;
}
static int decode_ub370( raw_t *raw )
{
    double tow, bd2tow;
    int msg, week, type = U2( raw->buff + 4 );
    unsigned short bd2leapsec;
    
    if ( crc32( raw->buff, raw->len ) != U4( raw->buff + raw->len ) )
    {
        return -1;
    }
    msg = ( U1( raw->buff + 6 ) >> 4 ) & 0x3;
    week = adjgpsweek( U2( raw->buff + 14 ) );
    tow = U4( raw->buff + 16 ) * 0.001;
    bd2leapsec = U2( raw->buff + 20 );
    bd2tow = tow - bd2leapsec;
    
    raw->time = gpst2time( week, tow );
    
    if ( raw->outtype )
    {
        sprintf( raw->msgtype, "ub370 %4d (%4d): msg=%d %s", type, raw->len, msg,
                 time_str( gpst2time( week, tow ), 2 ) );
    }
    if ( msg != 0 )
    {
        return 0;    /* message type: 0=binary,1=ascii */
    }
    
    switch ( type )
    {
        case ID_RANGE         :
            return decode_rangeb         ( raw );
        case ID_BD2EPHEMERIS  :
            return decode_bd2ephemerisb ( raw );
        case ID_GPSEPHEMERIS  :
            return decode_gpsephemerisb   ( raw );
        case ID_GLOEPHEMERIS  :
            return decode_gloephemerisb  ( raw );
        case ID_RAWEPHEM      :
            return decode_rawephemb      ( raw );
        case ID_IONUTC        :
            return decode_ionutcb        ( raw );
    }
    return 0;
}
extern int input_ub370( raw_t *raw, unsigned char data )
{
    /* synchronize header */
    if ( raw->nbyte == 0 )
    {
        if ( sync_ub370( raw->buff, data ) )
        {
            raw->nbyte = 3;
        }
        
        return 0;
    }
    raw->buff[raw->nbyte++] = data;
    //10为何物
    if ( raw->nbyte == 10 && ( raw->len = U2( raw->buff + 8 ) + UB370HLEN ) > MAXRAWLEN - UB370CRCLEN )
    {
        raw->nbyte = 0;
        return -1;
    }
    if ( raw->nbyte < 10 || raw->nbyte < raw->len + UB370CRCLEN )
    {
        return 0;
    }
    
    raw->nbyte = 0;
    
    /* decode ub370 message */
    return decode_ub370( raw );
}

extern int input_ub370_zy( raw_t *raw, unsigned char *data, int len, int *read_len )
{
	//fprintf(stderr, "=====input_ub370_zy \n");
    data[0] = UB370SYNC1;
    data[1] = UB370SYNC2;
    data[2] = UB370SYNC3;

    *read_len = len;
    
	if ( len < 8 )
	{
		return -1;
	}
	
	raw->len = U2( data + 8 ) + UB370HLEN;
	if( raw->len > ( MAXRAWLEN - UB370CRCLEN )) 
	{
		return -1;
	}
	
    if ( len < 10 || len < raw->len + UB370CRCLEN )
    {
        fprintf( stderr, "len < 10 || len < raw->len + UB370CRCLEN\n" );
        return 0;
    }
    
    raw->nbyte = 0;
    
    memcpy( raw->buff,  data, len );
    
    /* decode ub370 message */
    return decode_ub370( raw );
}


extern int init_raw( raw_t *raw )
{
    const double lam_glo[NFREQ] = {CLIGHT / FREQ1_GLO, CLIGHT / FREQ2_GLO};
    gtime_t time0 = {0};
    obsd_t data0 = {{0}};
    eph_t  eph0 = {0, -1, -1};
    alm_t  alm0 = {0, -1};
    geph_t geph0 = {0, -1};
    seph_t seph0 = {0};
    sbsmsg_t sbsmsg0 = {0};
    lexmsg_t lexmsg0 = {0};
    int i, j, sys;
    
    trace( 3, "init_raw:\n" );

    if( !raw )
    {
		return 0;
    }

    memset( raw, 0, sizeof( raw_t ) );
    
    raw->time = raw->tobs = time0;
    raw->ephsat = 0;
    raw->sbsmsg = sbsmsg0;
    raw->msgtype[0] = '\0';
    for ( i = 0; i < MAXSAT; i++ )
    {
        for ( j = 0; j < 150  ; j++ )
        {
            raw->subfrm[i][j] = 0;
        }
        for ( j = 0; j < NFREQ; j++ )
        {
            raw->lockt[i][j] = 0.0;
        }
        for ( j = 0; j < NFREQ; j++ )
        {
            raw->halfc[i][j] = 0;
        }
        raw->icpp[i] = raw->off[i] = raw->prCA[i] = raw->dpCA[i] = 0.0;
    }
    for ( i = 0; i < MAXOBS; i++ )
    {
        raw->freqn[i] = 0;
    }
    raw->lexmsg = lexmsg0;
    raw->icpc = 0.0;
    raw->nbyte = raw->len = 0;
    raw->iod = raw->flag = raw->tbase = raw->outtype = 0;
    raw->tod = -1;
    for ( i = 0; i < MAXRAWLEN; i++ )
    {
        raw->buff[i] = 0;
    }
    raw->opt[0] = '\0';
    
    raw->obs.data = NULL;
    raw->obuf.data = NULL;
    raw->nav.eph  = NULL;
    raw->nav.alm  = NULL;
    raw->nav.geph = NULL;
    raw->nav.seph = NULL;
    //raw->nav.leaps = 0;   //tya add
    raw->nav.leaps = 17;   //zy add
    
    for ( i = 0; i < MAXSAT; i++ )
    {
        for ( j = 0; j < 3; j++ )
        {
            raw->nav.cbias[i][j] = 0.0; //tya add
        }
    }
    
    if ( !( raw->obs.data = ( obsd_t * )malloc( sizeof( obsd_t ) * MAXOBS ) ) ||
            !( raw->obuf.data = ( obsd_t * )malloc( sizeof( obsd_t ) * MAXOBS ) ) ||
            !( raw->nav.eph  = ( eph_t * )malloc( sizeof( eph_t ) * MAXSAT ) ) ||
            !( raw->nav.alm  = ( alm_t * )malloc( sizeof( alm_t ) * MAXSAT ) ) ||
            !( raw->nav.geph = ( geph_t * )malloc( sizeof( geph_t ) * NSATGLO ) ) ||
            !( raw->nav.seph = ( seph_t * )malloc( sizeof( seph_t ) * NSATSBS * 2 ) ) )
    {
        free_raw( raw );
        return 0;
    }
    raw->obs.n = 0;
    raw->obuf.n = 0;
    raw->nav.n = MAXSAT;
    raw->nav.na = MAXSAT;
    raw->nav.ng = NSATGLO;
    raw->nav.ns = NSATSBS * 2;
    for ( i = 0; i < MAXOBS   ; i++ )
    {
        raw->obs.data [i] = data0;
    }
    for ( i = 0; i < MAXOBS   ; i++ )
    {
        raw->obuf.data[i] = data0;
    }
    for ( i = 0; i < MAXSAT   ; i++ )
    {
        raw->nav.eph  [i] = eph0;
    }
    for ( i = 0; i < MAXSAT   ; i++ )
    {
        raw->nav.alm  [i] = alm0;
    }
    for ( i = 0; i < NSATGLO  ; i++ )
    {
        raw->nav.geph [i] = geph0;
    }
    for ( i = 0; i < NSATSBS * 2; i++ )
    {
        raw->nav.seph [i] = seph0;
    }
    for ( i = 0; i < MAXSAT; i++ ) for ( j = 0; j < NFREQ; j++ )
        {
            if ( !( sys = satsys( i + 1, NULL ) ) )
            {
                continue;
            }
            raw->nav.lam[i][j] = sys == SYS_GLO ? lam_glo[j] : lam_carr[j];
        }
    raw->sta.name[0] = raw->sta.marker[0] = '\0';
    raw->sta.antdes[0] = raw->sta.antsno[0] = '\0';
    raw->sta.rectype[0] = raw->sta.recver[0] = raw->sta.recsno[0] = '\0';
    raw->sta.antsetup = raw->sta.itrf = raw->sta.deltype = 0;
    for ( i = 0; i < 3; i++ )
    {
        raw->sta.pos[i] = raw->sta.del[i] = 0.0;
    }
    raw->sta.hgt = 0.0;
    raw->receive_time = 0.0; //tya add
    raw->plen = 0.0;
    raw->pbyte = 0.0;
    raw->page = 0.0;
    raw->reply = 0.0;
    raw->week = 0.0;
    //strcpy(raw->opt,"-EPHALL");//tya add
    memset( raw->pbuff, 0, sizeof( char )*RT17BUFLEN );
    return 1;
}

extern void init_raw_nav( nav_t *nav )
{

	const double lam_glo[NFREQ] = {CLIGHT / FREQ1_GLO, CLIGHT / FREQ2_GLO};
	//gtime_t time0 = {0};
	//obsd_t data0 = {{0}};
	eph_t  eph0 = {0, -1, -1};
	alm_t  alm0 = {0, -1};
	geph_t geph0 = {0, -1};
	seph_t seph0 = {0};
	//sbsmsg_t sbsmsg0 = {0};
	//lexmsg_t lexmsg0 = {0};
	int i, j, sys;

	nav->leaps = 17;   //zy add

	nav->n = MAXSAT;
    nav->na = MAXSAT;
    nav->ng = NSATGLO;
    nav->ns = NSATSBS * 2;

    for ( i = 0; i < MAXSAT   ; i++ )
    {
        nav->eph  [i] = eph0;
    }
    for ( i = 0; i < MAXSAT   ; i++ )
    {
        nav->alm  [i] = alm0;
    }
    for ( i = 0; i < NSATGLO  ; i++ )
    {
        nav->geph [i] = geph0;
    }
    for ( i = 0; i < NSATSBS * 2; i++ )
    {
        nav->seph [i] = seph0;
    }
    for ( i = 0; i < MAXSAT; i++ ) for ( j = 0; j < NFREQ; j++ )
    {
        if ( !( sys = satsys( i + 1, NULL ) ) )
        {
            continue;
        }
        nav->lam[i][j] = sys == SYS_GLO ? lam_glo[j] : lam_carr[j];
    }    
    
}

extern void free_raw( raw_t *raw )
{
    trace( 3, "free_raw:\n" );
    if( raw->obs.data != NULL )
    {
	    free( raw->obs.data );
	    raw->obs.data = NULL;
	}
	raw->obs.n = 0;
	    
	if( raw->obuf.data != NULL )
	{
	    free( raw->obuf.data );
	    raw->obuf.data = NULL;
	}
	raw->obuf.n = 0;
	
	if( raw->nav.eph != NULL )
	{
	    free( raw->nav.eph  );
	    raw->nav.eph  = NULL;
	}
	raw->nav.n = 0;
	
	if( raw->nav.alm != NULL )
	{
	    free( raw->nav.alm  );
	    raw->nav.alm  = NULL;
	}
	raw->nav.na = 0;
	
	if( raw->nav.geph != NULL )
	{
	    free( raw->nav.geph );
	    raw->nav.geph = NULL;
	}
	raw->nav.ng = 0;
	
	if( raw->nav.seph != NULL ) 
	{
	    free( raw->nav.seph );
	    raw->nav.seph = NULL;
	}
	raw->nav.ns = 0;	
}
