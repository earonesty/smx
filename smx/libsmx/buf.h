/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
*/
#ifndef _BUF_H
#define _BUF_H

#ifdef WIN32
	#ifndef _WINDOWS_
		#include <windows.h>
		#define _WINDOWS_
	#endif

	#ifndef _WINBASE_
		#include <winbase.h>
		#define _WINBASE_
	#endif
#endif

#ifndef _INC_MEMORY
	#include <memory.h>
	#define _INC_MEMORY
#endif

#ifndef _INC_MALLOC
	#include <malloc.h>
	#define _INC_MALLOC
#endif

#ifndef _QLIB_
	#include "qlib.h"
#define _QLIB_
#endif

#ifndef BUFFER_CHAIN
	#define BUFFER_CHAIN 1
#endif

#ifndef _BUF_CHAIN_H_
	#include "buf_chain.h"
#endif

#ifndef _BUF_REF_H_
	#include "buf_ref.h"
#endif

#endif //#ifndef _BUF_H_
