/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/




#ifndef _ARY_H
#define _ARY_H

// template for array of initialized objects (lots of instantion overhead..just like Microsoft)

#ifndef ARY_CHAIN
	#define ARY_CHAIN 1
#endif


template<class TYPE>
class CAry {
// grow by this amount
	static	int m_grow;

// buffer
	int		m_x, m_n;
	TYPE*	m_buf;

#if ARY_CHAIN
	const CAry *m_parent;
	const CAry *m_child;
	int Unchain();
#endif

public:
	~CAry();
	CAry(int n = 0) {Init(n);}
	CAry(TYPE *d, int n);

	CAry(CAry &newAry) {Init(0); operator =(*((CAry *)&newAry));}

	CAry &Init(int n);
	TYPE *Grow(int n);
	void Shrink();
	CAry &operator =(const CAry &newAry);
// inline
	operator const TYPE *() const {return m_buf;}
	operator TYPE *() {Grow(m_n); return m_buf;}
	int Count() const {return m_n;}
	TYPE &GetAt(int i) const {return m_buf[i];}
	int Add(const TYPE &d) {Grow(m_n + 1); m_buf[m_n-1] = d; return m_n-1;}
	static void SetGrowBy(int n) {m_grow = n;}
};

//
// CAry - C++ buffer code (blows up exe with each use)
//

template <class TYPE>
int CAry<TYPE>::m_grow = BUFFER_GROW;

template<class TYPE>
CAry<TYPE> &CAry<TYPE>::Init(int n) {
	if (n > 0)
		m_buf = new TYPE[m_x = (m_n = n)];
	else 
		{ m_x = m_n = 0; m_buf = NULL; }
#if ARY_CHAIN
	m_parent = m_child = NULL;
#endif
	return *this;
}

template<class TYPE>
CAry<TYPE>::~CAry() {
#if ARY_CHAIN
	if (Unchain())
#endif
	if (m_buf) delete[] m_buf;
}

template<class TYPE>
TYPE *CAry<TYPE>::Grow(int n) {
	if (n > 0) {
		if (n > m_x) {
			int i;
			TYPE *newBuf = new TYPE[m_x = n + m_grow];
			for (i = 0; i < m_n; ++i)
				newBuf[i] = m_buf[i];
#if ARY_CHAIN
			if (!Unchain()) {
				m_parent = m_child = NULL;
			} else
#endif
			if (m_buf) delete[] m_buf;
			m_buf = newBuf;
		}
		m_n = n;
		return m_buf;
	} else {
		m_n = 0;
		return m_buf;
	}
}

template<class TYPE>
void CAry<TYPE>::Shrink() {
#if ARY_CHAIN
	if (!Unchain())
		m_parent = m_child = NULL;
	else
#endif
	delete[] m_buf;
	m_n = m_x = 0;
	m_buf = NULL;
}



template<class TYPE>
CAry<TYPE> &CAry<TYPE>::operator =(const CAry &newBuf) {
#if ARY_CHAIN
	if (m_buf != newBuf.m_buf) {
		if (Unchain()) if (m_buf) delete[] m_buf;
		m_parent = &newBuf;
		m_buf = newBuf.m_buf;
		m_child = newBuf.m_child;
		m_n = newBuf.m_n;
		m_x = newBuf.m_x;
		((CAry<TYPE> *) &newBuf)->m_child = this;
	}
#else
	Grow(newBuf.m_n);
	for (i = 0; i < m_n; ++i)
		m_buf[i] = newBuf[i];
#endif
	return *this;
}

#if ARY_CHAIN
// returns true if sole owner
template<class TYPE>
inline int CAry<TYPE>::Unchain() {
	if (m_parent || m_child) {
		if (m_parent)
			((CAry<TYPE> *) m_parent)->m_child = m_child;
		if (m_child)
			((CAry<TYPE> *) m_child)->m_parent = m_parent;
		return 0;
	} else
		return 1;
}
#endif

#endif// #ifndef _ARY_H
