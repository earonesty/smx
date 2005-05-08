/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _QSCRIPT_STDINC_H
#define _QSCRIPT_STDINC_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#if defined(SYSV) || defined(sun)
	extern long timezone;
#endif

#if defined(_WINDOWS) || defined(WIN32)
	#define timezone _timezone;
#endif

#ifdef _DEBUG 
	#ifdef WIN32
		#define _AFX_NOFORCE_LIBS
		#include <afxwin.h>
		#undef _AFX_NOFORCE_LIBS
		#undef TRACE
	#else
		#include "unix.h"
	#endif

	inline void TRACE(char * format, ...)
	{
		va_list args;
		va_start(args, format);

		unsigned int n;
		char buf[512];

		n = _vsnprintf(buf, sizeof(buf), format, args);
		assert(n < sizeof(buf)); // output was trunc'd
		OutputDebugStringA(buf);
		va_end(args);
	}

#else
	#ifdef WIN32
		#include <windows.h>
	#else
		#include "unix.h"
	#endif

	#undef TRACE
	#define TRACE
#endif

#define _Q_NOFORCE_LIBS

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "str.h"
#include "map.h"
#include "ary.h"
#include "lst.h"
#include "ex.h"
#include "crit.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#endif // #ifndef _QSCRIPT_STDINC_H
