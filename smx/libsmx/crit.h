/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
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
