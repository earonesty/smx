/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

#endif // #ifndef _QSCRIPT_STDINC_H
