#ifndef _STR_H
#define _STR_H

#include "StdString.h"

#ifndef UCHAR
	#define UCHAR unsigned char
#endif

class CStr : public CStdString
{
protected:
        static char *s_empty;
public:

// CStr inline constructors
        CStr() { }
        CStr(int n) {Grow(n);}
        CStr(unsigned int n) {Grow(n);}
        CStr(char c, int n = 1) : CStdString(n, c) { }
        CStr(const CStr& str) : CStdString(str) { }
        CStr(const std::string& str) : CStdString(str) { }
        CStr(const std::wstring& str) : CStdString(str) { }
        CStr(const_pointer pT, size_type n) : CStdString(pT, n) { }
#ifdef SS_UNSIGNED
        CStr(PCUSTR pU) : CStdString(pU) { }
#endif
        CStr(PCSTR pA) : CStdString(pA) { }
        CStr(PCWSTR pW) : CStdString(pW) { }

        TCHAR* Data() const
	        {return empty() ? const_cast<TCHAR*>(this->data()) : (TCHAR *) &(this->at(0));}
        TCHAR* SafeP() const
	        {return (Data()) ? Data() : ((TCHAR *)Empty.GetBuffer());}
	operator bool() const
		{return !empty();}
//	TCHAR &operator *() const
//		{return *Data();}

	CStr &Grow(int n)
		{resize(n); return *this;}
	CStr &Grow(int n, TCHAR *d)
		{resize(n); memcpy(Data(),d,n); return *this;}

	CStr &Append(const CStr s)
		{append(s); return *this;}
	CStr &Append(const char *d)
		{append(d); return *this;}
	CStr &Append(char d)
		{resize(size()+1); Data()[size()-1]=d; return *this;}
        CStr &Append(const char *d, int n)
                {append(d, n); return *this;}
	int Length() const
		{return size();}

	CStr &operator +=(const char *d)
                {return Append(d);}
        CStr &operator +=(char d)
                {return Append(d);}
        CStr &operator +=(const CStr &d)
                {return Append(d);}

        CStr &operator <<(char d)
                {return Append(d);}
        CStr &operator <<(const char *d)
                {return Append(d);}
        CStr &operator <<(const CStr &d)
                {return Append(d);}

        static CStr Null;
        static CStr Empty;

// helpers *** note it is assumed that memcpy has to work here ... memmove is correct for some optimizers
        CStr &LTrim()
                { TrimLeft(); return *this; }
        CStr &RTrim()
                { TrimRight(); return *this; }
        CStr &Trim()
                { TrimLeft(); TrimRight(); return *this; }
        CStr &LTrim(char c)
                { TrimLeft(c); return *this; }
        CStr &RTrim(char c)
                { TrimRight(c); return *this; }
        CStr &Trim(char c)
                { TrimLeft(c); TrimRight(c); return *this; }
        CStr &Shrink()
                { if (!IsEmpty()) Grow(strlen(Data())); return *this; }

// touch more efficient than strrchr because of the scan-forward
        char *RFindC(char c) const
                { if (Length() > 0) { char *p = ((char*)Data()) + Length() - 1; while(p >= Data() && *p != c) --p; return (p >= Data()) ? p : 0;} return 0; }

        char *RFind(const CStr &s) const
                { if (Length() > 0) { char *p = ((char*)Data()) + Length() - s.Length(); while(p >= Data() && strncmp(p,s,s.Length())) --p; return p >= Data() ? p : 0;} return 0; }

        char *RFindI(const CStr &s) const
                { if (Length() > 0) { char *p = ((char*)Data()) + Length() - s.Length(); while(p >= Data() && strnicmp(p,s,s.Length())) --p; return p >= Data() ? p : 0;} return 0; }

        char *operator +(int d) const
                {return Data() + d;}
	bool IsEmpty() const
		{ return size() == 0; }
        bool operator !() const 
  		{return empty();}
	void Free()
		{resize(0);reserve(0);}
	void Change()
		{reserve(size());}

};

// helpers in "strx.cpp"
CStr operator <<(CStr left, int right);
CStr &operator <<(CStr &left, long right);
CStr &operator <<(CStr &left, float right);
CStr &operator <<(CStr &left, double right);
CStr operator <<(const char *left, const CStr &right);
CStr operator +(const CStr &left, char right);

inline bool operator &&(const CStr &a, bool b) {
        return b && a.Length() > 0;
}
inline bool operator &&(bool a, const CStr &b) {
        return a && b.Length() > 0;
}

double strtodx(const char *str);
int    strlenx(const char *b);
double strtolx(const char *str);

#endif
