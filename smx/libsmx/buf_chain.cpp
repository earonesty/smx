/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#include "buf_chain.h"

//
// CBufChainChar - C void buffer code, malloc/free
//

int s_growby = BUFFER_GROW;

CBufChainChar::CBufChainChar(const CBufChainChar &newBuf) 
{
#if BUFFER_CHAIN
	m_parent=m_child=0;
#endif
	m_buf = 0; Copy(newBuf);
}




CBufChainChar::CBufChainChar(char *d, int n)  
{
#if BUFFER_CHAIN
	m_parent=m_child=0;
#endif
	m_x = 0; Grow(n); memcpy(m_buf, d, n );
}


CBufChainChar::CBufChainChar() 
{
#if BUFFER_CHAIN
	m_parent = m_child = NULL;
#endif
	m_buf = NULL; 
	m_x = 0;
}

CBufChainChar::CBufChainChar(int n) 
{
#if BUFFER_CHAIN
	m_parent = m_child = NULL;
#endif
	if (n > 0)
		{m_buf = (char *) malloc(m_x = m_n = n);}
	else
		m_buf = NULL; 
}

CBufChainChar::~CBufChainChar() 
{
#if BUFFER_CHAIN
	if (Unchain())
#endif
	free(m_buf);
}


CBufChainChar &CBufChainChar::Grow(int n) 
{
	if (n > 0) {
#if BUFFER_CHAIN
		if (!Unchain()) {
			m_parent = m_child = NULL;
			char *temp = m_buf;
			m_buf = (char *) malloc(m_x = n + s_growby);
			memcpy(m_buf, temp, n);
		} else
#endif
		if (n > m_x)
			m_buf = (char *) realloc(m_buf, m_x = n + s_growby);
		m_n = n;
		return *this;
	} else {
		m_n = 0;
		return *this;
	}
}


CBufChainChar &CBufChainChar::Free() 
{
#if BUFFER_CHAIN

	if (!Unchain())
		m_parent = m_child = NULL;
	else
#endif
	free(m_buf);
	m_n = m_x = 0;
	m_buf = NULL;
	return *this;
}


CBufChainChar &CBufChainChar::Change() 
{
#if BUFFER_CHAIN
    if (!Unchain()) {
	    m_parent = m_child = NULL;
	    char *temp = m_buf;
	    m_buf = (char *) malloc(m_x);
	    memcpy(m_buf, temp, m_n);
    }
#endif
	return *this;
}

CBufChainChar &CBufChainChar::Copy(const CBufChainChar &newBuf) 
{
#if BUFFER_CHAIN
	if (m_buf != newBuf.m_buf) {
		if (Unchain()) 
			free(m_buf);
		if (newBuf.m_parent == (CBufChainChar *) -1) 
			m_parent = (CBufChainChar *) -1;
		else {
			m_parent = (CBufChainChar *) &newBuf;
			m_child = newBuf.m_child;
			m_parent->m_child = this;
			if (m_child)
				m_child->m_parent = this;
		}
		m_buf = newBuf.m_buf;
		m_n = newBuf.m_n;
		m_x = newBuf.m_x;
	}
#else
	Grow(newBuf.m_n);
	memcpy(m_buf, newBuf.m_buf, newBuf.m_n);
#endif
	return *this;
}

#if BUFFER_CHAIN
// returns true if sole owner
inline int CBufChainChar::Unchain() 
{
	if (m_parent || m_child) {
		if (m_parent && (m_parent != (CBufChainChar *) -1)) {
			assert(m_parent->m_child = this);
			m_parent->m_child = m_child;
		}
		if (m_child) {
			assert(m_child->m_parent = this);
			if (m_parent != (CBufChainChar *) -1)
				m_child->m_parent = m_parent;
			else
				m_child = 0;
		}
		return 0;
	} else
		return 1;
}
#endif

#if BUFFER_CHAIN
CBufChainChar &CBufChainChar::PointAt(char *p, int n)		
{
	if (Unchain()) 
		free(m_buf);
	m_buf = p;
	m_n = n;
	m_parent = (CBufChainChar *) -1;
	return *this;
}
#endif
