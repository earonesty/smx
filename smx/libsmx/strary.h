/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#ifndef _STRARY_H
#define _STRARY_H

#ifndef _STR_H
#include "str.h"
#endif

#include <vector>

#include <memory.h>

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
