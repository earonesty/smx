/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/
#ifdef unix

#include "stdafx.h"
#include "crit_unix.h"
// Windows to UNIX compatibility library.

#include <stdio.h>

char CMutex::s_mutex_path[] = "/tmp/smx_mutex.";

#endif

