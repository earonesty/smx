/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _QFOPEN_
#define _QFOPEN_

#ifdef WIN32
	#include <io.h>
	#include <process.h>
#endif

#if defined(_WINDOWS) || defined(WIN32)
	#define fix_path(f) bslash(f)
	#define SHARED_LIB_EXT ".dll"
#else
	#define fix_path(f) fslash(f)
	#define SHARED_LIB_EXT ".so"
#endif

#if defined(_WINDOWS) || defined(WIN32)

#ifdef __GNUC__
#define ConvWin32Time(t) \
		(time_t) ((((ULARGE_INTEGER&)t).QuadPart-116444736000000000LL)/10000000)
#else
#define ConvWin32Time(t) \
		(time_t) ((((ULARGE_INTEGER&)t).QuadPart-116444736000000000)/10000000)
#endif
#endif

time_t GetFileModified(const FILE *fp);

qStr *OpenURL(qCtx *ctx, const CStr &path);
qStr *OpenFileFQ(qCtx *ctx, const char *p);
qStr *OpenFile(qCtx *ctx, CStr &path);
void ResolvePath(qCtx *ctx, CStr &path);

#endif //#ifndef _QFOPEN_
