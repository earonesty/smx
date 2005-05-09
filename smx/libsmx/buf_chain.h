/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/




#ifndef _BUF_CHAIN_H_
#define _BUF_CHAIN_H_

#ifndef BUFFER_GROW
	#define BUFFER_GROW 16
#endif

#ifndef BUFFER_CHAIN
	#define BUFFER_CHAIN 1
#endif

#ifndef memcpy
	#include <memory.h>
#endif

#ifdef WIN32
#ifdef _DEBUG
    #pragma comment(lib, "qlibd.lib")
#else
    #pragma comment(lib, "qlib.lib")
#endif
#endif

class CBufChainChar {
protected:

// buffer
	char *  m_buf;

#if BUFFER_CHAIN
	CBufChainChar *m_parent;
	CBufChainChar *m_child;
	inline int Unchain();
#endif

	int		m_x, m_n;

public:
   ~CBufChainChar();
    void Init();
	CBufChainChar();
	CBufChainChar(int n);
	CBufChainChar(const CBufChainChar &newBuf);
	CBufChainChar(char *d, int n);

	CBufChainChar &Grow(int n);
	CBufChainChar &Change();
	CBufChainChar &Free();
	CBufChainChar &Copy(const CBufChainChar &newBuf);

	CBufChainChar &Grow(int n, char *d) {Grow(n); memcpy(m_buf, d, n); return *this;}

	CBufChainChar &PointAt(char *p, int n);

// inline
	int Alloc() const   {return m_x;}
	int Count() const	{return m_n;}
	char *Data() const	{return (char *) m_buf;}
	void SetCount(int n){m_n = n;}

// operators
	CBufChainChar &operator =(const CBufChainChar &newBuf)
		{return Copy(newBuf);}
};

// template for buffer of uninitialized objects (zero instantion overhead ... just like C)
template<class TYPE>
class CBufChain : public CBufChainChar {
public:
	~CBufChain()									{}
	CBufChain(int n = 0)							{Grow(n);}
	CBufChain(const CBufChain<TYPE> &newBuf)		{Copy(newBuf);}
	CBufChain(TYPE *d, int n)						{Grow(n); memcpy(m_buf, d, n * sizeof(TYPE));}

	CBufChain<TYPE> &Grow(int n)					{return (CBufChain<TYPE> &) CBufChainChar::Grow(n*sizeof(TYPE));}
	CBufChain<TYPE> &Grow(int n, TYPE *d)		    {return (CBufChain<TYPE> &) CBufChainChar::Grow(n*sizeof(TYPE), d);}
	CBufChain<TYPE> &PointAt(TYPE *p, int n)		{return (CBufChain<TYPE> &) CBufChainChar::PointAt((char *)p, n*sizeof(TYPE));}

// inline
	int Count() const	{return m_n / sizeof(TYPE);}
	TYPE *Data() const	{return (TYPE *) m_buf;}

// operators
	CBufChain<TYPE> &operator =(const CBufChain<TYPE> &newBuf)
		{return (CBufChain<TYPE> &) Copy(newBuf);}

	const TYPE &Add(const TYPE &d) 
		{Grow(Count() + 1); Data()[Count()-1] = d; return d;}

	operator TYPE *() const {return Data();}
	operator TYPE *()       {return Data();}
	operator bool()         {return Count() > 0;}

	TYPE *Shift(int n) { 
		m_buf += n*sizeof(TYPE); 
		SetCount((Count()-n)*sizeof(TYPE));
		return Data(); 
	}

	TYPE *Restore(int n) { 
		m_buf -= n*sizeof(TYPE); 
		SetCount((Count()+n)*sizeof(TYPE));
		return Data(); 
	}
};

// template for buffer of zero-initialized objects (memset, not calloc, lazy)
template<class TYPE>
class CBufChainZ : public CBufChain<TYPE> {
public:
	CBufChainZ(int n = 0) 
		{ m_buf = 0; Grow(n); }

	CBufChainZ(TYPE *d, int n) 
		{ m_buf = 0; Grow(n); memcpy(m_buf, d, n * sizeof(TYPE)); }

	TYPE *Grow(int n) 
		{int o = Alloc(); CBufChainChar::Grow(n * sizeof(TYPE)); if (Alloc() > o) memset(m_buf + o, 0, (Alloc()-o)); return Data();}

	const TYPE &Add(const TYPE &d) 
		{Grow(Count() + 1); Data()[Count()-1] = d; return d;}
};
#endif //#ifndef _BUF_CHAIN_H_
