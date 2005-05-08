#ifndef _DEBUG_H

#include <windows.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <crtdbg.h>

#ifdef _DEBUG
	inline void TRACE(char * format, ...)
	{
		va_list args;
		va_start(args, format);

		int n;
		char buf[512];

		n = _vsnprintf(buf, sizeof(buf), format, args);
		assert(n < sizeof(buf)); // output was trunc'd
		OutputDebugStringA(buf);
		va_end(args);
	}
#else
	inline void TRACE(char * format, ...) {}
#endif

#endif // #ifndef _DEBUG_H