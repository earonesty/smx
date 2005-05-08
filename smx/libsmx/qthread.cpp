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
