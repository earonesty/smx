/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _QLIB_H
#define _QLIB_H

#ifdef unix
	#include "unix.h"
#endif

#ifndef BUFFER_GROW
	#define BUFFER_GROW 32
#endif

#ifndef UCHAR
	typedef unsigned char UCHAR;
#endif

#if defined(__GNUC__)
#  if __GNUC__ < 3 && __GNUC__ >= 2 && __GNUC_MINOR__ >= 95
#     include <hash_map>
#  elif __GNUC__ >= 3
#     include <ext/hash_map>
#  else
#     include <hash_map.h>
#  endif
#elif defined(__MSVC_VER__)
#  include <hash_map>
#elif defined(__sgi__)
#  include <hash_map>
#else
#  include <hash_map>
#endif

#ifndef _Q_NOFORCE_LIBS
#ifdef WIN32
	#ifdef _DEBUG
		#pragma comment(lib, "qlibd.lib")
	#else
		#pragma comment(lib, "qlib.lib")
	#endif
#endif
#endif

#ifdef _MT
	#define TLS __declspec(thread)
#else
	#define TLS 
#endif

#if defined(WIN32) || defined(DOS)
	#ifndef _INC_STDLIB
		#include <stdlib.h>
		#define _INC_STDLIB
	#endif
	#ifndef _INC_CTYPE
		#include <ctype.h>
		#define _INC_CTYPE
	#endif
#endif

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#endif  //#ifndef _QLIB_H
