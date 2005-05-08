/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _QFOPEN_
#define _QFOPEN_

// Copyright (C) 1999 by Prime Data Corp. All Rights Reserved.
// For information contact erik@primedata.org

#ifdef WIN32
	#include <io.h>
	#include <process.h>
#endif

#if defined(_WINDOWS) || defined(WIN32)
	#define fix_path(f) bslash(f)
	#define SHARED_LIB_EXT ".dll"
#else
	#define fix_path(f) fslash(f)
	#define SHARED_LIB_EXT ".so"
#endif

#if defined(_WINDOWS) || defined(WIN32)

#define ConvWin32Time(t) \
		(time_t) ((((ULARGE_INTEGER&)t).QuadPart-116444736000000000)/10000000)

#endif

time_t GetFileModified(const FILE *fp);

qStr *OpenURL(qCtx *ctx, const CStr &path);
qStr *OpenFileFQ(qCtx *ctx, const char *p);
qStr *OpenFile(qCtx *ctx, CStr &path);
void ResolvePath(qCtx *ctx, CStr &path);

#endif //#ifndef _QFOPEN_
