/* COPYRIGHT 1998 Prime Data Corp.
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _SMX_CRIT_UNIX_H
#define _SMX_CRIT_UNIX_H

#include <pthread.h>
#include <sys/file.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

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

typedef int		MTHREADINT;
typedef void	VOID;
typedef void	ANY;
typedef void* MTHREADROUTINE;

#ifndef MTHREADONCEINIT

#define	MTHREADONCEINIT PTHREAD_ONCE_INIT

typedef	pthread_once_t	MTHREADONCE; 

typedef struct	_mthreadid 
{
	pthread_t		ThreadId; /* handle of system thread_id */
}MTHREADID;

typedef	struct	_mthread_mutex 
{
	int				Type; /* 0 = non-recursive : 1 = recursive */
	int				LCnt; /* number of locks acquired by owner of mutex */
	pthread_t		Owner; /* owner of mutex */
	pthread_mutex_t	Mutex; /* handle of system mutex */
} MTHREADMUTEX;

typedef	struct	_mthread_cond 
{
	pthread_cond_t	Cond; /* handle of system mutex */
} MTHREADCOND;

typedef	struct	_mthread_key 
{
	pthread_key_t	Key; /* handle of system key */
}MTHREADKEY;

#endif

class CCrit {
friend class CLock;
friend class CMutex;

//	CRITICAL_SECTION  m_crit;
	MTHREADMUTEX m_crit;
	MTHREADINT m_error;

	MTHREADINT MthreadMutexCreate(MTHREADMUTEX *MThreadMutex, 
				   MTHREADINT MutexType,  
				   MTHREADINT *Errno)
{
	MTHREADINT					Ret;

	Ret = pthread_mutex_init(&MThreadMutex->Mutex, NULL);
	if (Ret != 0)
	{
		*Errno = Ret;
		return(MTHREAD_ER_SYSERR);
	}
	MThreadMutex->Type = MutexType;
	MThreadMutex->LCnt = 0; /* initialize lock cnt to no owner */
	return(MTHREAD_SUCCESS);
}

MTHREADINT MthreadMutexLock(MTHREADMUTEX *MThreadMutex, 
				 MTHREADINT *Errno)
{
	MTHREADINT		Ret;
	pthread_t	MySelf;

	MySelf = pthread_self();

	if (MThreadMutex->LCnt && pthread_equal(MThreadMutex->Owner,MySelf))
	{
		if (MThreadMutex->Type == MTHREAD_NON_RECURSIVE_MUTEX)
		{
			*Errno = MTHREAD_ER_INTERNAL;
			return(MTHREAD_ER_MUTEX_LOCKED);
		}
		else /* default is recursive-mutex */
		{
			MThreadMutex->LCnt += 1; /* increment lock count */
			return(MTHREAD_SUCCESS);
		}
	}
	Ret = pthread_mutex_lock(&MThreadMutex->Mutex);
	if (Ret != 0)
	{
		*Errno = Ret;
		return(MTHREAD_ER_SYSERR);
	}
	/* warning: see file readme for processor assumptions */
	MThreadMutex->Owner = MySelf;
	MThreadMutex->LCnt = 1;
	return(MTHREAD_SUCCESS);
}



MTHREADINT MthreadMutexTryLock(MTHREADMUTEX *MThreadMutex, 
				 MTHREADINT *Errno)
{
	MTHREADINT		Ret;
	pthread_t	MySelf;

	MySelf = pthread_self();

	if (MThreadMutex->LCnt && pthread_equal(MThreadMutex->Owner,MySelf))
	{
		if (MThreadMutex->Type == MTHREAD_NON_RECURSIVE_MUTEX)
		{
			*Errno = MTHREAD_ER_INTERNAL;
			return(MTHREAD_ER_MUTEX_LOCKED);
		}
		else /* default is recursive-mutex */
		{
			MThreadMutex->LCnt += 1; /* increment lock count */
			return(MTHREAD_SUCCESS);

		}
	}
	Ret = pthread_mutex_trylock(&MThreadMutex->Mutex);
	if (Ret != 0)
	{
		*Errno = Ret;
		return(MTHREAD_ER_SYSERR);
	}
	/* warning: see file readme for processor assumptions */
	MThreadMutex->Owner = MySelf;
	MThreadMutex->LCnt = 1;
	return(MTHREAD_SUCCESS);
}

MTHREADINT MthreadMutexUnlock(MTHREADMUTEX *MThreadMutex, 
				 MTHREADINT *Errno)
{
	
	pthread_t	MySelf;
	MTHREADINT	Ret;

	MySelf = pthread_self();

	if (!MThreadMutex->LCnt)
	{
		*Errno = MTHREAD_ER_INTERNAL;
		return(MTHREAD_ER_MUTEX_UNLOCKED);
	}
		
	if (MThreadMutex->LCnt && !pthread_equal(MThreadMutex->Owner,MySelf))
	{
		*Errno = MTHREAD_ER_INTERNAL;
		return(MTHREAD_ER_MUTEX_NOTOWNER);
	}
	--MThreadMutex->LCnt;

	if (!MThreadMutex->LCnt)
	{
		/*************************************************** 
		Wish there was some POSIX define for NULL pthread_t 
		Setting the owner field to 0 is not portable 
		since pthread_t is not an integer on all platforms 
		(e.g. AIX 4.1 pthread_t is a void pointer, on Digital
		Unix 3.2 (and DCE) it is a structure with three fields)
		The statement shown under #ifdef TBD is implementation
		dependent and should be changed accordingly
 		***************************************************/

#ifdef TBD
		MThreadMutex->Owner = 0;
#endif
		Ret = pthread_mutex_unlock(&MThreadMutex->Mutex);
		if (Ret != 0)
		{
			*Errno = Ret;
			return(MTHREAD_ER_SYSERR);
		}
	}
	return(MTHREAD_SUCCESS);
}

