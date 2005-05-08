/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _CRIT_WIN_H
#define _CRIT_WIN_H

class CCrit {
friend class CLock;
	CRITICAL_SECTION  m_crit;
    void _Enter()   {EnterCriticalSection(&m_crit);}

public:
	~CCrit()        {DeleteCriticalSection(&m_crit);}
	 CCrit()        {InitializeCriticalSection(&m_crit);}
	 CLock Enter();

	 void Leave()   {LeaveCriticalSection(&m_crit);}
};

inline rWait Win32Wait(HANDLE h, long ms = 1) {
	if (ms == -1)
		ms = INFINITE; 
	DWORD rVal; 
	if ( (rVal = WaitForSingleObject(h, ms)) ==  WAIT_OBJECT_0 )
		return rWaitOK; 
	return rVal ==  WAIT_TIMEOUT ? rWaitTimeout : rWaitInvalid;
}

class CMutex {
friend class CMutexLock;

	HANDLE  m_mutex;

public:
	~CMutex() {
		CloseHandle(m_mutex);
	}

	CMutex(const char *name) {
		m_mutex = CreateMutex(0, 0, name);
	}

	void Signal() {
		ReleaseMutex(m_mutex);
	}

	rWait Wait(long ms = -1) {
		return Win32Wait(m_mutex, ms);
	}
};

class CEvent {

	HANDLE  m_event;
public:
	~CEvent() {
		CloseHandle(m_event);
	}

	CEvent() {
		m_event = CreateEvent(0, 0, 0, 0);
	}

	void Pulse() {
		PulseEvent(m_event);
	}
	void Signal() {
		SetEvent(m_event);
	}
	void Reset() {
		ResetEvent(m_event);
	}

	rWait Wait(long ms = -1) {
		return Win32Wait(m_event, ms);
	}
};

#endif// #ifndef _CRIT_H