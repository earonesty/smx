/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _STR_H
#define _STR_H

#ifndef _INC_STDARG
	#include <stdarg.h>
	#define _INC_STDARG
#endif

#ifndef _INC_STDIO
	#include <stdio.h>
	#define _INC_STDIO
#endif

#ifndef _INC_STRING
	#include <string.h>
	#define _INC_STRING
#endif

#ifndef _INC_CTYPE
	#include <ctype.h>
	#define _INC_CTYPE
#endif

#ifndef _BUF_H
	#include "buf.h"
#endif

// simple string class, uses CBuf
class CStr : public CBuf<char> {
public:
	int Length() const	{return CBuf<char>::Count() ? CBuf<char>::Count() - 1 : 0;}

	CStr(int n = 0) \
		{Grow(n);}

	CStr(const char *d) \
		{if (d) {int n = strlen(d); Grow(n); memcpy(m_buf, d, n);}}

  	CStr(char c, int n) \
		{Grow(n); memset(m_buf, c, n);}

	CStr(const char *d, int n) \
		{Grow(n); memcpy(m_buf, d, n);}

	CStr &operator =(const char *d) \
		{if (d) {int n = strlen(d); CVBuf::Grow(n+1); memcpy(m_buf, d, n); ((char *)m_buf)[n] = '\0';} else Grow(0); return *this;}

	CStr &operator +=(const char *d) \
		{if (d) {int n = strlen(d); return Append(d, n); } return *this;}

	CStr &operator +=(char d) \
		{int o = Length(); Grow(o + 1); Data()[o] = d; return *this;}

	CStr &operator +=(const CStr &d) \
		{if (d.Length()) {int o = Length(); Grow(o + d.Length()); memcpy(Data() + o, (const char *) d, d.Length());} return *this;}

	CStr operator +(const char *d) \
		{return CStr(*this) += d;}

	CStr operator +(char d) \
		{return CStr(*this) += d;}

	CStr operator +(const CStr &d) \
		{return CStr(*this) += d;}

	char *Grow(int n) \
		{CVBuf::Grow(n+1);  ((char *)m_buf)[n] = '\0'; return Data();}

	CStr &Append(const char *d, int n) \
		{int o = Length(); Grow(o + n); memcpy(Data() + o, d, n); return *this;}

// helpers *** note it is assumed that memcpy has to work here ... memmove is correct for some optimizers
	CStr LTrim()
		{ if (Length() > 0) { char *p = ((char*)m_buf); while(isspace(*p)) ++p; SetCount(Count() - (p-m_buf)); memcpy(m_buf, p, Count()); } return *this; }
	CStr RTrim()
		{ if (Length() > 0) { char *p = ((char*)m_buf) + Length() - 1; while(p >= m_buf && isspace(*p)) --p; *++p = 0; SetCount(p - m_buf + 1); } return *this; }
	CStr Trim()
		{LTrim(); return RTrim();}
	CStr Shrink()
		{if (Length() > 0) {Grow(strlen((char *)m_buf));} return *this;}

	CStr Format(const char *format, ...) { \
		va_list marker; va_start( marker, format); \
		Grow(_vsnprintf(Grow(Length()+1024), Length()+1024, format, marker)); \
		va_end( marker ); return *this; \
	}
	static CStr &Null;
};


// helpers in "strx.cpp"
CStr operator +(CStr left, int right);
CStr operator +(CStr left, long right);
CStr operator +(CStr left, float right);
CStr operator +(CStr left, double right);
CStr operator +(const char *left, CStr right);

CStr &operator +=(CStr &left, int right);
CStr &operator +=(CStr &left, long right);
CStr &operator +=(CStr &left, float right);
CStr &operator +=(CStr &left, double right);
#endif // #ifndef _STR_H