	 void _Enter()   {MthreadMutexLock(&m_crit, &m_error);}

public:
	 CCrit()        {MthreadMutexCreate(&m_crit,MTHREAD_RECURSIVE_MUTEX,&m_error);}
	 ~CCrit()        {pthread_mutex_destroy(&(m_crit.Mutex));}

	 bool _TryEnter()   {return MthreadMutexTryLock(&m_crit, &m_error) == MTHREAD_SUCCESS;}

	 void Leave()    {MthreadMutexUnlock(&m_crit, &m_error);}

	 CLock Enter();
};

class CMutex {
friend class CMutexLock;

	CCrit m_crit;
	int m_mutex;
	int m_spin;

	static char s_mutex_path[];

public:
	~CMutex() {
		close(m_mutex);
		if (m_spin)
			Signal();
	}

	static void SetMutexPath(const char * path) {
		assert(path && *path);
		if (path && *path)
			strcpy(s_mutex_path, path);
	}

	CMutex(const char *name) {
		char full_name[1024];

		strcpy(full_name, s_mutex_path);
		strcat(full_name, name);
		m_mutex = open(full_name, O_CREAT);
		m_spin = 0;
	}

	void Signal() {
		if (m_mutex) {
			--m_spin;
			if (m_spin == 0) {
//				struct flock fl = {F_UNLCK, 0, 0, 0, 0};
//				int rVal = flock(m_mutex,LOCK_UN);
				  flock(m_mutex,LOCK_UN);
			}
			m_crit.Leave();
		}
	}

