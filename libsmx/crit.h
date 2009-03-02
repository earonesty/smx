/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _SMX_CRIT_H
#define _SMX_CRIT_H

class CCrit;
class CLock;
class CMutex;
class CMutexLock;

enum rWait {
	rWaitInvalid   = -1,
	rWaitTimeout =  0,
	rWaitOK =  1
};

#ifdef WIN32
  #include "crit_win.h"
#else
  #include "crit_unix.h"
#endif

class CLock {
protected:
	CCrit  *m_pCrit;

public:
	CLock(CCrit *pCrit = NULL) {
		m_pCrit = pCrit; 
		if (m_pCrit) 
			m_pCrit->_Enter(); 
	}
	CLock(const CLock &lock) {
		m_pCrit = lock.m_pCrit; 
		if (m_pCrit) 
			m_pCrit->_Enter(); 
	}
	~CLock() {
		Leave(); 
	}
	
	bool IsLocked() {
		return (m_pCrit!=NULL);
	}

	CLock &operator =(const CLock &lock) {
		if (m_pCrit != lock.m_pCrit) {
			CCrit * pTmp = m_pCrit; 
			m_pCrit = lock.m_pCrit; 
			if (m_pCrit)
				m_pCrit->_Enter();
			if (pTmp)
				pTmp->Leave();
		}
		return *this;
	}
	void Leave() {

		if (m_pCrit) {
			CCrit * pTmp = m_pCrit; 
			m_pCrit = 0; 
			pTmp->Leave();
		}  
	}
};


class CMutexLock {
protected:
	CMutex * m_mutex;
	rWait    m_rw;
public:
	CMutexLock() {
		m_mutex = NULL; 
		m_rw = rWaitInvalid;
	}
	CMutexLock(CMutex * mutex, rWait rw) {
		m_mutex = mutex; 
		m_rw = rw;
	}
	CMutexLock(CMutexLock &lock) {
		m_mutex = lock.m_mutex; 
		if (m_mutex)
			m_rw = m_mutex->Wait(-1);
		else
			m_rw = rWaitInvalid;
	}
	CMutexLock(CMutex &mutex, long ms = -1) {

		m_mutex = &mutex; 
		m_rw = mutex.Wait(ms);
	}
	~CMutexLock() {
		Leave();
	}
	
	operator bool() {
		return (m_mutex && m_rw == rWaitOK);
	}

	CMutexLock &operator =(CMutexLock &lock) {
		if (m_mutex != lock.m_mutex) {
			CMutex * pTmp = (m_rw == rWaitOK) ? m_mutex : 0;
			m_mutex = lock.m_mutex; 
			if (m_mutex)
				m_rw = m_mutex->Wait(-1);
			else
				m_rw = rWaitInvalid;
			if (pTmp)
				pTmp->Signal();
		}
		return *this;
	}
	void Leave() {
		if (m_mutex && m_rw == rWaitOK) {

			m_mutex->Signal();
			m_mutex = 0;
			m_rw = rWaitInvalid;
		}
	}
};

inline CLock CCrit::Enter()  {return CLock(this);}


#endif// #ifndef _CRIT_H
