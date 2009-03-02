/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#ifndef _STR_H
#define _STR_H

#ifdef unix
#include "unix.h"
#endif

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

#ifndef _INC_ASSERT
	#include <assert.h>
#endif

#ifndef _BUF_H
	#include "buf.h"
#endif

class CStrNull;

class CStr : public CBufChar {
protected:
	static char *s_empty;
public:
	int Length() const	{return Count() ? Count() - 1 : 0;}

	CStr() 
		{}
	
	CStr(int n) 
		{Grow(n);}

	CStr(unsigned int n) 
		{Grow(n);}
	
	CStr(const char *d) 
		{if (d) {int n = strlen(d); Grow(n); memcpy(m_buf, d, n);}}
  	
	CStr(char c, int n = 1) 
		{Grow(n); memset(m_buf, c, n);}
	
	CStr(const char *d, int n) 
		{Grow(n); memcpy(m_buf, d, n);}
	
	CStr(const CStr &ref) 
		{CBufChar::Copy(ref);}

	CStr (const CBufChar &ref) 
		{CBufChar::Copy(ref);}

	CStr &operator =(const CBufChar &ref) 
		{return (CStr &) CBufChar::Copy(ref);}

	CStr &operator =(const CStr &ref) 
		{return (CStr &) CBufChar::Copy(ref);}

	CStr(double val) 
		{assert(0);}

	bool IsEmpty()  const {return Length() == 0;}
/*
	operator bool()  const {
		//return !IsEmpty();
		return !m_buf;
	}

	bool operator !() const {
		//return IsEmpty();
		return !m_buf;
	}

	operator char*() const {return m_buf ? m_buf : s_empty;}
*/

	operator char*() const {return m_buf;}

	char *SafeP() const {return m_buf ? m_buf : s_empty;}
	char *GetBuffer() const {return Data();}

	CStr &operator =(const char *d) 
		{if (d) {int n = strlen(d); Grow(n); if (m_buf) {memcpy(m_buf, d, n); m_buf[n] = '\0';}} else Grow(0); return *this;}

	char *operator +(int d) const
		{return m_buf + d;}

	char *operator +=(int d) 
		{return m_buf += d;}

	CStr &operator +=(const char *d) 
		{return Append(d);}

	CStr operator +(const char *d) const
    		{if (d) {int n = strlen(d); return CStr(*this).Append(d, n); } return *this;}

	CStr &operator <<(const char *d) 
		{if (d) {int n = strlen(d); return Append(d, n); } return *this;}

	CStr &operator <<(char d) 
		{int o = Length(); Grow(o + 1); Data()[o] = d; return *this;}

	CStr &operator <<(const CStr &d) 
		{if (d.Length()) {int o = Length(); Grow(o + d.Length()); memcpy(Data() + o, (const char *) d, d.Length());} return *this;}

	CStr &Grow(int n) 
		{assert(n==0 || this != &Empty); assert(n==0 || this != &Null); if (n > 0) {CBufChar::Grow(n+1); if (m_buf) m_buf[n] = '\0';} else { CBufChar::Grow(0); if (m_buf) m_buf[n] = '\0'; } return *this;}

	CStr &Copy(const CStr &ref) 
		{CBufChar::Copy(ref); return *this;}

	CStr &Copy(const char *d, int n) 
		{Grow(n); memcpy(m_buf, d, n); return *this;}

	CStr &Append(const char *d, int n) 
		{int o = Length(); Grow(o + n); memcpy(Data() + o, d, n); return *this;}

	CStr &Append(const char *d) 
		{return Append(d, d ? strlen(d) : 0);}

	CStr &Append(char c) 
		{int o = Length(); Grow(o + 1); Data()[o]=c; return *this;}

// helpers *** note it is assumed that memcpy has to work here ... memmove is correct for some optimizers
	CStr &LTrim()
		{ if (Length() > 0) { char *p = ((char*)m_buf); while(isspace(*p)) ++p; SetCount(Count() - (p-m_buf)); memcpy(m_buf, p, Count()); } return *this; }
	CStr &RTrim()
		{ if (Length() > 0) { UCHAR *p = ((UCHAR *)m_buf) + Length() - 1; while(p >= (UCHAR *) m_buf && isspace(*p)) --p; *++p = 0; SetCount(p - (UCHAR *) m_buf + 1); } return *this; }
	CStr &Trim()
		{LTrim(); return RTrim();}
	CStr &LTrim(char c)
		{ if (Length() > 0) { char *p = ((char*)m_buf); while(*p==c) ++p; SetCount(Count() - (p-m_buf)); memcpy(m_buf, p, Count()); } return *this; }
	CStr &RTrim(char c)
		{ if (Length() > 0) { char *p = ((char*)m_buf) + Length() - 1; while(p >= m_buf && *p==c) --p; *++p = 0; SetCount(p - m_buf + 1); } return *this; }
	CStr &Trim(char c)
		{LTrim(c); return RTrim(c);}
	CStr &Shrink()
		{if (Length() > 0) {Grow(strlen(m_buf));} return *this;}

// touch more efficient than strrchr because of the scan-forward
	char *RFindC(char c)
		{ if (Length() > 0) { char *p = ((char*)m_buf) + Length() - 1; while(p >= m_buf && *p != c) --p; return (p >= m_buf) ? p : 0;} return 0; }

	char *RFind(const CStr &s)
		{ if (Length() > 0) { char *p = ((char*)m_buf) + Length() - s.Length(); while(p >= m_buf && strncmp(p,s,s.Length())) --p; return p >= m_buf ? p : 0;} return 0; }

	char *RFindI(const CStr &s)
		{ if (Length() > 0) { char *p = ((char*)m_buf) + Length() - s.Length(); while(p >= m_buf && strnicmp(p,s,s.Length())) --p; return p >= m_buf ? p : 0;} return 0; }

	CStr &Format(const char *format, ...) { 
		va_list marker; va_start( marker, format); 
		Grow(_vsnprintf(Grow(Length()+1024), Length()+1024, format, marker)); 
		va_end( marker ); return *this; 
	}

	static CStr Null;
	static CStr Empty;
};


// helpers in "strx.cpp"
CStr operator <<(CStr left, int right);
CStr &operator <<(CStr &left, long right);
CStr &operator <<(CStr &left, float right);
CStr &operator <<(CStr &left, double right);
CStr operator <<(const char *left, const CStr &right);

inline bool operator &&(const CStr &a, bool b) {
        return b && a.Length() > 0;
}
inline bool operator &&(bool a, const CStr &b) {
        return a && b.Length() > 0;
}

double strtodx(const char *str);
int    strlenx(const char *b);
double strtolx(const char *str);

#endif // #ifndef _STR_H
