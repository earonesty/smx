/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _PSTIME_H
#define _PSTIME_H

#include <time.h>

class qStr;

class qTime {
	time_t myTime;
public:
	qTime()
		{myTime = 0;}

	operator time_t &() 
		{return (myTime);}

	qTime & operator = (time_t t) 
		{myTime = t; return *this;}

	int Compare(time_t time) 
		{return (myTime > time ? 1 : myTime < time ? -1 : 0);}

	static long GetZoneOffset()
		{return timezone;}
};


void FmtTime(struct tm * tms, const char *p, qStr *out);


#endif // #ifndef _PSTIME_H
