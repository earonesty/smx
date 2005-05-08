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