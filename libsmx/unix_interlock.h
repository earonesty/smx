/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

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

#if (defined(__i386__)||defined(__x86_64__)) && defined(__GNUC__)

	inline void *InterlockedCompareExchange( void **dest, void *xchg, void *compare )
	{
		register void *ret;
		__asm__ __volatile__( "lock; cmpxchgl %2,(%1)"
							  : "=a" (ret) : "r" (dest), "r" (xchg), "0" (compare) : "memory" );
		return ret;
	}

	inline long InterlockedExchange( volatile long * dest, long val )
	{
		register long ret;
		__asm__ __volatile__( "lock; xchgl %0,(%1)"
							  : "=r" (ret) :"r" (dest), "0" (val) : "memory" );
		return ret;
	}

	inline int InterlockedExchangeAdd(volatile int *val, int add)
	{
		int ret;
		
		__asm__ __volatile__ ("lock; xaddl %0, %1"
				      : "=r" (ret), "=m" (*val)
				      : "0" (add), "m" (*val));
	
		return(ret);
	}


	inline long InterlockedIncrement( volatile int * dest )
	{
		return InterlockedExchangeAdd( dest, 1 ) + 1;
	}

	inline long InterlockedDecrement( volatile int * dest )
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
