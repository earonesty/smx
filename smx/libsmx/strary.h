/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
*/
#ifndef _STRARY_H
#define _STRARY_H

#ifndef _STR_H
#include "str.h"
#endif

#include <vector>

class CStrAry : public std::vector<CStr> {
protected:
	const char **myV;
public:
	CStrAry(int n=0) : std::vector<CStr>(n)
		{ myV = NULL; }
	CStrAry(CStr *d, int n) : std::vector<CStr>(n)
		{ memcpy(pointer(), d, n * sizeof(CStr)); myV = NULL; }
	~CStrAry() { if (myV) free(myV); }

	CStr *Grow(int n)
		{resize(n); return Data();}
	CStr *Data() 
		{return &(*begin());};
	int Count() const
		{if (this) return size(); else return 0;}
	CStr &GetAt(int i)
		{if (i < Count()) return *(begin() + i); else return CStr::Null;}
	CStr &SetAt(int i, const char *str)
		{if (i >= Count()) Grow(i+1); return *(begin() + i) = str;}
	CStr &operator[](int i)
		{return GetAt(i);}
	CStr &SetAt(int i, CStr &str)
		{if (i >= Count()) Grow(i+1); return *(begin() + i) = str;}
	CStr &Add(CStr str)
		{push_back(str); return back();}
	CStrAry &operator =(const char **newBuf) {
		if (newBuf) {
			const char **t = newBuf;
			while (*t) ++t;
			Grow(t-newBuf);
			int i=0;
			for (t = newBuf; *t; ++t)
				Data()[i++] = *t;
		}
		return *this;
	}
	operator const char ** () {
		myV = (const char **) realloc(myV,sizeof(char *) * size());
		unsigned int i; for (i=0;i<size();++i) {
			myV[i] = GetAt(i);
		}
		myV[size()]=NULL;
		return myV;
	}
        void Shift(int n) {
		_M_start += n;
	}
        void Restore(int n) {
		_M_start -= n;
	}
};
#endif // #ifndef _STRARY_H
