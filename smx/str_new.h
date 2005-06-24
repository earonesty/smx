/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
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

// simple string class, uses CBufChain or CBufRef - whichever is preferred
#ifndef CBuf
	#define CBuf CBufRef
#endif

class CStrNull;

class CStr : public CBuf<char> {
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
		{CBuf<char>::Copy(ref);}

	CStr (const CBuf<char> &ref) 
		{CBuf<char>::Copy(ref);}

	CStr &operator =(const CBuf<char> &ref) 
		{return (CStr &) CBuf<char>::Copy(ref);}

	CStr &operator =(const CStr &ref) 
		{return (CStr &) CBuf<char>::Copy(ref);}

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

	CStr &operator =(const char *d) 
		{if (d) {int n = strlen(d); Grow(n); if (m_buf) {memcpy(m_buf, d, n); m_buf[n] = '\0';}} else Grow(0); return *this;}

	char *operator +(int d) const
		{return m_buf + d;}

	char *operator +=(int d) 
		{return m_buf += d;}

	CStr operator +(const char *d) const

    {if (d) {int n = strlen(d); return CStr(*this).Append(d, n); } return *this;}

	CStr &operator <<(const char *d) 
		{if (d) {int n = strlen(d); return Append(d, n); } return *this;}

	CStr &operator <<(char d) 
		{int o = Length(); Grow(o + 1); Data()[o] = d; return *this;}

	CStr &operator <<(const CStr &d) 
		{if (d.Length()) {int o = Length(); Grow(o + d.Length()); memcpy(Data() + o, (const char *) d, d.Length());} return *this;}

	CStr &Grow(int n) 
		{assert(n==0 || this != &Empty); assert(n==0 || this != &Null); if (n > 0) {CBuf<char>::Grow(n+1); if (m_buf) m_buf[n] = '\0';} else { CBuf<char>::Grow(0); if (m_buf) m_buf[n] = '\0'; } return *this;}

	CStr &Copy(const CStr &ref) 
		{CBuf<char>::Copy(ref); return *this;}

	CStr &Copy(const char *d, int n) 
		{Grow(n); memcpy(m_buf, d, n); return *this;}

	CStr &Append(const char *d, int n) 
		{int o = Length(); Grow(o + n); memcpy(Data() + o, d, n); return *this;}

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
CStr &operator <<(CStr &left, int right);
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
