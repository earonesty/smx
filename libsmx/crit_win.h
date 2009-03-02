/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
