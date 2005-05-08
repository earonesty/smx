/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

// 
// Straightforward date_parse function
//
// Dies in the year 2037
// Need to fix so as NOT to use time_t
// Rather use a double-precision #of leap-days since '0 AD' method
//

#ifndef _qlib_h_
	#include "qlib.h"
#endif

#ifndef time_t
	#include <time.h>
#endif

#define DATE_NULL -1
#define DATE_VALID(t) (t > 0)
#define DATEV time_t
#define time_to_tm localtime
DATEV date_parse(const char *str);
//struct tm *time_to_tm( const DATEV *timer );