	rWait Wait(long ms = -1) {
		if (m_mutex) {
//			struct flock fl = {F_WRLCK, 0, 0, 0, 0};
			if (ms == -1) {
				m_crit._Enter();

				++m_spin;
				if (m_spin == 1) {
//					int rVal = flock(m_mutex, LOCK_EX);
//					int rErr = errno;
            flock(m_mutex, LOCK_EX);
				}
			} else {
				struct timeval tend, tcur;
				gettimeofday(&tend, NULL);
				long ts = tend.tv_usec + ms * 1000;
				tend.tv_sec += ts / 1000000;
				tend.tv_usec += ts % 1000000;

				while (!m_crit._TryEnter()) {
					gettimeofday(&tcur, NULL);
					if (tcur.tv_sec > tend.tv_sec || (tcur.tv_sec == tend.tv_sec && tcur.tv_usec > tend.tv_usec) ) {
						return rWaitTimeout;
					} else
						usleep(30*1000);
				}

				++m_spin;
				if (m_spin == 1) {
//					while (!fcntl(m_mutex,F_SETLK, &fl) && (errno == EACCES || errno == EAGAIN)) {}

					while (flock(m_mutex,LOCK_EX | LOCK_NB) && (errno == EWOULDBLOCK)) {
						gettimeofday(&tcur, NULL);
						if (tcur.tv_sec > tend.tv_sec || (tcur.tv_sec == tend.tv_sec && tcur.tv_usec > tend.tv_usec) ) {
							--m_spin;
							m_crit.Leave();
							return rWaitTimeout;
						} else
							usleep(30*1000);
					}

	//				printf("locking %d->%d (%d)\n", m_mutex, rVal, rErr);
				}
			}
			return rWaitOK;
		} else
			return rWaitInvalid;
		
	}
};


class CEvent {
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	bool ConditionExpression;
	bool IsPulsed;

public:
	~CEvent() {
		int 
		retval = pthread_cond_destroy(&cond); // if (retval!=0) then that's bad!
		retval = pthread_mutex_destroy(&mutex);
	}

	CEvent() {
		int 
		retval = pthread_cond_init(&cond, (pthread_condattr_t *)NULL); // if (retval!=0) then that's bad!
		retval = pthread_mutex_init(&mutex, NULL);
	}

	void Pulse() {//signal
		pthread_mutex_lock(&mutex);
		IsPulsed = true;
		ConditionExpression = true;
//		int retval = pthread_cond_signal(&cond); // if (retval!=0) then that's bad!
		pthread_cond_signal(&cond); // if (retval!=0) then that's bad!
		pthread_mutex_unlock(&mutex);
	}

	void Signal() {//broadcast
		pthread_mutex_lock(&mutex);
		ConditionExpression = true;
//		int retval = pthread_cond_broadcast(&cond); // if (retval!=0) then that's bad!
		pthread_cond_broadcast(&cond); // if (retval!=0) then that's bad!
		pthread_mutex_unlock(&mutex);
	}

	void Reset() {
		pthread_mutex_lock(&mutex);
		ConditionExpression = false;
		pthread_mutex_unlock(&mutex);
	}

	rWait Wait(long ms = -1) {
		int ret = 0;

		pthread_mutex_lock(&mutex);
		if (ms==-1)
		{
			while (ConditionExpression == false)
				pthread_cond_wait(&cond, &mutex);
		}
		else
		{

			struct timespec timeOut;
			struct timeval timeOutX;

			gettimeofday(&timeOutX, NULL);

			timeOutX.tv_sec = timeOutX.tv_sec + ms / 1000;
			timeOutX.tv_usec = timeOutX.tv_usec + (ms % 1000) * 1000;
			timeOutX.tv_sec = timeOutX.tv_sec + timeOutX.tv_usec % 1000000;
			timeOutX.tv_usec = timeOutX.tv_usec % 1000000;

			timeOut.tv_sec = timeOutX.tv_sec;
			timeOut.tv_nsec = timeOutX.tv_usec * 1000;

			while ((ConditionExpression == false) && (ret != ETIMEDOUT))
				ret = pthread_cond_timedwait(&cond, &mutex, &timeOut);
		}
		if (IsPulsed == true)
		{
			IsPulsed = false;
			ConditionExpression = false;
		}
		pthread_mutex_unlock(&mutex);
		if (ret==ETIMEDOUT)
			return (rWaitTimeout);
		else if (ret != 0)
			return (rWaitInvalid);
		else
			return (rWaitOK);
	}
};

#endif// #ifndef _SMX_CRIT_UNIX_H
