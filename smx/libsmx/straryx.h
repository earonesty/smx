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

#ifndef _MAP_H
#include "str.h"
#endif

#define CBufZ CBufChainZ
class CStrAry : public CBufZ<CStr> {
public:
	CStrAry(int n = 0)
		{ m_buf = 0; Grow(n); }
	CStrAry(CStr *d, int n)
		{ m_buf = 0; Grow(n); memcpy(m_buf, d, n * sizeof(CStr)); }

	CStr *Grow(int n)
		{int o = Count(); CBufZ<CStr>::Grow(n);
				if (Count() < o)
				{int i; for (i = Count(); i < o; ++i) Data()[i].Free();}
		return Data();}

	~CStrAry()
		{int i; for (i = 0; i < (Alloc() / (int) sizeof(CStr)); ++i) Data()[i].Free();}

	int Count() const
		{if (this) return CBufZ<CStr>::Count(); else return 0;}
	CStr &GetAt(int i)
		{if (i < Count()) return Data()[i]; else return CStr::Null;}
	CStr &SetAt(int i, const char *str)
		{if (i >= Count()) Grow(i+1); return Data()[i] = str;}
	CStr &SetAt(int i, CStr &str)
		{if (i >= Count()) Grow(i+1); return Data()[i] = str;}
	CStr &Add(CStr str)
		{Grow(Count()+1); return Data()[Count()-1] = str;}

	operator char **()
		{return (char **) Data();}
	operator const char **()
		{return (const char **) Data();}
	CStr &operator [](int index)
		{return GetAt(index);}


	inline CStrAry &operator =(const CStrAry &newBuf) {
		int i; 
		for (i = 0; i < (Alloc() / (int) sizeof(CStr)); ++i) 
			Data()[i].Free();
		if (&newBuf && newBuf.Data()) {
			i = newBuf.Count();
			Grow(i);
			for (i = 0; i < Count(); ++i) 
				Data()[i] = newBuf.Data()[i];
		}
		return *this;
	}


	inline CStrAry &operator =(const char **newBuf) {
		int i; 
		for (i = 0; i < (Alloc() / (int) sizeof(CStr)); ++i) 
			Data()[i].Free();
		
		if (newBuf) {
			const char **t = newBuf;
			while (*t) ++t;
			Grow(t-newBuf);
			for (t = newBuf; *t; ++t) 
				Data()[i] = *t;
		}
		return *this;
	}
};

#endif // #ifndef _STRARY_H
