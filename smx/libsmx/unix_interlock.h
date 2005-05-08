#ifdef unix

#ifndef unix_interlock_h
#define unix_interlock_h

//
// The MS-MIPS and Alpha compilers support intrinsic functions for interlocked
// increment, decrement, and exchange. 
//

#if (defined(_M_MRX000) || defined(_M_ALPHA) || (defined(_M_PPC) && (_MSC_VER >= 1000))) && !defined(RC_INVOKED)

	#define InterlockedIncrement _InterlockedIncrement
	#define InterlockedDecrement _InterlockedDecrement
	#define InterlockedExchange _InterlockedExchange
	#define InterlockedExchangeAdd _InterlockedExchangeAdd
	#define InterlockedCompareExchange _InterlockedCompareExchange

	LONG
	WINAPI
	InterlockedIncrement(
		Llong * lpAddend
		);

	LONG
	WINAPI
	InterlockedDecrement(
		Llong * lpAddend
		);

	LONG
	WINAPI
	InterlockedExchange(
		Llong * Target,
		long Value
		);

	PVOID
	WINAPI
	InterlockedCompareExchange (
		void **Destination,
		void *Exchange,
		void *Comperand
		);

	LONG
	WINAPI
	InterlockedExchangeAdd(
		Llong * Addend,
		long Value
		);

	//??????????????????????????
	#pragma intrinsic(_InterlockedIncrement)
	#pragma intrinsic(_InterlockedDecrement)
	#pragma intrinsic(_InterlockedExchange)
	#pragma intrinsic(_InterlockedCompareExchange)
	#pragma intrinsic(_InterlockedExchangeAdd)

#else

#if defined(__i386__) && defined(__GNUC__)

	inline void *InterlockedCompareExchange( void **dest, void *xchg, void *compare )
	{
		void *ret;
		__asm__ __volatile__( "lock; cmpxchgl %2,(%1)"
							  : "=a" (ret) : "r" (dest), "r" (xchg), "0" (compare) : "memory" );
		return ret;
	}

	inline long InterlockedExchange( long * dest, long val )
	{
		long ret;
		__asm__ __volatile__( "lock; xchgl %0,(%1)"
							  : "=r" (ret) :"r" (dest), "0" (val) : "memory" );
		return ret;
	}

	inline long InterlockedExchangeAdd( long * dest, long incr )
	{
		long ret;
		__asm__ __volatile__( "lock; xaddl %0,(%1)"
							  : "=r" (ret) : "r" (dest), "0" (incr) : "memory" );
		return ret;
	}

	inline long InterlockedIncrement( long * dest )
	{
		return InterlockedExchangeAdd( dest, 1 ) + 1;
	}

	inline long InterlockedDecrement( long * dest )
	{
		return InterlockedExchangeAdd( dest, -1 ) - 1;
	}

#else  /* __i386__ && __GNUC__ */
	void *      WINAPI InterlockedCompareExchange(PVOID*,PVOID,PVOID);
	long        WINAPI InterlockedDecrement(long *);
	long        WINAPI InterlockedExchange(long *,LONG);
	long        WINAPI InterlockedExchangeAdd(long *,LONG);
	long        WINAPI InterlockedIncrement(long *);
#endif  /* __i386__ && __GNUC__ */

#endif

#endif //ifndef unix_interlock_h

#endif //ifdef unix
