/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef QTHREAD_H
#define QTHREAD_H

class qThread
{
	HANDLE   m_thread;
	unsigned m_tid;
	int      m_rval;

	struct RunWrapInfo {
		qThread *thread;
		int (*func)(void *);
		void *data;
	};

public:

	void * GetHandle() {
		return m_thread;
	}

	static unsigned int __stdcall _RunWrap(void *info) {
		return ((RunWrapInfo *) info)->thread->RunWrap((RunWrapInfo *) info);
	}

	int RunWrap(RunWrapInfo *info) {
		if (info->func) {
			int m_rval = info->func(info->data);
			CloseHandle(m_thread);
			m_thread = NULL;
		}
		free(info);
		return m_rval;
	}

	bool Run(int (*func)(void *), void *data) {
		assert(sizeof(void *) == sizeof(int));
		RunWrapInfo *info = (RunWrapInfo *) malloc(sizeof(RunWrapInfo));
		info->func = func;
		info->data = data;
		info->thread = this;
		return (_beginthreadex(NULL, 0, _RunWrap, info, 0, &m_tid) != 0);
	}

	bool Kill() {
		if (m_thread) {
			bool ok = TerminateThread(m_thread, -1) != 0;
			CloseHandle(m_thread);
			m_thread = NULL;
			return ok;
		} else
			return false;
	}

	bool Wait(int timeout = -1) {
		return WaitForSingleObject(m_thread, timeout) == WAIT_OBJECT_0;
	}

	bool IsDead() {
		return m_thread == NULL;
	}

// construct/destruct
	qThread() {
		m_thread = NULL;
		m_tid    = 0;
		m_rval   = 0;
	}

	~qThread() {

	}
};

#endif