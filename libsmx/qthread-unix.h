/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef QTHREAD_H
#define QTHREAD_H

#include <pthread.h>
#include "crit.h"

//---------------------------#include "mthrdef.h"
/* MUTEX TYPES */
#define	MTHREAD_NON_RECURSIVE_MUTEX	 1
#define	MTHREAD_RECURSIVE_MUTEX	 	 2

#define	MTHREAD_SUCCESS			 	 0
#define	MTHREAD_ER_SYSERR			-1
#define	MTHREAD_ER_INTERNAL			-2
#define	MTHREAD_ER_MEMORY			-4
#define	MTHREAD_ER_NOT_IMPLEMENTED	-5

#define	MTHREAD_ER_DETACHED			-6

/* MUTEX ERROR CODES */
#define	MTHREAD_ER_MUTEX_CREATE		-40
#define	MTHREAD_ER_MUTEX_LOCKED		-41
#define	MTHREAD_ER_MUTEX_UNLOCKED	-42
#define	MTHREAD_ER_MUTEX_NOTOWNER	-43
#define	MTHREAD_ER_MUTEX_RELEASE	-44

/* KEY ERROR CODES */
#define	MTHREAD_ER_KEYBINDING		-50
#define	MTHREAD_ER_KEYCAPACITY		-51
#define	MTHREAD_ER_KEYINVALID		-52


#define MTHREAD_DEFAULT_STACKSIZE	0
//---------------------------#include "mthrdef.h"


//---------------------------#include "mthrpubd.h"
/*********************************************************************
** header files
*********************************************************************/

#include <pthread.h>
#include <stdio.h>

/***
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
**/

typedef int		MTHREADINT;
typedef void	VOID;
typedef void	ANY;
typedef void*   MTHREADROUTINE;

class qThread
{
	ThreadHandle    m_thread;
	pthread_attr_t	m_threadAttr;
	pthread_t		m_threadId;
	int *			threadStatus;
	int             m_rVal;

	struct RunWrapInfo {
		qThread *thread;
		int (*func)(void *);
		void *data;
	};

	bool RunVoid(void * (*func)(void *), void *data);

public:

	void *GetHandle() {
		return (void *) m_thread;
	}

	static void * _RunWrap(void *info) {
		return ((RunWrapInfo *) info)->thread->RunWrap((RunWrapInfo *) info);
	}

	void *RunWrap(RunWrapInfo *info) {
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
		m_rVal = info->func(info->data);
		return &m_rVal;	// for some silly reason it's a void pointer
	}

	bool Run(int (*func)(void *), void *data) {
		RunWrapInfo info = {this, func, data};
		return (RunVoid(_RunWrap, &info));
	}

	int Kill();
	
	bool Wait(int timeout=-1); // -1 is forever (TODO: timeout IS NOT YET IMPLEMENTED in UNIX

	bool IsDead();

// construct/destruct
	 qThread();
	~qThread();

private:
	MTHREADINT 	MthreadCreate (MTHREADROUTINE (*ThreadRoutine)(void *),
                       		   ANY *ThreadArg, 
					   		   MTHREADID *m_threadId, 
		  		       		   MTHREADINT StackSize, 
		  		       		   MTHREADINT DetachFlag, 
		  		       		   MTHREADINT KernelThread, 
					   		   MTHREADINT *Errno
					  		  );

	VOID		MthreadExit   (ANY *ExitCode);

	VOID		MthreadYield  (VOID);

	VOID		MthreadSelf   (MTHREADID *);

	MTHREADINT	MthreadKill   (MTHREADID *m_threadId, 
							   MTHREADINT *Errno);

	MTHREADINT	MthreadWait   (MTHREADID *m_threadId, 
							   MTHREADINT **ExitCode, 
							   MTHREADINT *Errno);

	VOID		MthreadOnce   (ANY (*OnceFunc)(), 
							   MTHREADONCE *OnceVariable);

	MTHREADINT	MthreadKeyCreate (MTHREADKEY *Key, 
								  ANY (*KeyDestructorFunc)(void *),
								  MTHREADINT KeySize, 
								  MTHREADINT *Errno);

	MTHREADINT	MthreadSetValue (MTHREADKEY *Key, 
								 ANY 		*Value, 
								 MTHREADINT *Errno);

	ANY *		MthreadGetValue   (MTHREADKEY *Key, 
								   MTHREADINT *Errno);

	MTHREADINT 	MthreadCondCreate (MTHREADCOND *Cond, 
								   MTHREADINT *Errno);

	MTHREADINT 	MthreadCondWait (MTHREADCOND *Cond, 
							 MTHREADMUTEX *Mutex, 
							 MTHREADINT *Errno);

	MTHREADINT	MthreadCondBroadcast (MTHREADCOND *Cond, 
							 MTHREADINT *Errno);

	MTHREADINT	MthreadCondSignal (MTHREADCOND *Cond, 
							 MTHREADINT *Errno);

};
#endif //#ifndef QTHREAD_H
