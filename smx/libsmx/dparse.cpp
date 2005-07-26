/* date_parse - parse string dates into internal form
**
** Copyright (C) 1995 by Jef Poskanzer <jef@netcom.com>.
** Mangled For C++    by Erik Aronesty <erik@inch.com>.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** You got it Jef -- EA
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "unix.h"

#ifndef _BZERO
	#define bzero(p, n) memset(p, 0, n)
#endif

#if defined(_WINDOWS) || defined(WIN32)
	#define CDECL _cdecl
#else
	#define CDECL
#endif


#if defined(SYSV) || defined(sun)
	extern long timezone;
#endif

#if defined(_WINDOWS) || defined(WIN32)
	#define timezone _timezone;
#endif

#ifdef _DEBUG
#define DP(str) (void) fprintf( stderr, "%s\n", str )
#else
#define DP(str) (0)
#endif

struct strint 
{
    char* s;
    int i;
};

int CDECL strint_compare( const void* v1, const void* v2 ) 
{
    return stricmp( ((struct strint*) v1)->s, ((struct strint*) v2)->s );
}

static int strint_search(char* str, struct strint* tab, int n, int* iP)
{
    int i, h, l, r;
    l = 0;
    h = n - 1;
    for (;;) {
		i = ( h + l ) / 2;
		r = stricmp( str, tab[i].s );
		if ( r < 0 )
			h = i - 1;
		else if ( r > 0 )
			l = i + 1;
		else
			{
			*iP = tab[i].i;
			return 1;
			}
		if ( h < l )
			return 0;
	}
}

#define AMPM_NONE 0
#define AMPM_AM 1
#define AMPM_PM 2
static int ampm_fix( int hour, int ampm )
{
    switch ( ampm )	{
	case AMPM_NONE:
	break;
	case AMPM_AM:
		if ( hour == 12 )
			hour = 0;
	break;
	case AMPM_PM:
		if ( hour != 12 )
			hour += 12;
	break;
	}
    return hour;
}

static int scan_ampm( char* str_ampm, int* ampmP )
{
    static struct strint ampm_tab[] = 
	{
		{ "am", AMPM_AM }, { "pm", AMPM_PM },
		{ "a" , AMPM_AM }, { "p" , AMPM_PM }
	};

    static int sorted = 0;

    if ( ! sorted ) {
		(void) qsort(
			ampm_tab, sizeof(ampm_tab)/sizeof(struct strint),
			sizeof(struct strint), strint_compare );
		sorted = 1;
	}
    
    return strint_search( 
		str_ampm, ampm_tab, sizeof(ampm_tab)/sizeof(struct strint), ampmP );
}

static int scan_wday( char* str_wday, int* tm_wdayP )
{
    static struct strint wday_tab[] = {
	{ "sun", 0 }, { "sunday", 0 },
	{ "mon", 1 }, { "monday", 1 },
	{ "tue", 2 }, { "tuesday", 2 },
	{ "wed", 3 }, { "wednesday", 3 },
	{ "thu", 4 }, { "thursday", 4 },
	{ "fri", 5 }, { "friday", 5 },
	{ "sat", 6 }, { "saturday", 6 },
	};
    static int sorted = 0;

    if ( ! sorted )
	{
	(void) qsort(
	    wday_tab, sizeof(wday_tab)/sizeof(struct strint),
	    sizeof(struct strint), strint_compare );
	sorted = 1;
	}
    
    return strint_search( 
	str_wday, wday_tab, sizeof(wday_tab)/sizeof(struct strint), tm_wdayP );
}

static int scan_mon( char* str_mon, int* tm_monP )
{
    static struct strint mon_tab[] = 
	{
		{ "jan", 0 }, { "january", 0 },
		{ "feb", 1 }, { "february", 1 },
		{ "mar", 2 }, { "march", 2 },
		{ "apr", 3 }, { "april", 3 },
		{ "may", 4 },
		{ "jun", 5 }, { "june", 5 },
		{ "jul", 6 }, { "july", 6 },
		{ "aug", 7 }, { "august", 7 },
		{ "sep", 8 }, { "september", 8 },
		{ "oct", 9 }, { "october", 9 },
		{ "nov", 10 }, { "november", 10 },
		{ "dec", 11 }, { "december", 11 },
	};
    static int sorted = 0;

    if ( ! sorted )
	{
	(void) qsort(
	    mon_tab, sizeof(mon_tab)/sizeof(struct strint),
	    sizeof(struct strint), strint_compare );
	sorted = 1;
	}
    
    return strint_search( 
	str_mon, mon_tab, sizeof(mon_tab)/sizeof(struct strint), tm_monP );
    }

static int scan_gmtoff( char* str_gmtoff, int* gmtoffP )
{
	static struct strint gmtoff_tab[] = 
	{
		{ "gmt", 0 }, { "utc", 0 }, { "ut", 0 },
		{ "0000", 0 }, { "+0000", 0 }, { "-0000", 0 },
		{ "0100", 3600 }, { "+0100", 3600 }, { "-0100", -3600 },
		{ "0200", 7200 }, { "+0200", 7200 }, { "-0200", -7200 },
		{ "0300", 10800 }, { "+0300", 10800 }, { "-0300", -10800 },
		{ "0400", 14400 }, { "+0400", 14400 }, { "-0400", -14400 },
		{ "0500", 18000 }, { "+0500", 18000 }, { "-0500", -18000 },
		{ "0600", 21600 }, { "+0600", 21600 }, { "-0600", -21600 },
		{ "0700", 25200 }, { "+0700", 25200 }, { "-0700", -25200 },
		{ "0800", 28800 }, { "+0800", 28800 }, { "-0800", -28800 },
		{ "0900", 32400 }, { "+0900", 32400 }, { "-0900", -32400 },
		{ "1000", 36000 }, { "+1000", 36000 }, { "-1000", -36000 },
		{ "1100", 39600 }, { "+1100", 39600 }, { "-1100", -39600 },
		{ "1200", 43200 }, { "+1200", 43200 }, { "-1200", -43200 },
		{ "jst", 7200 }, { "jdt", 10800 },
		{ "bst", -3600 },
		{ "nst", -12600 },
		{ "ast", -14400 }, { "edt", -10800 },
		{ "est", -18000 }, { "edt", -14400 },
		{ "cst", -21600 }, { "cdt", -18000 },
		{ "mst", -25200 }, { "mdt", -21600 },
		{ "pst", -28800 }, { "pdt", -25200 },
		{ "yst", -32400 }, { "ydt", -28800 },
		{ "hst", -36000 }, { "hdt", -32400 },
		{ "a", -3600 }, { "b", -7200 }, { "c", -10800 }, { "d", -14400 },
		{ "e", -18000 }, { "f", -21600 }, { "g", -25200 }, { "h", -28800 },
		{ "i", -32400 }, { "k", -36000 }, { "l", -39600 }, { "m", -43200 },
		{ "n", -3600 }, { "o", -7200 }, { "p", -10800 }, { "q", -14400 },
		{ "r", -18000 }, { "s", -21600 }, { "t", -25200 }, { "u", -28800 },
		{ "v", -32400 }, { "w", -36000 }, { "x", -39600 }, { "y", -43200 },
	};
    static int sorted = 0;

    if ( ! sorted )
	{
	(void) qsort(
	    gmtoff_tab, sizeof(gmtoff_tab)/sizeof(struct strint),
	    sizeof(struct strint), strint_compare );
	sorted = 1;
	}
    
    return strint_search( 
	str_gmtoff, gmtoff_tab, sizeof(gmtoff_tab)/sizeof(struct strint),
	gmtoffP );
}

static int mm_fix( int mm )
{
    return mm - 1;
}

static int is_leap( int year )
{
    return year % 400? ( year % 100 ? ( year % 4 ? 0 : 1 ) : 0 ) : 1;
}

/* Basically the same as mktime(). */
static time_t tm_to_time( struct tm* tmP )
{
    time_t t;
    static int monthtab[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    /* Years since epoch, converted to days. */
    t = ( tmP->tm_year - 70 ) * 365;
    /* Leap days for previous years. */
    t += ( tmP->tm_year - 68 ) / 4;
    /* Days for the beginning of this month. */
    t += monthtab[tmP->tm_mon];
    /* Leap day for this year. */
    if ( tmP->tm_mon >= 2 && is_leap( tmP->tm_year ) )
	++t;
    /* Days since the beginning of this month. */
    t += tmP->tm_mday - 1;	/* 1-based field */
    /* Hours, minutes, and seconds. */
    t = t * 24 + tmP->tm_hour;
    t = t * 60 + tmP->tm_min;
    t = t * 60 + tmP->tm_sec;

    return t;
}

time_t date_parse(const char *str)
{
    time_t now;
    struct tm* now_tmP;
    struct tm tm;
    const char* cp;
    char str_mon[500], str_wday[500], str_gmtoff[500], str_ampm[500];
    int tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, gmtoff, local_gmtoff;
    int ampm, got_zone;
    time_t t;

    /* Initialize tm with relevant parts of current local time. */
    now = time( (time_t*) 0 );
    now_tmP = localtime( &now );

    bzero( (char*) &tm, sizeof(struct tm) );
    tm.tm_sec = now_tmP->tm_sec;
    tm.tm_min = now_tmP->tm_min;
    tm.tm_hour = now_tmP->tm_hour;
    tm.tm_mday = now_tmP->tm_mday;
    tm.tm_mon = now_tmP->tm_mon;
    tm.tm_year = now_tmP->tm_year;
    ampm = AMPM_NONE;
    got_zone = 0;

    /* Find local zone offset.  This is the only real area of
    ** non-portability, and it's only used for local times that don't
    ** specify a zone - those don't occur in email and netnews.
    */
#if defined(SYSV) || defined(WIN32) || defined(sun)
    tzset();
    gmtoff = -timezone;
#else /* SYSV */
#ifdef BSD
    gmtoff = now_tmP->tm_gmtoff;
#else /* BSD */
    /* You have to fill this in yourself. */
//    gmtoff = !!!;
    tzset();
    gmtoff = -timezone;

#endif /* BSD */
#endif /* SYSV */

	local_gmtoff = gmtoff;

    /* Skip initial whitespace ourselves - sscanf is clumsy at this. */
    for ( cp = str; *cp == ' ' || *cp == '\t'; ++cp )
	;

    /* And do the sscanfs.  WARNING: you can add more formats here,
    ** but be careful!  You can easily screw up the parsing of existing
    ** formats when you add new ones.
    */

    /* N mth YYYY HH:MM:SS ampm zone */
    if ( ( ( sscanf( cp, "%d %[a-zA-Z] %d %d:%d:%d %[apmAPM] %[^: ]",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
            sscanf( cp, "%d %[a-zA-Z] %d %d:%d:%d %[^: ]",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 ) &&
	    scan_mon( str_mon, &tm_mon ) &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) )
	{
	DP( "N mth YYYY HH:MM:SS ampm zone" );
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	tm.tm_hour = ampm_fix( tm_hour, ampm );
	tm.tm_min = tm_min;
	tm.tm_sec = tm_sec;
	got_zone = 1;
	}
    /* N mth YYYY HH:MM ampm zone */
    else if ( ( ( sscanf( cp, "%d %[a-zA-Z] %d %d:%d %[apmAPM] %[^: ]",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
            sscanf( cp, "%d %[a-zA-Z] %d %d:%d %[^: ]",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,	str_gmtoff ) == 6 ) &&
	    scan_mon( str_mon, &tm_mon ) &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) )
	{
	DP( "N mth YYYY HH:MM ampm zone" );
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	tm.tm_hour = ampm_fix( tm_hour, ampm );
	tm.tm_min = tm_min;
	tm.tm_sec = 0;
	got_zone = 1;
	}
    /* N mth YYYY HH:MM:SS ampm */
    else if ( ( ( sscanf( cp, "%d %[a-zA-Z] %d %d:%d:%d %[apmAPM]",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
            sscanf( cp, "%d %[a-zA-Z] %d %d:%d:%d",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,	&tm_sec ) == 6 ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	DP( "N mth YYYY HH:MM:SS ampm" );
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	tm.tm_hour = ampm_fix( tm_hour, ampm );
	tm.tm_min = tm_min;
	tm.tm_sec = tm_sec;
	}
    /* N mth YYYY HH:MM ampm */
    else if ( ( ( sscanf( cp, "%d %[a-zA-Z] %d %d:%d %[apmAPM]",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 &&
		scan_ampm( str_ampm, &ampm ) ) ||
            sscanf( cp, "%d %[a-zA-Z] %d %d:%d",
		&tm_mday, str_mon, &tm_year, &tm_hour, &tm_min ) == 5 ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	DP( "N mth YYYY HH:MM ampm" );
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	tm.tm_hour = ampm_fix( tm_hour, ampm );
	tm.tm_min = tm_min;
	tm.tm_sec = 0;
	}
    /* mm/dd/yy[yy] HH:MM:SS [ampm] [zone]*/
    else if (
			( sscanf( cp, "%d/%d/%d %d:%d:%d %[apmAPM] %[^: ]",
		&tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8 &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) && scan_ampm( str_ampm, &ampm ) && (got_zone = 1))
		||
			( sscanf( cp, "%d/%d/%d %d:%d:%d %[apmAPM]",
			&tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 &&
			scan_ampm( str_ampm, &ampm ) )
		||
			( sscanf( cp, "%d/%d/%d %d:%d:%d",
		&tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 6 )
			)
	{
	DP( "mm/dd/yy[yy] HH:MM:SS [ampm] [zone]" );
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	tm.tm_hour = ampm_fix( tm_hour, ampm );
	tm.tm_min = tm_min;
	tm.tm_sec = tm_sec;
	}
    /* mm/dd/yy[yy] HH:MM [ampm] [zone]*/
    else if (
		( sscanf( cp, "%d/%d/%d %d:%d %[apmAPM] %[^: ]",
		&tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) && scan_ampm( str_ampm, &ampm ) && (got_zone = 1) )
		||
		( sscanf( cp, "%d/%d/%d %d:%d %[apmAPM]",
		&tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_ampm ) == 6 &&
	    scan_ampm( str_ampm, &ampm ) ) 
		||
		( sscanf( cp, "%d/%d/%d %d:%d %[^: ]",
		&tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min, str_gmtoff ) == 6 &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) && (got_zone = 1) ) 
		||
        ( sscanf( cp, "%d/%d/%d %d:%d",
		&tm_mon, &tm_mday, &tm_year, &tm_hour, &tm_min ) == 5 ) 
		)
	{
	DP( "mm/dd/yy[yy] HH:MM [ampm] [zone]" );
	tm.tm_mday = tm_mday;
	tm.tm_mon = mm_fix(tm_mon);
	tm.tm_year = tm_year;
	tm.tm_hour = ampm_fix( tm_hour, ampm );
	tm.tm_min = tm_min;
	tm.tm_sec = 0;
	}
    /* yy[yy]-mm-dd hh:nn:ss [ampm] [zone]*/
    else if (
		( sscanf( cp, "%d-%d-%d %d:%d:%d %[apmAPM] %[^: ]",
		&tm_year, &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff ) == 8 &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) && scan_ampm( str_ampm, &ampm ) && (got_zone = 1) )
		||
		( sscanf( cp, "%d-%d-%d %d:%d:%d %[apmAPM]",
		&tm_year, &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_ampm ) == 7 &&
	    scan_ampm( str_ampm, &ampm ) ) 
		||
		( sscanf( cp, "%d-%d-%d %d:%d:%d %[^: ]",
		&tm_year, &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec, str_gmtoff ) == 7 &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) && (got_zone = 1) ) 
		||
        ( sscanf( cp, "%d-%d-%d %d:%d:%d",
		&tm_year, &tm_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec ) == 6 ) 
		)
	{
		DP( "yy[yy]-mm-dd hh:nn:ss [ampm] [zone]" );
		tm.tm_mday = tm_mday;
		tm.tm_mon = mm_fix(tm_mon);
		tm.tm_year = tm_year;
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
	}
    /* yy[yy]-mm-dd hh:nn [ampm] [zone]*/
    else if (
		( sscanf( cp, "%d-%d-%d %d:%d %[apmAPM] %[^: ]",
		&tm_year, &tm_mon, &tm_mday, &tm_hour, &tm_min, str_ampm, str_gmtoff ) == 7 &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) && scan_ampm( str_ampm, &ampm ) && (got_zone = 1) )
		||
		( sscanf( cp, "%d-%d-%d %d:%d %[apmAPM]",
		&tm_year, &tm_mon, &tm_mday, &tm_hour, &tm_min, str_ampm ) == 6 &&
	    scan_ampm( str_ampm, &ampm ) ) 
		||
		( sscanf( cp, "%d-%d-%d %d:%d %[^: ]",
		&tm_year, &tm_mon, &tm_mday, &tm_hour, &tm_min, str_gmtoff ) == 6 &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) && (got_zone = 1) ) 
		||
        ( sscanf( cp, "%d-%d-%d %d:%d",
		&tm_year, &tm_mon, &tm_mday, &tm_hour, &tm_min) == 5 ) 
		)
	{
		DP( "yy[yy]-mm-dd hh:nn:ss [ampm] [zone]" );
		tm.tm_mday = tm_mday;
		tm.tm_mon = mm_fix(tm_mon);
		tm.tm_year = tm_year;
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = 0;
	}
	/* mm/dd/yy[yy]*/
    else if ( sscanf( cp, "%d/%d/%d", &tm_mon, &tm_mday, &tm_year) == 3 ) {
		DP( "mm/dd/yy[yy]" );
		tm.tm_mday = tm_mday;
		tm.tm_mon = mm_fix(tm_mon);
		tm.tm_year = tm_year;
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
	}
	/*yy[yy]-mm-dd*/
    else if ( sscanf( cp, "%d-%d-%d", &tm_year, &tm_mon, &tm_mday) == 3 ) {
		DP( "yy[yy]-mm-dd" );
		tm.tm_mday = tm_mday;
		tm.tm_mon = mm_fix(tm_mon);
		tm.tm_year = tm_year;
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
	}	/* mm/dd*/
    else if ( sscanf( cp, "%d/%d %d:%d %[apmAPM]", &tm_mon, &tm_mday, &tm_hour, &tm_min, str_ampm) == 5  && scan_ampm( str_ampm, &ampm ) ) {
		DP( "mm/dd hh:ss aa" );
		tm.tm_mday = tm_mday;
		tm.tm_mon = mm_fix(tm_mon);
		//  tm.tm_year = same as it was;
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = 0;
		tm.tm_sec = 0;
	}
    else if ( sscanf( cp, "%d/%d %d:%d", &tm_mon, &tm_mday, &tm_hour, &tm_min) == 4 ) {
		DP( "mm/dd hh:ss" );
		tm.tm_mday = tm_mday;
		tm.tm_mon = mm_fix(tm_mon);
		//  tm.tm_year = same as it was;
		tm.tm_hour = tm_hour;
		tm.tm_min = tm_min;
		tm.tm_sec = 0;
	}
    else if ( sscanf( cp, "%d/%d", &tm_mon, &tm_mday) == 2 ) {
		DP( "mm/dd" );
		tm.tm_mday = tm_mday;
		tm.tm_mon = mm_fix(tm_mon);
		//  tm.tm_year = same as it was;
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
	}
    /* HH:MM:SS ampm zone N mth YYYY */
    else if ( ( ( sscanf( cp, "%d:%d:%d %[apmAPM] %[^: ] %d %[a-zA-Z] %d",
		&tm_hour, &tm_min, &tm_sec, str_ampm, str_gmtoff, &tm_mday,
		str_mon, &tm_year ) == 8 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%d:%d:%d %[^: ] %d %[a-zA-Z] %d",
		&tm_hour, &tm_min, &tm_sec, str_gmtoff, &tm_mday, str_mon,
		&tm_year ) == 7 ) &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
		DP( "HH:MM:SS ampm zone N mth YYYY" );
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
	}
    /* HH:MM ampm zone N mth YYYY */
    else if ( ( ( sscanf( cp, "%d:%d %[apmAPM] %[^: ] %d %[a-zA-Z] %d",
		&tm_hour, &tm_min, str_ampm, str_gmtoff, &tm_mday, str_mon,
		&tm_year ) == 7 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%d:%d %[^: ] %d %[a-zA-Z] %d",
		&tm_hour, &tm_min, str_gmtoff, &tm_mday, str_mon,
		&tm_year ) == 6 ) &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	DP( "HH:MM ampm N mth YYYY" );
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = 0;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
	}
    /* HH:MM:SS ampm N mth YYYY */
    else if ( ( ( sscanf( cp, "%d:%d:%d %[apmAPM] %d %[a-zA-Z] %d",
		&tm_hour, &tm_min, &tm_sec, str_ampm, &tm_mday, str_mon,
		&tm_year ) == 7 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%d:%d:%d %d %[a-zA-Z] %d",
		&tm_hour, &tm_min, &tm_sec, &tm_mday, str_mon,
		&tm_year ) == 6 ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	DP( "HH:MM:SS ampm N mth YYYY" );
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
	}
    /* HH:MM ampm N mth YYYY */
    else if ( ( ( sscanf( cp, "%d:%d %[apmAPM] %d %[a-zA-Z] %d",
		&tm_hour, &tm_min, str_ampm, &tm_mday, str_mon,
		&tm_year ) == 6 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%d:%d %d %[a-zA-Z] %d",
		&tm_hour, &tm_min, &tm_mday, str_mon, &tm_year ) == 5 ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
		DP( "HH:MM ampm N mth YYYY" );
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = 0;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
	}
    /* wdy, N mth YYYY HH:MM:SS ampm zone */
    else if ( ( ( sscanf( cp, "%[a-zA-Z], %d %[a-zA-Z] %d %d:%d:%d %[apmAPM] %[^: ]",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec, str_ampm, str_gmtoff ) == 9 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%[a-zA-Z], %d %[a-zA-Z] %d %d:%d:%d %[^: ]",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec, str_gmtoff ) == 8 ) &&
	    scan_wday( str_wday, &tm_wday ) &&
	    scan_mon( str_mon, &tm_mon ) &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) )
	{
		DP( "wdy, N mth YYYY HH:MM:SS ampm zone" );
		tm.tm_wday = tm_wday;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
		got_zone = 1;
	}
    /* wdy, N mth YYYY HH:MM ampm zone */
    else if ( ( ( sscanf( cp, "%[a-zA-Z], %d %[a-zA-Z] %d %d:%d %[apmAPM] %[^: ]",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		str_ampm, str_gmtoff ) == 8 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%[a-zA-Z], %d %[a-zA-Z] %d %d:%d %[^: ]",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		str_gmtoff ) == 7 ) &&
	    scan_wday( str_wday, &tm_wday ) &&
	    scan_mon( str_mon, &tm_mon ) &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) )
	{
		DP( "wdy, N mth YYYY HH:MM ampm zone" );
		tm.tm_wday = tm_wday;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = 0;
		got_zone = 1;
	}
    /* wdy, N mth YYYY HH:MM:SS ampm */
    else if ( ( ( sscanf( cp, "%[a-zA-Z], %d %[a-zA-Z] %d %d:%d:%d %[apmAPM]",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec, str_ampm ) == 8 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%[a-zA-Z], %d %[a-zA-Z] %d %d:%d:%d",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		&tm_sec ) == 7 ) &&
	    scan_wday( str_wday, &tm_wday ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
		DP( "wdy, N mth YYYY HH:MM:SS ampm" );
		tm.tm_wday = tm_wday;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
	}
    /* wdy, N mth YYYY HH:MM ampm */
    else if ( ( ( sscanf( cp, "%[a-zA-Z], %d %[a-zA-Z] %d %d:%d %[apmAPM]",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min,
		str_ampm ) == 7 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%[a-zA-Z], %d %[a-zA-Z] %d %d:%d",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour,
		&tm_min ) == 6 ) &&
	    scan_wday( str_wday, &tm_wday ) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
		DP( "wdy, N mth YYYY HH:MM ampm" );
		tm.tm_wday = tm_wday;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = 0;
	}
    /* wdy mth N HH:MM:SS ampm zone YYYY */
    else if ( ( ( sscanf( cp, "%[a-zA-Z] %[a-zA-Z] %d %d:%d:%d %[apmAPM] %[^: ] %d",
		str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec,
		str_ampm, str_gmtoff, &tm_year ) == 9 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%[a-zA-Z] %[a-zA-Z] %d %d:%d:%d %[^: ] %d",
		str_wday, str_mon, &tm_mday, &tm_hour, &tm_min, &tm_sec,
		str_gmtoff, &tm_year ) == 8 ) &&
	    scan_wday( str_wday, &tm_wday ) &&
	    scan_mon( str_mon, &tm_mon ) &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) )
	{
		DP( "wdy mth N HH:MM:SS ampm zone YYYY" );
		tm.tm_wday = tm_wday;
		tm.tm_mon = tm_mon;
		tm.tm_mday = tm_mday;
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
		got_zone = 1;
		tm.tm_year = tm_year;
	}
    /* wdy mth N HH:MM ampm zone YYYY */
    else if ( ( ( sscanf( cp, "%[a-zA-Z] %[a-zA-Z] %d %d:%d %[apmAPM] %[^: ] %d",
	    str_wday, str_mon, &tm_mday, &tm_hour, &tm_min,
	    str_ampm, str_gmtoff, &tm_year ) == 8 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%[a-zA-Z] %[a-zA-Z] %d %d:%d %[^: ] %d",
		str_wday, str_mon, &tm_mday, &tm_hour, &tm_min,
		str_gmtoff, &tm_year ) == 7 ) &&
	    scan_wday( str_wday, &tm_wday ) &&
	    scan_mon( str_mon, &tm_mon ) &&
	    scan_gmtoff( str_gmtoff, &gmtoff ) )
	{
		DP( "wdy mth N HH:MM ampm zone YYYY" );
		tm.tm_wday = tm_wday;
		tm.tm_mon = tm_mon;
		tm.tm_mday = tm_mday;
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = 0;
		got_zone = 1;
		tm.tm_year = tm_year;
	}
    /* N mth YY[YY] */
    else if ( sscanf( cp, "%d %[a-zA-Z] %d",
	    &tm_mday, str_mon, &tm_year ) == 3 &&
	    scan_mon( str_mon, &tm_mon ) )
	{
	DP( "N mth YYYY" );
	tm.tm_mday = tm_mday;
	tm.tm_mon = tm_mon;
	tm.tm_year = tm_year;
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;
	}
    /* mth N [,] YY[YY] */
    else if ( (sscanf( cp, "%[a-zA-Z] %d %d",
			str_mon, &tm_mday, &tm_year ) == 3 || 
			   sscanf( cp, "%[a-zA-Z] %d , %d",
			str_mon, &tm_mday, &tm_year ) == 3)
		&&
	    scan_mon( str_mon, &tm_mon ) )
	{
		DP( "N mth YYYY" );
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
	}
    /* mth [,] YY[YY] */
    else if ( (
			sscanf( cp, "%[a-zA-Z] %d", str_mon, &tm_year ) == 2
			||
			sscanf( cp, "%[a-zA-Z] , %d", str_mon, &tm_year ) == 2
		) &&
	    scan_mon( str_mon, &tm_mon ) )
	{
		DP( "mth [,] YY[YY]" );
		tm.tm_mday = 1;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
	}
    /* mth*/
    else if ( sscanf( cp, "%[a-zA-Z]", str_mon) == 1 && 
		scan_mon( str_mon, &tm_mon ) )
	{
		DP( "mth" );
		tm.tm_mday = 1;
		tm.tm_mon = tm_mon;
		//  tm.tm_year = same as it was;
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
	}
    /* HH:MM:SS ampm */
    else if ( ( sscanf( cp, "%d:%d:%d %[apmAPM]",
	    &tm_hour, &tm_min, &tm_sec, str_ampm ) == 4 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%d:%d:%d", &tm_hour, &tm_min, &tm_sec ) == 3 )
	{
		DP( "HH:MM:SS ampm" );
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
	}
    /* HH:MM ampm */
    else if ( ( sscanf( cp, "%d:%d %[apmAPM]", &tm_hour, &tm_min,
		str_ampm ) == 3 &&
	    scan_ampm( str_ampm, &ampm ) ) ||
	    sscanf( cp, "%d:%d", &tm_hour, &tm_min ) == 2 )
	{
		DP( "HH:MM" );
		tm.tm_hour = ampm_fix( tm_hour, ampm );
		tm.tm_min = tm_min;
		tm.tm_sec = 0;
	}
    /* yymmdd */
    else if ( (sscanf( cp, "%d", &tm_year) == 1) && 
			(tm_year % 100 > 0) &&
			(tm_year % 100 <= 31) &&
			((tm_year / 100) % 100 > 0) &&
			((tm_year / 100) % 100 <= 12) &&
			(strlen(cp) == 6) )
	{
		DP( "yymmdd" );
		tm.tm_mday = (int) (tm_mon % 100);
		tm.tm_mon =  mm_fix((int) (tm_mon / 100) % 100);
		tm.tm_year = (int) (tm_year / 10000);
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
    }
	else if ( (sscanf( cp, "%d", &tm_year) == 1) && 
			(tm_year % 100 > 0) &&
			(tm_year % 100 <= 31) &&
			((tm_year / 100) % 100 > 0) &&
			((tm_year / 100) % 100 <= 12) &&
			(strlen(cp) == 8) )
	{
		DP( "yyyymmdd" );
		tm.tm_mday = (int) (tm_mon % 100);
		tm.tm_mon =  mm_fix((int) (tm_mon / 100) % 100);
		tm.tm_year = (int) (tm_year / 10000);
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
	}
    else
	return (time_t) -1;

	// solve yNk bug.... 2 digit year = closest to now
	static int yk_low = -1;
	static int yk_off = 0;
	if (yk_low < 0) {
		yk_low = ((now_tmP->tm_year + 50) % 100);
		yk_off = 100 * (int) ((now_tmP->tm_year-50) / 100);
	}

    if ( tm.tm_year > 1900 )
		tm.tm_year -= 1900;
    else {
		if ( tm.tm_year < yk_low )
			tm.tm_year += 100;
		tm.tm_year += yk_off;
	}

	tm.tm_isdst = -1;
    t = mktime(&tm);
	t += (local_gmtoff - gmtoff);

    return t;
}
