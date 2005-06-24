/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

#ifndef _BUF_REF_H_
	#include "buf_ref.h"
#endif

#endif //#ifndef _BUF_H_
