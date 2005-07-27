/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _BUF_REF_H
#define _BUF_REF_H

struct BUFFER_INFO
{
#if defined WIN32
	long  m_ref;     // reference count
#else
#ifdef USE_ATOMIC_H
	atomic_t m_ref;
#else
	_Atomic_word m_ref;
#endif
#endif
	int   m_x;
	int   m_n;
	char * buf() { return (char*)(this+1); }
};

class CBufRefChar {

protected:
	char *m_buf;

// helpers
	inline BUFFER_INFO * GetX() const	{ return (BUFFER_INFO *) m_buf - 1; }

	inline int SetCount(int n)		{ return GetX()->m_n = n; }
	inline int SetAlloc(int n)		{ return GetX()->m_x = n; }
	inline int Alloc() const       { return GetX()->m_x; }

	inline bool _Free();
//	inline long LokRef();

public:
	~CBufRefChar();
	CBufRefChar() 
		{m_buf = NULL;}
	CBufRefChar(int n);
	CBufRefChar(const CBufRefChar &newBuf) 
		{m_buf = NULL; Copy(newBuf);}
	
	char *Data() const	{return m_buf;}
	void *Grow(int n);
	void *Grow(int n, char *data)
		{Grow(n); return memcpy(m_buf, data, n);}

	CBufRefChar &operator =(const CBufRefChar &newBuf)
		{return Copy(newBuf);}

	CBufRefChar &Copy(const CBufRefChar &newBuf);

	int  Count() const	{ return m_buf ? GetX()->m_n : 0; }
#if defined WIN32
	void IncRef()			  { InterlockedIncrement(&(GetX()->m_ref)); }
	bool DecRef()			  { return (InterlockedDecrement(&(GetX()->m_ref)) <= 0); }
	void SetRef(long n) 		  { GetX()->m_ref = n; }
	long GetRef()			  { return GetX()->m_ref; }
#else
#ifdef USE_ATOMIC_H
	void IncRef()			  { atomic_inc(&(GetX()->m_ref)); }
	bool DecRef()			  { return (bool) atomic_dec_and_test(&(GetX()->m_ref)); }
	void SetRef(long n) 		  { atomic_set(&(GetX()->m_ref),n); }
	long GetRef()			  { return atomic_read(&(GetX()->m_ref)); }
#else
/*
        void IncRef()                     { __atomic_add(&(GetX()->m_ref),1); }
        bool DecRef()                     { return __exchange_and_add(&(GetX()->m_ref),-1) <= 0; }
        void SetRef(long n) 		  { GetX()->m_ref = n; }
        long GetRef()                     { return GetX()->m_ref; }
*/
        void IncRef()                     { ++GetX()->m_ref; }
        bool DecRef()                     { return --GetX()->m_ref <= 0; }
        void SetRef(long n) 		  { GetX()->m_ref = n; }
        long GetRef()                     { return GetX()->m_ref; }
#endif	
#endif
	void Change();
	void Free();
};

// template for buffer of uninitialized objects (zero instantion overhead ... just like C)
template<class TYPE>
class CBufRef : public CBufRefChar {
protected:
	int  Alloc() const      {return m_buf ? CBufRefChar::Alloc() : 0;}
public:
	CBufRef() 
		{}
	CBufRef(const CBufRef<TYPE> &newBuf) : 	CBufRefChar(newBuf)
		{}
	CBufRef(int n) : CBufRefChar(n * sizeof(TYPE))
		{}
	CBufRef(TYPE *d, int n)
		{Grow(n); memcpy(m_buf, d, n * sizeof(TYPE));}

	TYPE *Data() const	{return (TYPE *) m_buf;}
	TYPE *Grow(int n)
		{return (TYPE *) CBufRefChar::Grow(n * sizeof(TYPE));}
	TYPE *Grow(int n, TYPE *d)
		{Grow(n); memcpy(m_buf, d, n * sizeof(TYPE)); return Data();}
	CBufRef<TYPE> &Copy(const CBufRefChar &newBuf)
		{CBufRefChar::Copy((CBufRefChar &)newBuf); return *this;}
	CBufRef<TYPE> &operator =(const CBufRef<TYPE> &newBuf)
		{return Copy(newBuf);}

// inline
	operator TYPE *() const	{return (TYPE *) m_buf;}
//	operator bool()         {return Count() > 0;}
	int Count() const		{return CBufRefChar::Count() / sizeof(TYPE);}

	const TYPE &Add(const TYPE &d) {Grow(Count() + 1); Data()[Count()-1] = d; return d;}

	TYPE *Shift(int n) {
		void *tmp=malloc(sizeof(BUFFER_INFO));
		memcpy(tmp,GetX(),sizeof(BUFFER_INFO));
		memmove(GetX(),m_buf,n*sizeof(TYPE));
		m_buf += n*sizeof(TYPE);
		memcpy(GetX(),tmp,sizeof(BUFFER_INFO));
		SetCount(Count()-n);
		return Data();
	}

	TYPE *Restore(int n) {
		void *tmp=malloc(sizeof(BUFFER_INFO));
		memcpy(tmp,GetX(),sizeof(BUFFER_INFO));
		m_buf -= n*sizeof(TYPE);
		memmove(m_buf,m_buf-sizeof(BUFFER_INFO),n*sizeof(TYPE));
		memcpy(GetX(),tmp,sizeof(BUFFER_INFO));
		SetCount(Count()+n);
		return Data();
	}
};

// template for buffer of zero-initialized objects (memset, not calloc, lazy)
template<class TYPE>
class CBufRefZ : public CBufRef<TYPE> {
public:
	CBufRefZ(int n = 0)
		{Grow(n); }
	CBufRefZ(TYPE *d, int n)
		{Grow(n); memcpy(CBufRef<TYPE>::m_buf, d, n * sizeof(TYPE)); }

	TYPE *Grow(int n)
		{int o = CBufRef<TYPE>::Alloc(); CBufRef<TYPE>::Grow(n * sizeof(TYPE)); if (CBufRef<TYPE>::Alloc() > o) memset(CBufRef<TYPE>::m_buf + o, 0, (CBufRef<TYPE>::Alloc()-o)); return CBufRef<TYPE>::Data();}

	const TYPE &Add(const TYPE &d)
		{Grow(CBufRef<TYPE>::Count() + 1); CBufRef<TYPE>::Data()[CBufRef<TYPE>::Count()-1] = d; return d;}
};

#endif //#ifndef _BUF_REF_H
