/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "qthread.h"

#ifdef unix

qThread::qThread()
{
	pthread_attr_init(&m_threadAttr);
	pthread_attr_setdetachstate(&m_threadAttr, PTHREAD_CREATE_JOINABLE);
}

qThread::~qThread()
{
}

bool qThread::RunVoid(void * (*func)(void *data), void *data)
{
	return (pthread_create(&m_threadId,&m_threadAttr, func,data) == 0);
}

int qThread::Kill()
{
	return(0);
}

bool qThread::Wait(int timeout) // -1 is forever
{
	// WARNING TIMOUT IS IGNORED... WE NEED CONDITIONS AND STUFF IF THERE IS A TIMEOUT
  if (m_threadId) {
  	pthread_join(m_threadId,(void**) &threadStatus);	
  	return (*threadStatus);
  } else {
    return false;
  }
}

bool qThread::IsDead()
{
	return (true);
}


MTHREADINT qThread::MthreadCreate (MTHREADROUTINE (*ThreadRoutine)(void *),
			   ANY			  *ThreadArg,
			   MTHREADID	  *MThreadId,
			   MTHREADINT			  StackSize, /* UNUSED IN PTHREADS */
			   MTHREADINT	  DetachFlag, 
			   MTHREADINT	  KernelThread, 
			   MTHREADINT			  *Errno)
{
	
	MTHREADINT			Ret;
//	pthread_t			Tid;
	pthread_attr_t		ThreadAttr;

	pthread_attr_init(&ThreadAttr);

	if (DetachFlag == 0)
		pthread_attr_setdetachstate(&ThreadAttr, PTHREAD_CREATE_JOINABLE);

	if (KernelThread == 1)
		pthread_attr_setscope(&ThreadAttr, PTHREAD_SCOPE_SYSTEM);

	Ret = pthread_create(&MThreadId->ThreadId, &ThreadAttr, 
									ThreadRoutine, ThreadArg);
	if (Ret != 0)
	{
		*Errno = Ret;
		return(MTHREAD_ER_SYSERR);
	}
	return(MTHREAD_SUCCESS);
}

VOID qThread::MthreadExit(ANY*	ExitCode)
{
	pthread_exit(ExitCode);
	return;
}



VOID qThread::MthreadYield(VOID)
{
	sched_yield();
	return;
}


VOID qThread::MthreadSelf(MTHREADID *MThreadId)
{
	MThreadId->ThreadId = pthread_self();
	return;
}

MTHREADINT qThread::MthreadKill(MTHREADID *MThreadId, MTHREADINT	*Errno)
{
	MTHREADINT	Ret;
	Ret = pthread_cancel(MThreadId->ThreadId);
	if (Ret != 0)
	{
		*Errno = Ret;
		return(MTHREAD_ER_SYSERR);
	}
	return(MTHREAD_SUCCESS);
}

MTHREADINT qThread::MthreadWait(MTHREADID		*MThreadId,
			MTHREADINT		**Status,
			MTHREADINT		*Errno)
{
	MTHREADINT	Ret;

	Ret = pthread_join(MThreadId->ThreadId, (ANY **) Status);
	if (Ret != 0)
	{
		*Errno = Ret;
		return(MTHREAD_ER_SYSERR);
	}
	return(MTHREAD_SUCCESS);
}

VOID qThread::MthreadOnce(ANY (*OnceFunction)(),
			MTHREADONCE	*OnceVariable)
{
	pthread_once(OnceVariable, OnceFunction);
	return;
}

MTHREADINT qThread::MthreadKeyCreate(MTHREADKEY *MThreadKey,
				 ANY 		(*DestructorFunc)(void *),
				 MTHREADINT		 KeySize, 
				 MTHREADINT		 *Errno)
{
	MTHREADINT	Ret;

	Ret = pthread_key_create(&MThreadKey->Key, DestructorFunc);  
	if (Ret != 0)
	{
		*Errno = Ret;
		return(MTHREAD_ER_SYSERR);
	}
	return(MTHREAD_SUCCESS);
}

MTHREADINT qThread::MthreadSetValue(MTHREADKEY *MThreadKey,
				ANY	   *Value,
				MTHREADINT	   *Errno)
{
	MTHREADINT	Ret;
	Ret = pthread_setspecific(MThreadKey->Key, Value);
	if (Ret != 0)
	{
		*Errno = Ret;
		return(MTHREAD_ER_SYSERR);
	}
	return(MTHREAD_SUCCESS);
}

ANY * qThread::MthreadGetValue(MTHREADKEY *MThreadKey, MTHREADINT *Errno)
{
	void	*ThreadData;

	ThreadData = pthread_getspecific(MThreadKey->Key);
	return(ThreadData);
}


MTHREADINT qThread::MthreadCondCreate(MTHREADCOND *MThreadCond, MTHREADINT *Errno)
{
	MTHREADINT Ret;
	Ret = pthread_cond_init(&MThreadCond->Cond, NULL);
	if (Ret != 0)
	{
		*Errno = Ret;
		return(MTHREAD_ER_SYSERR);
	}
	return(MTHREAD_SUCCESS);
}

MTHREADINT qThread::MthreadCondWait(MTHREADCOND  *MThreadCond, 
					 MTHREADMUTEX *MThreadMutex, 
					 MTHREADINT *Errno)
{
	MTHREADINT	Ret;

	pthread_cond_wait(&MThreadCond->Cond, &MThreadMutex->Mutex);
	if (Ret != 0)
	{
		*Errno = Ret;
		return(MTHREAD_ER_SYSERR);
	}
	return(MTHREAD_SUCCESS);
}

MTHREADINT qThread::MthreadCondBroadcast(MTHREADCOND *MThreadCond, 
						  MTHREADINT *Errno)
{
	MTHREADINT	Ret;

	pthread_cond_broadcast(&MThreadCond->Cond);
	if (Ret != 0)
	{
		*Errno = Ret;
		return(MTHREAD_ER_SYSERR);
	}
	return(MTHREAD_SUCCESS);
}

#endif
