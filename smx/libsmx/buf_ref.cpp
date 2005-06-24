/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/




#ifdef WIN32
	#include <windows.h>
	#include <winbase.h>
#else
  #include "unix.h"
#endif

#ifndef BUFFER_GROW
#ifdef _DEBUG
#define BUFFER_GROW 256
#else
#define BUFFER_GROW 256
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#include "buf_ref.h"


//
// CBufRefChar - C buffer code, malloc/free
//

static int s_growby = BUFFER_GROW;

//#define GETX ((BUFFER_INFO *) m_buf - 1)

#if defined WIN32
bool CBufRefChar::_Free()				{ return DecRef(); }
//	long CBufRefChar::LokRef()			{ return (long) InterlockedCompareExchange(((void**)(&GETX->m_ref)), 0, (void *) 1); }
#else
bool CBufRefChar::_Free()			{ return DecRef(); }
//	long CBufRefChar::LokRef()			{ if (DecRef() == 1) {SetRef(0); return 1;} else return GetRef(); }
#endif

CBufRefChar::CBufRefChar(int n) 
{
	if (n > 0) {
		BUFFER_INFO* pX = (BUFFER_INFO*) malloc(n + sizeof(BUFFER_INFO));

		m_buf = pX->buf();
		m_buf[n] = '\0';
		pX->m_n = pX->m_x = n;
		SetRef(1);
	} else { 
		m_buf = 0;
	}
}

CBufRefChar::~CBufRefChar() 
{
	assert(!m_buf || GetRef() > 0);
	Free();
}

void *CBufRefChar::Grow(int n) 
{
	if (n > 0) {
		BUFFER_INFO* pX;

		int z;
		if (!m_buf)
			z = n;
		else
			z = n + max(s_growby, n>>1);

		if (!m_buf) {
			pX = (BUFFER_INFO*) malloc(z + sizeof(BUFFER_INFO));
      if (pX) {
  			m_buf = pX->buf();
  			SetRef(1);
  			SetAlloc( z );
      }
		} else if (GetRef() == 1) {
			if (n > Alloc()) {
				//z = (n << 1) + s_growby;
				pX = (BUFFER_INFO*) realloc(GetX(), z + sizeof(BUFFER_INFO));
				if (pX) {
					m_buf = pX->buf();
					SetAlloc( z );
				}
			}
		} else {
			assert(GetRef() >= 1);
			pX = (BUFFER_INFO*) malloc(z + sizeof(BUFFER_INFO));
			assert(pX != NULL);
			if (pX) {
				memcpy(pX->buf(), m_buf, Count() < n ? Count() : n);
				Free();
				m_buf = pX->buf();
				SetRef(1);
				SetAlloc( z );
			}
		}
		
		//SetAlloc( z );
		if (m_buf) SetCount(n);
		return m_buf;
	} else {
		if (m_buf) {
			if (GetRef() == 1) {
				SetCount(0);
			} else {
				Free();
			}
		}
		return m_buf;
	}
}

void CBufRefChar::Free() 
{
	if (m_buf) {
		if (_Free()) 
			free(GetX());
		m_buf = NULL;
	}
}

void CBufRefChar::Change()
{
	if (m_buf) {
		BUFFER_INFO* pX = (BUFFER_INFO*) malloc(Alloc() + sizeof(BUFFER_INFO));
		memcpy(pX, GetX(), Count() + sizeof(BUFFER_INFO));
		Free();
		m_buf = pX->buf();
		SetRef(1);
	}
}

CBufRefChar &CBufRefChar::Copy(const CBufRefChar &newBuf) 
{
	if (m_buf != newBuf.m_buf) {
		if (newBuf.m_buf) {
			assert(((CBufRefChar &)newBuf).GetRef() > 0);
			((CBufRefChar &)newBuf).IncRef();
			Free();
			m_buf = newBuf.m_buf;
		} else {
			Free();
		}
	}
	return *this;
}
